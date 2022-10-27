  ldi a 0b1100
  ldi b 0b0101
  xor a b
  ldi c 0xDA
  mov d c
  jnz a pass
  halt

pass:
  ldi c 0xff
  halt