# lab4asm
Assembly language for a microcoded CPU implemented on FPGA, for EEE 333 with Seth Abraham

### Usage:

```
cmake .
cmake --build . --target all
./uasm [input_file] [output_file]
```

If you don't specify `output_file` then it will use the name of the input file and add the extension `.dat`. If you don't specify filenames then it will use `ucpu.asm` and output to `ucpu.dat`.

### Example program (`ucpu.asm`):
```
  ; Demonstrated here:
  ; - comments
  ; - binary, hex, and decimal literals
  ; - named registers
  ; - jump to labels
  ; - named variables

  JUMP start


firstvar:       ; Data can appear anywhere if you jump over it
  WORD

start:
  loadi a 0b1   ; Load a binary literal constant into register A
  LOADI B 0x2   ; Load a hex literal constant into register B

  ADDR  A B     ; Add the contents of register B into register A
  STORE A myvar ; Store the result into the memory address called myvar
  JUMP  end     ; Skip the next instruction
  ADDI  A 2     ; Add 2 into the contents of register A
                ; If it works correctly then this should be skipped,
                ; and the value of myvar will be 3
end:
  HALT

myvar:
  WORD          ; Allocate 8 bits of memory and call it myvar
```

### Assembler output:
```
uASM - by Matt Murphy, for EEE 333 wth Seth Abraham
  Input file:   ucpu.asm
  Output file:  ucpu.dat

Labels

  firstvar   = 02 on line 10
  start      = 03 on line 13
  end        = 0f on line 23
  myvar      = 10 on line 26

Instructions

  JUMP start       00  03
                   01  03
  WORD             02  00
  loadi a 0b1      03  38
                   04  01
  LOADI B 0x2      05  39
                   06  02
  ADDR  A B        07  08
                   08  01
  STORE A myvar    09  2c
                   0a  10
  JUMP  end        0b  03
                   0c  0f
  ADDI  A 2        0d  04
                   0e  02
  HALT             0f  00
  WORD             10  00
```

### Resulting `loader.dat` file:
```
00  03
01  03
02  00
03  38
04  01
05  39
06  02
07  08
08  01
09  2c
0a  10
0b  03
0c  0f
0d  04
0e  02
0f  00
10  00
```
