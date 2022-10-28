# uasm
Assembly language for a microcoded CPU implemented on FPGA, for EEE 333 with Seth Abraham

### Usage:

```
cmake .
cmake --build . --target all
./uasm [input_file] [output_file]
```

If you don't specify `output_file` then it will use the name of the input file and add the extension `.dat`. If you don't specify filenames then it will use `ucpu.asm` and output to `ucpu.dat`.

### Instructions
| Example<img width=300/>                       | First argument                  | Second argument                | Third argument               | Description                                                                                                                                         |
|-----------------------------------------------|---------------------------------|--------------------------------|------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------|
| `ldi a 0xff`<br/>`ldi b 0b1010`<br/>`ldi c 3` | Destination register            | Source constant                |                              | Load the given constant value into the destination register.                                                                                        |
| `ld a myvar`<br/>`load a 0xda`                | Destination register            | Source label or memory address |                              | Load the contents of memory at the given address into the destination register. The address may be specified literally or referred to with a label. |
| `mov a b`                                     | Desination register             | Source register                |                              | Copy the contents of the source register into the destination register.                                                                             |
| `store myvar a`<br/>`store 0xff a`            | Destination memory address      | Source register                |                              | Store the contents of the source register into memory at the given address. The address can be specified literally or referred to with a label.     |
| `inc a`                                       | Source and destination register |                                |                              | Increment the contents of the given register.                                                                                                       |
| `dec a`                                       | Source and destination register |                                |                              | Decrement the contents of the given register.                                                                                                       |
| `add a b`                                     | Destination register            | RHS register                   |                              | Add the contents of the RHS register to the destination register.                                                                                   |
| `sub a b`                                     | Destination register            | RHS register                   |                              | Subtract the contents of the RHS register from the destination register.                                                                            |
| `mul a b`                                     | Destination register            | RHS register                   |                              | Multiply the contents of the RHS register by the destination register.                                                                              |
| `div a b`                                     | Destination register            | RHS register                   |                              | Divide the contents of the destination register by the RHS register.                                                                                |
| `and a b`                                     | Destination register            | RHS register                   |                              | Bitwise AND the contents of the destination register with the RHS register.                                                                         |
| `or a b`                                      | Destination register            | RHS register                   |                              | Bitwise OR the contents of the destination register with the RHS register.                                                                          |
| `xor a b`                                     | Destination register            | RHS register                   |                              | Bitwise XOR the contents of the destination register with the RHS register.                                                                         |
| `jmp mylabel`<br/>`jmp 0xff`                  | Destination address or label    |                                |                              | Jump to the given address. The address can be specified literally or referred to with a label.                                                      |
| `je a b mylabel`                              | LHS register                    | RHS register                   | Destination address or label | Jump to the given address if the contents of the two registers are equal.                                                                           |
| `jne a b mylabel`                             | LHS register                    | RHS register                   | Destination address or label | Jump to the given address if the contents of the registers are not equal.                                                                           |
| `jgr a b mylabel`                             | LHS register                    | RHS register                   | Destination address or label | Jump to the given address if the contents of the LHS register are greater than the contents of the RHS register.                                    |
| `jz a mylabel`                                | LHS register                    | Destination address or label   |                              | Jump to the given address if the contents of the LHS register are zero.                                                                             |
| `jnz a mylabel`                               | LHS register                    | Destination address            |                              | Jump to the given address if the contents of the LHS register are not zero.                                                                         |
| `call mylabel`                                | Destination address or label    |                                |                              | Push the next instruction address to the stack and jump to the given address.                                                                       |
| `ret`                                         |                                 |                                |                              | Pop an instruction address from the stack and jump to it.                                                                                           |
| `push a`                                      | Source register                 |                                |                              | Push the contents of the source register onto the stack.                                                                                            |
| `pop b`                                       | Destination register            |                                |                              | Pop the top of the stack into the destination register.                                                                                             |


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

  ADD  A B     ; Add the contents of register B into register A
  STORE A myvar ; Store the result into the memory address called myvar
  JUMP  end     ; Skip the next instruction
  ADDI  A 2     ; Add 2 into the contents of register A
                ; If it works correctly then this should be skipped,
                ; and the value of myvar will be 3
end:
  load  c myvar
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
  myvar      = 12 on line 27

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
                   0a  12
  JUMP  end        0b  03
                   0c  0f
  ADDI  A 2        0d  04
                   0e  02
  load  c myvar    0f  2a
                   10  12
  HALT             11  00
  WORD             12  00
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
0a  12
0b  03
0c  0f
0d  04
0e  02
0f  2a
10  12
11  00
12  00
```
