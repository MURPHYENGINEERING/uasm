  ldi a 0x17
  mov b a
  nop
  store myvar b
  ld c myvar
  addi c 0x3
  add c myvar
  jmp label
  add d myvar
label:
  halt

myvar:
  word 0
