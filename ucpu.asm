  ldi a 0X01
  ldi b 0B10
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
