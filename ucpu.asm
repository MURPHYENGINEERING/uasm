  ldi a 1
  ldi b 2
  jmp fun1
ret: 
  ld d myvar
  halt

fun1:
  ldi c 3
  ldi d 4
  store myvar d
  ldi d 0
  jmp ret

myvar:
  WORD
