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

; Data must appear at the end of your program!
myvar:
  WORD          ; Allocate 8 bits of memory and call it myvar
```

### Program output:
```
uASM - by Matt Murphy, for EEE 333 wth Seth Abraham
  Input file:   ucpu.asm
  Output file:  ucpu.dat

Labels

  end        = 0b on line 17
  myvar      = 0c on line 21

Instructions

  loadi a 0b1      00  38
                   01  01
  LOADI B 0x2      02  39
                   03  02
  ADDR  A B        04  08
                   05  01
  STORE A myvar    06  2c
                   07  0c
  JUMP  end        08  03
                   09  0b
  ADDI  A 2        0a  04
  HALT             0b  00
  WORD             0c  00
```

### Resulting `loader.dat` file:
```
00  38
01  01
02  39
03  02
04  08
05  01
06  2c
07  0c
08  03
09  0b
0a  04
0b  00
0c  00
```
