  ; Demonstrated here:
  ; - comments
  ; - binary, hex, and decimal literals
  ; - named registers
  ; - jump to labels
  ; - named variables

  JUMP start
; Data can appear anywhere in the program, but you have to jump over it
firstvar:
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