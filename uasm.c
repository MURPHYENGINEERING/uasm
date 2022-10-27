#include <ctype.h>
#include <errno.h>
#include <libgen.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#define LINE_MAX_LEN 1024
#define LABEL_MAX_LEN 32

#define MEMORY_WIDTH 16
#define MEMORY_DEPTH 256

#define DELIM " \t"

typedef struct {
  char name[LABEL_MAX_LEN];
  size_t addr;
} Label;

typedef enum {
  OP_LOADI = 0b111000,
  OP_LOAD  = 0b101000,
  OP_STORE = 0b101100,
  OP_ADDI  = 0b000100,
  OP_ADDR  = 0b001000,
  OP_ADDM  = 0b001100,
  OP_JUMP  = 0b000011,
  OP_HALT  = 0b000000,
  OP_NOP   = 0b000001,
  OP_WORD  = 0b000000
} Opcode;

typedef struct {
  Opcode opcode;
  uint8_t reg;   // Specified register, or 0 if not specified
  int16_t arg;   // Argument: constant, memory address, register index, or -1
  uint16_t mach; // Machine code translation
} OP;


// Jump addresses
Label labels[1024];
size_t nLabels = 0;

// Current line number being read, for error reporting
size_t iInputLine = 0;
// Current program counter location, for labels
size_t curAddr    = 0;

// For left-justifying the output
size_t longestLineLen = 0;


static void
strip_file_extension(char* fname)
{
  char* end = fname + strlen(fname);

  while (end > fname && *end != '.')
    --end;

  if (end > fname)
    *end = '\0';
}


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


static void
strip_comments(char* str)
{
  while (*str != 0 && *str != ';')
    ++str;
  *str = '\0';
}


static void
expected(char const* msg)
{
  printf("\n\n! Expected %s at line %lu\n\n", msg, iInputLine);
  exit(1);
}


static bool
find_label(char* line, bool notify)
{
  size_t end = strnlen(line, LINE_MAX_LEN) - 1;
  if (line[end] == ':') {
    line[end] = '\0';
    if (strlen(line) == 0)
      expected("label declaration");

    strlcpy(labels[nLabels].name, line, LABEL_MAX_LEN);
    labels[nLabels].addr = curAddr;
    if (notify)
      printf(
          "  %-10s = %02x on line %lu\n",
          labels[nLabels].name,
          (unsigned int) curAddr,
          iInputLine);
    ++nLabels;
    return true;
  }
  return false;
}


static bool
isop(char const* search, char const* match)
{
  return strcasecmp(search, match) == 0;
}


static uint8_t
get_register()
{
  char* arg = strtok(NULL, DELIM);
  if ((arg == NULL) || (toupper(*arg) < 'A') || (toupper(*arg) > 'D'))
    expected("register (A, B, C, or D)");

  return toupper(*arg) - 'A';
}


static uint8_t
get_const_ex(char* arg)
{
  if (*arg == '0') {
    if (*(arg + 1) == 'x') {
      return strtoul(arg, NULL, 16);
    } else if (*(arg + 1) == 'b') {
      return strtoul(arg + 2, NULL, 2);
    }
  }
  return strtoul(arg, NULL, 10);
}


static uint8_t
get_const()
{
  char* arg = strtok(NULL, DELIM);
  if (arg == NULL)
    expected("constant argument");

  return get_const_ex(arg);
}


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
    return get_const_ex(arg);
  }
}


static void
translate_line(char* line, FILE* of, bool emit)
{
  char buf[128];
  if (emit) {
    sprintf(buf, "  %%-%lus", longestLineLen + 4);
    printf(buf, line);
  }

  char* tok = strtok(line, DELIM);

  if (tok == NULL) {
    expected("opcode");
  }

  OP op;
  op.reg = 0;
  op.arg = -1;

  if (isop(tok, "LOADI")) {
    op.opcode = OP_LOADI;
    op.reg    = get_register();
    op.arg    = get_const();
  } else if (isop(tok, "LOAD")) {
    op.opcode = OP_LOAD;
    op.reg    = get_register();
  } else if (isop(tok, "STORE")) {
    op.opcode = OP_STORE;
    op.reg    = get_register();
    op.arg    = get_address(emit);
  } else if (isop(tok, "ADDI")) {
    op.opcode = OP_ADDI;
    op.reg    = get_register();
  } else if (isop(tok, "ADDR")) {
    op.opcode = OP_ADDR;
    op.reg    = get_register();
    op.arg    = get_register();
  } else if (isop(tok, "ADDM")) {
    op.opcode = OP_ADDM;
    op.reg    = get_register();
  } else if (isop(tok, "JUMP")) {
    op.opcode = OP_JUMP;
    op.arg    = get_address(emit);
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

  op.mach = op.opcode + op.reg;

  if (emit) {
    fprintf(of, "%02x  %02x\n", (unsigned int) curAddr, op.mach);
    printf("%02x  %02x\n", (unsigned int) curAddr, op.mach);
  }

  if (op.arg > -1) {
    ++curAddr;
    if (emit) {
      fprintf(of, "%02x  %02x\n", (unsigned int) curAddr, op.arg);
      sprintf(buf, "  %%-%lus%%02x  %%02x\n", longestLineLen + 4);
      printf(buf, "", curAddr, op.arg);
    }
  }
}


static void
translate_file(FILE* inFile, FILE* outFile)
{
  char* line = (char*) malloc(LINE_MAX_LEN);
  size_t len = 0;

  bool startJumpWritten = false;

  for (size_t pass = 0; pass < 2; ++pass) {
    fseek(inFile, 0, 0);
    iInputLine = 0;
    curAddr    = 0;

    if (pass == 0)
      printf("\nLabels\n\n");
    else
      printf("\nInstructions\n\n");

    int read = getline(&line, &len, inFile);
    while (read > -1) {
      ++iInputLine;

      strip_comments(line);
      char* cleanLine = strip_whitespace(line);

      if (*cleanLine != 0) {
        size_t len = strlen(cleanLine);
        if (len > longestLineLen)
          longestLineLen = len;

        if (!find_label(cleanLine, (pass == 0))) {
          translate_line(cleanLine, outFile, (pass == 1));
          ++curAddr;
        }
      }

      read = getline(&line, &len, inFile);
    }

    if (errno) {
      printf(
          "\n! Error reading line from input file (%d): %s",
          errno,
          strerror(errno));
      break;
    }
  }

  free(line);
}


int
main(int argc, char* argv[])
{
  char* inFilename = argv[1];
  char* outFilename;

  inFilename = (argc > 1) ? argv[1] : "ucpu.asm";

  if (argc > 2) {
    outFilename = argv[2];
  } else {
    char buf[1024];
    strncpy(buf, basename(inFilename), sizeof(buf) - 1);
    strip_file_extension(buf);
    strncat(buf, ".dat", sizeof(buf) - 1);
    outFilename = buf;
  }

  printf("uASM - by Matt Murphy, for EEE 333 wth Seth Abraham\n");
  printf("  Input file:\t%s\n  Output file:\t%s\n", inFilename, outFilename);

  FILE* inFile = fopen(inFilename, "r");
  if (!inFile) {
    printf("\n! Input file does not exist\n\n");
    return -1;
  }

  FILE* outFile = fopen(outFilename, "w");
  if (!outFile) {
    printf("\n! Couldn't open output file '%s' for writing", outFilename);
    fclose(inFile);
    return -1;
  }

  translate_file(inFile, outFile);

  fclose(inFile);
  fclose(outFile);

  printf("\n");

  return 0;
}
