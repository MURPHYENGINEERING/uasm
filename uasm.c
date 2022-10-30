#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _MSC_VER
  // not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
  #define strncasecmp _strnicmp
  #define strcasecmp _stricmp
#else
  #include <strings.h>
#endif

// Specifies the default output file format
// .dat : loader.dat format
// .mif : Memory Initialization File format
#define DEFAULT_OUTPUT_EXTENSION ".mif"
// TODO: get these from command line arguments
// and actually use them to format opcodes
#define WORD_WIDTH 32
#define MEMORY_DEPTH 1024

// Maximum length of an input line
#define LINE_MAX_LEN 1024
// Maximum length of a label name
#define LABEL_MAX_LEN 128
// Number of spaces separating the input and output when printed to stdout
#define PADDING_SEP 4
// Maximum number of arguments that any opcode can have
#define N_ARGS_MAX 8

// Separates tokens in the input
#define DELIM " \t"

// Named jump addresses or variables
typedef struct {
  char name[LABEL_MAX_LEN];
  uint8_t addr;
} Label;

typedef enum {
  OP_WORD = 0b00000000,
  OP_HALT = 0b00000000,
  OP_NOP  = 0b00000001,

  OP_LD    = 0b00101000,
  OP_LDI   = 0b00111000,
  OP_STORE = 0b00101100,
  OP_MOV   = 0b00111100,

  OP_ADD = 0b00000100,
  OP_SUB = 0b00001000,
  OP_MUL = 0b00001100,
  OP_DIV = 0b00010000,

  OP_AND = 0b00010100,
  OP_OR  = 0b00011000,
  OP_XOR = 0b00011100,
  OP_SHL = 0b01101100,
  OP_SHR = 0b01110000,

  OP_INC = 0b00100000,
  OP_DEC = 0b00100100,

  OP_CALL = 0b10000000,
  OP_RET  = 0b10000001,

  OP_PUSH = 0b10000100,
  OP_POP  = 0b10001000,

  OP_JUMP = 0b00000011,
  OP_JE   = 0b11000100,
  OP_JNE  = 0b11001000,
  OP_JG   = 0b11001100,
  OP_JZ   = 0b11010000,
  OP_JNZ  = 0b11100100,
} Opcode;

typedef struct {
  Opcode opcode;
  uint8_t reg; // Target register, or 0 if not specified.
               // The target register index becomes the low 2 bits of the opcode
  // Opcode arguments appear as data between instructions
  uint32_t args[N_ARGS_MAX];
  size_t nArgs;
  uint32_t mach; // Machine code translation
} OP;


typedef enum { OF_LOADER, OF_MIF } OutputFormat;
OutputFormat outputFormat;


// Jump addresses
Label labels[1024];
size_t nLabels = 0;

// Current line number being read, for error reporting
size_t iInputLine    = 0;
// Current program counter location, for labels
unsigned int curAddr = 0;

// For left-justifying the output
size_t longestLineLen = 0;



static void
emit_mif_header(FILE* of, size_t width, size_t depth)
{
  fprintf(
      of,
      "-- Intel Memory Initialization File\n"
      "\n"
      "-- Generated by uasm by Matthew Murphy\n"
      "-- for EEE 333 with Seth Abraham\n"
      "\n"
      "WIDTH=%lu;\n"
      "DEPTH=%lu;\n"
      "\n"
      "ADDRESS_RADIX=HEX;\n"
      "DATA_RADIX=HEX;\n"
      "\n"
      "CONTENT BEGIN\n",
      width,
      depth);
}


/*
 * Return the extension of the given filename, or an empty string if none exists
 */
static char const*
get_filename_ext(char const* filename)
{
  char const* dot = strrchr(filename, '.');
  if (!dot || dot == filename)
    return "";
  return dot + 1;
}


/*
 * Modify the given file name to remove the last extension that appears.
 */
static void
strip_filename_ext(char* fname)
{
  char* end = fname + strlen(fname);

  while (end > fname && *end != '.')
    --end;

  if (end > fname)
    *end = '\0';
}


/*
 * Modify the given string to remove whitespace from the beginning and end.
 */
static char*
strip_whitespace(char* str)
{
  char* end;

  // Trim leading space
  while (isspace((unsigned char) *str))
    str++;

  if (*str == 0) // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char) *end))
    end--;

  // Write new null terminator character
  end[1] = '\0';

  return str;
}


/*
 * Modify the given string to remove the first comment and everything following.
 */
static void
strip_comments(char* str)
{
  while (*str != 0 && *str != ';')
    ++str;
  *str = '\0';
}


/*
 * Signal an exception and terminate the program.
 */
static void
expected(char const* msg)
{
  printf("\n\n! Expected %s at line %lu\n\n", msg, iInputLine);
  exit(1);
}


/*
 * Search the given input line for a label name. If a name is found, create
 * an entry in the label vector with the current PC address as its pointer.
 *
 * Return true if a label entry was created.
 */
static bool
find_label(char* line, bool notify)
{
  size_t end = strnlen(line, LINE_MAX_LEN) - 1;
  if (line[end] == ':') {
    line[end] = '\0';
    if (strnlen(line, LINE_MAX_LEN) == 0)
      expected("label declaration");

    strncpy(labels[nLabels].name, line, LABEL_MAX_LEN);
    labels[nLabels].addr = curAddr;
    if (notify)
      printf(
          "  %-10s = %08x on line %lu\n",
          labels[nLabels].name,
          curAddr,
          iInputLine);
    ++nLabels;
    return true;
  }
  return false;
}


/*
 * Return true if the given operator names match.
 */
static bool
isop(char const* search, char const* match)
{
  return strcasecmp(search, match) == 0;
}


/*
 * Read a letter register from the input and return the associated register
 * index.
 */
static uint8_t
get_register()
{
  char* arg = strtok(NULL, DELIM);
  if ((arg == NULL) || (toupper(*arg) < 'A') || (toupper(*arg) > 'D'))
    expected("register (A, B, C, or D)");

  return toupper(*arg) - 'A';
}


/*
 * Read a constant literal from the given token string.
 */
static uint32_t
get_const_from_token(char* arg)
{
  if (*arg == '0') {
    if (*(arg + 1) == 'x' || *(arg + 1) == 'X') {
      return strtoul(arg, NULL, 16);
    } else if (*(arg + 1) == 'b' || *(arg + 1) == 'B') {
      return strtoul(arg + 2, NULL, 2);
    }
  }
  return strtoul(arg, NULL, 10);
}


/*
 * Read a constant literal from the input.
 */
static uint32_t
get_const()
{
  char* arg = strtok(NULL, DELIM);
  if (arg == NULL)
    expected("constant argument");

  return get_const_from_token(arg);
}


/*
 * Read and return an address from the input. If the input is a jump label,
 * then return the address associated with that name.
 *
 * If emit is true then missing labels will throw an exception; otherwise they
 * will be ignored
 */
static uint8_t
get_address(bool emit)
{
  char* arg = strtok(NULL, DELIM);
  if (arg == NULL)
    expected("label or memory address");

  if (isalpha(*arg)) {
    for (size_t i = 0; i < nLabels; ++i) {
      if (strcasecmp(arg, labels[i].name) == 0)
        return labels[i].addr;
    }
    if (emit) {
      printf("\n! Unknown label: %s", arg);
      expected("label");
    }
    return 0;
  } else {
    return get_const_from_token(arg);
  }
}


/*
 * Translate the given line of input to machine code and optionally emit the
 * result into the given output file, as well as to standard output.
 *
 * Translating without emitting is useful to keep track of the program counter
 * as it's incremented by instructions with variable numbers of arguments.
 */
static void
translate_line(char* line, FILE* of, bool emit)
{
  char buf[128];
  if (emit) {
    // Left-justify the output and pad with spaces
    snprintf(buf, sizeof(buf) - 1, "  %%-%lus", longestLineLen + PADDING_SEP);
    printf(buf, line);
  }

  char* tok = strtok(line, DELIM);

  if (tok == NULL) {
    expected("opcode");
  }

  OP op;
  memset(&op, 0, sizeof(OP));

  if (isop(tok, "LDI")) {
    op.opcode  = OP_LDI;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_const();

  } else if (isop(tok, "LD")) {
    op.opcode  = OP_LD;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_address(emit);

  } else if (isop(tok, "STORE")) {
    op.opcode  = OP_STORE;
    op.nArgs   = 1;
    op.args[0] = get_address(emit);
    op.reg     = get_register();

  } else if (isop(tok, "MOV")) {
    op.opcode  = OP_MOV;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_register();

  } else if (isop(tok, "INC")) {
    op.opcode = OP_INC;
    op.reg    = get_register();

  } else if (isop(tok, "DEC")) {
    op.opcode = OP_DEC;
    op.reg    = get_register();

  } else if (isop(tok, "ADD")) {
    op.opcode  = OP_ADD;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_register();

  } else if (isop(tok, "SUB")) {
    op.opcode  = OP_SUB;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_register();

  } else if (isop(tok, "MUL")) {
    op.opcode  = OP_MUL;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_register();

  } else if (isop(tok, "DIV")) {
    op.opcode  = OP_DIV;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_register();

  } else if (isop(tok, "AND")) {
    op.opcode  = OP_AND;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_register();

  } else if (isop(tok, "OR")) {
    op.opcode  = OP_OR;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_register();

  } else if (isop(tok, "XOR")) {
    op.opcode  = OP_XOR;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_register();

  } else if (isop(tok, "SHL")) {
    op.opcode  = OP_SHL;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_register();

  } else if (isop(tok, "SHR")) {
    op.opcode  = OP_SHR;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_register();

  } else if (isop(tok, "JMP")) {
    op.opcode  = OP_JUMP;
    op.nArgs   = 1;
    op.args[0] = get_address(emit);

  } else if (isop(tok, "JE")) {
    op.opcode  = OP_JE;
    op.nArgs   = 2;
    op.reg     = get_register();
    op.args[0] = get_register();
    op.args[1] = get_address(emit);

  } else if (isop(tok, "JNE")) {
    op.opcode  = OP_JNE;
    op.nArgs   = 2;
    op.reg     = get_register();
    op.args[0] = get_register();
    op.args[1] = get_address(emit);

  } else if (isop(tok, "JG")) {
    op.opcode  = OP_JG;
    op.nArgs   = 2;
    op.reg     = get_register();
    op.args[0] = get_register();
    op.args[1] = get_address(emit);

  } else if (isop(tok, "JZ")) {
    op.opcode  = OP_JZ;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_address(emit);

  } else if (isop(tok, "JNZ")) {
    op.opcode  = OP_JNZ;
    op.nArgs   = 1;
    op.reg     = get_register();
    op.args[0] = get_address(emit);

  } else if (isop(tok, "CALL")) {
    op.opcode  = OP_CALL;
    op.nArgs   = 1;
    op.args[0] = get_address(emit);

  } else if (isop(tok, "RET")) {
    op.opcode = OP_RET;

  } else if (isop(tok, "PUSH")) {
    op.opcode = OP_PUSH;
    op.reg    = get_register();

  } else if (isop(tok, "POP")) {
    op.opcode = OP_POP;
    op.reg    = get_register();

  } else if (isop(tok, "NOP")) {
    op.opcode = OP_NOP;

  } else if (isop(tok, "HALT")) {
    op.opcode = OP_HALT;

  } else if (isop(tok, "WORD")) {
    op.opcode = OP_WORD;

  } else if (emit) {
    snprintf(buf, sizeof(buf) - 1, "opcode ('%s' is not valid)", tok);
    expected(buf);
  }

  // The machine code of an opcode has the target register in the low two bits.
  op.mach = op.opcode + op.reg;

  if (emit) {
    switch (outputFormat) {
    case OF_LOADER:
      fprintf(of, "%08x  %08x\n", curAddr, op.mach);
      printf("%08x  %08x\n", curAddr, op.mach);
      break;

    case OF_MIF:
      fprintf(of, "  %08x : %08x;\n", curAddr, op.mach);
      printf("%08x : %08x;\n", curAddr, op.mach);
      break;
    }
  }

  for (size_t i = 0; i < op.nArgs; ++i) {
    ++curAddr;
    if (emit) {
      switch (outputFormat) {
      case OF_LOADER:
        fprintf(of, "%08x  %08x\n", curAddr, op.args[i]);
        // Left-justify the output and pad with spaces
        snprintf(
            buf,
            sizeof(buf) - 1,
            "  %%-%lus%%08x  %%08x\n",
            longestLineLen + PADDING_SEP);
        printf(buf, "", curAddr, op.args[i]);
        break;

      case OF_MIF:
        fprintf(of, "  %08x : %08x;\n", curAddr, op.args[i]);
        // Left-justify the output and pad with spaces
        snprintf(
            buf,
            sizeof(buf) - 1,
            "  %%-%lus%%08x : %%08x\n",
            longestLineLen + PADDING_SEP);
        printf(buf, "", curAddr, op.args[i]);
        break;
      }
    }
  }
}


/*
 * Translate every line in the given input file, emitting machine code into the
 * given output file, as well as to standard output.
 */
static void
translate_file(FILE* inFile, FILE* outFile)
{
  size_t len = 0;

  // The first pass finds labels. The input is translated so that the PC can be
  // counted, but no output is emitted.

  // The second pass emits translation output, replacing labels with their
  // addresses.
  for (size_t pass = 0; pass < 2; ++pass) {
    fseek(inFile, 0, 0);
    iInputLine = 0;
    curAddr    = 0;

    if (pass == 0)
      printf("\nLabels\n\n");
    else
      printf("\nInstructions\n\n");

    char buf[8192];
    char* line;
      while ((line = fgets(buf, sizeof(buf), inFile))) {
      ++iInputLine;

      strip_comments(line);
      // Don't change line because we need to free it later
      char* cleanLine = strip_whitespace(line);

      if (*cleanLine == '\0')
        continue;

      // Measure the longest line so we can left-justify and pad stdout
      size_t cleanLen = strlen(cleanLine);
      if (cleanLen > longestLineLen)
        longestLineLen = cleanLen;

      if (!find_label(cleanLine, (pass == 0))) {
        translate_line(cleanLine, outFile, (pass == 1));
        ++curAddr;
      }
    }

    if (errno) {
      printf(
          "\n! Error reading line from input file (%d): %s",
          errno,
          strerror(errno));
      break;
    }
  }
}


int
main(int argc, char* argv[])
{
  char* inFilename;
  char* outFilename;

  inFilename = (argc > 1) ? argv[1] : "ucpu.asm";

  if (argc > 2) {
    outFilename = argv[2];
  } else {
    char buf[FILENAME_MAX];
    strncpy(buf, inFilename, sizeof(buf) - 1);
    strip_filename_ext(buf);
    strncat(buf, DEFAULT_OUTPUT_EXTENSION, sizeof(buf) - 1);
    outFilename = buf;
  }

  char const* ext = get_filename_ext(outFilename);
  if (strcasecmp(ext, "dat") == 0) {
    outputFormat = OF_LOADER;
  } else if (strcasecmp(ext, "mif") == 0) {
    outputFormat = OF_MIF;
  } else {
    printf("\n! Output file should have .dat or .mif extension.\n");
    return 1;
  }

  printf("uASM - by Matt Murphy, for EEE 333 wth Seth Abraham\n");
  printf("  Input file:\t\t%s\n  Output file:\t\t%s\n", inFilename, outFilename);
  printf(
      "  Output Format:\t%s\n",
      (outputFormat == OF_LOADER) ? "loader.dat"
                                  : "Intel Memory Initialization File");

  FILE* inFile = fopen(inFilename, "r");
  if (!inFile) {
    printf("\n! Input file does not exist\n\n");
    return 1;
  }

  FILE* outFile = fopen(outFilename, "w");
  if (!outFile) {
    printf("\n! Couldn't open output file '%s' for writing", outFilename);
    fclose(inFile);
    return 1;
  }

  if (outputFormat == OF_MIF)
    emit_mif_header(outFile, WORD_WIDTH, MEMORY_DEPTH);

  translate_file(inFile, outFile);

  if (outputFormat == OF_MIF) {
    fprintf(outFile, "  [%08x..%08x] : 00;\n", curAddr, MEMORY_DEPTH - 1);
    fprintf(outFile, "END;");
  }

  fclose(inFile);
  fclose(outFile);

  printf("\n");

  return 0;
}
