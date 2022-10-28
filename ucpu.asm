  ldi   a 7       ; literals can be 0x7, 0b111, or 7
  push  a         ; save the contents of register A
  call  fun
  pop   a         ; restore the contents of register A
  ld    b myvar   ; read B from memory at myvar
  add   a b       ; add B into A
  halt            ; result: A = 10, B = 3

fun: 
  ldi   a 3       ; interfere with register A
  store myvar a   ; write A into memory at myvar
  ret

myvar:
  word            ; allocate 8 bits at a named location
