main:
  ldi a message
  push a
  call print_string
  halt


; Calling convention: string address is on the stack
print_string:
  ; Get the string address argument
  pop a
print_string_loop:
  rld b a                     ; Retrieve the character from the string
  jz b print_string_loop_end  ; escape at the null terminator

  ; Decode the character into a font glyph
  ldi c 0x30    ; Translate from ASCII to our table
  sub b c
  ldi c font_0  ; Point at the glyph in the font table
  add b c

  ; Print the character
  push b
  call print_char
  
  ; Go to the next character
  inc a     
  jmp b print_string_loop   
print_string_loop_end:
  ret ; print_string


; Calling convention: character address is on the stack
print_char:
  ; Get the character address argument into A
  pop a

  ldi c 7           ; A character font comprises 8 words
print_char_loop:
  rld b a           ; Load the data at memory location A into B

  ; TODO: Copy character line from B into framebuffer
  ; The character lines are arranged vertically on the screen, so we need to
  ; write a line, then advance the cursor by one whole screen width

  inc a             ; Go to the next character word
  dec c             ; count down
  jnz c print_char_loop

  ret ; print_char


; ---- DATA ----
cursor:
  word 0  ; The cursor points at a word in memory where the next
          ; character will be written. The character is multiple vertical lines,
          ; though, so it won't be written contiguously; instead, each character
          ; word will be written with a stride of SCREEN_WIDTH.

message:
  string HELLO WORLD

; ---- 8 bit Font ----

; ASCII 0 starts at 0x30, so to find the character in this table, just subtract
; 0x30 from the ASCII value and add the result to font_0
font_0:
  word 0b01111100
  word 0b11001110
  word 0b11011110
  word 0b11110110
  word 0b11100110
  word 0b11000110
  word 0b01111100
  word 0b00000000

font_1:
  word 0b00011000
  word 0b00111000
  word 0b00011000
  word 0b00011000
  word 0b00011000
  word 0b00011000
  word 0b01111110
  word 0b00000000

font_2:
  word 0b01111100
  word 0b11000110
  word 0b00000110
  word 0b01111100
  word 0b11000000
  word 0b11000000
  word 0b11111110
  word 0b00000000

font_3:
  word 0b11111100
  word 0b00000110
  word 0b00000110
  word 0b00111100
  word 0b00000110
  word 0b00000110
  word 0b11111100
  word 0b00000000

font_4:
  word 0b00001100
  word 0b11001100
  word 0b11001100
  word 0b11001100
  word 0b11111110
  word 0b00001100
  word 0b00001100
  word 0b00000000

font_5:
  word 0b11111110
  word 0b11000000
  word 0b11111100
  word 0b00000110
  word 0b00000110
  word 0b11000110
  word 0b01111100
  word 0b00000000

font_6:
  word 0b01111100
  word 0b11000000
  word 0b11000000
  word 0b11111100
  word 0b11000110
  word 0b11000110
  word 0b01111100
  word 0b00000000

font_7:
  word 0b11111110
  word 0b00000110
  word 0b00000110
  word 0b00001100
  word 0b00011000
  word 0b00110000
  word 0b00110000
  word 0b00000000

font_8:
  word 0b01111100
  word 0b11000110
  word 0b11000110
  word 0b01111100
  word 0b11000110
  word 0b11000110
  word 0b01111100
  word 0b00000000

font_9:
  word 0b01111100
  word 0b11000110
  word 0b11000110
  word 0b01111110
  word 0b00000110
  word 0b00000110
  word 0b01111100
  word 0b00000000

font_colon:
  word 0b00000000
  word 0b00011000
  word 0b00011000
  word 0b00000000
  word 0b00000000
  word 0b00011000
  word 0b00011000
  word 0b00000000

font_semicolon:
  word 0b00000000
  word 0b00011000
  word 0b00011000
  word 0b00000000
  word 0b00000000
  word 0b00011000
  word 0b00011000
  word 0b00110000

font_<:
  word 0b00001100
  word 0b00011000
  word 0b00110000
  word 0b01100000
  word 0b00110000
  word 0b00011000
  word 0b00001100
  word 0b00000000

font_=:
  word 0b00000000
  word 0b00000000
  word 0b01111110
  word 0b00000000
  word 0b01111110
  word 0b00000000
  word 0b00000000
  word 0b00000000

font_>:
  word 0b00110000
  word 0b00011000
  word 0b00001100
  word 0b00000110
  word 0b00001100
  word 0b00011000
  word 0b00110000
  word 0b00000000

font_?:
  word 0b00111100
  word 0b01100110
  word 0b00001100
  word 0b00011000
  word 0b00011000
  word 0b00000000
  word 0b00011000
  word 0b00000000

font_@:
  word 0b01111100
  word 0b11000110
  word 0b11011110
  word 0b11011110
  word 0b11011110
  word 0b11000000
  word 0b01111110
  word 0b00000000

font_A:
  word 0b00111000
  word 0b01101100
  word 0b11000110
  word 0b11000110
  word 0b11111110
  word 0b11000110
  word 0b11000110
  word 0b00000000

font_B:
  word 0b11111100
  word 0b11000110
  word 0b11000110
  word 0b11111100
  word 0b11000110
  word 0b11000110
  word 0b11111100
  word 0b00000000

font_C:
  word 0b01111100
  word 0b11000110
  word 0b11000000
  word 0b11000000
  word 0b11000000
  word 0b11000110
  word 0b01111100
  word 0b00000000

font_D:
  word 0b11111000
  word 0b11001100
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11001100
  word 0b11111000
  word 0b00000000

font_E:
  word 0b11111110
  word 0b11000000
  word 0b11000000
  word 0b11111000
  word 0b11000000
  word 0b11000000
  word 0b11111110
  word 0b00000000

font_F:
  word 0b11111110
  word 0b11000000
  word 0b11000000
  word 0b11111000
  word 0b11000000
  word 0b11000000
  word 0b11000000
  word 0b00000000

font_G:
  word 0b01111100
  word 0b11000110
  word 0b11000000
  word 0b11000000
  word 0b11001110
  word 0b11000110
  word 0b01111100
  word 0b00000000

font_H:
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11111110
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b00000000

font_I:
  word 0b01111110
  word 0b00011000
  word 0b00011000
  word 0b00011000
  word 0b00011000
  word 0b00011000
  word 0b01111110
  word 0b00000000

font_J:
  word 0b00000110
  word 0b00000110
  word 0b00000110
  word 0b00000110
  word 0b00000110
  word 0b11000110
  word 0b01111100
  word 0b00000000

font_K:
  word 0b11000110
  word 0b11001100
  word 0b11011000
  word 0b11110000
  word 0b11011000
  word 0b11001100
  word 0b11000110
  word 0b00000000

font_L:
  word 0b11000000
  word 0b11000000
  word 0b11000000
  word 0b11000000
  word 0b11000000
  word 0b11000000
  word 0b11111110
  word 0b00000000

font_M:
  word 0b11000110
  word 0b11101110
  word 0b11111110
  word 0b11111110
  word 0b11010110
  word 0b11000110
  word 0b11000110
  word 0b00000000

font_N:
  word 0b11000110
  word 0b11100110
  word 0b11110110
  word 0b11011110
  word 0b11001110
  word 0b11000110
  word 0b11000110
  word 0b00000000

font_O:
  word 0b01111100
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b01111100
  word 0b00000000

font_P:
  word 0b11111100
  word 0b11000110
  word 0b11000110
  word 0b11111100
  word 0b11000000
  word 0b11000000
  word 0b11000000
  word 0b00000000

font_Q:
  word 0b01111100
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11010110
  word 0b11011110
  word 0b01111100
  word 0b00000110

font_R:
  word 0b11111100
  word 0b11000110
  word 0b11000110
  word 0b11111100
  word 0b11011000
  word 0b11001100
  word 0b11000110
  word 0b00000000

font_S:
  word 0b01111100
  word 0b11000110
  word 0b11000000
  word 0b01111100
  word 0b00000110
  word 0b11000110
  word 0b01111100
  word 0b00000000

font_T:
  word 0b11111111
  word 0b00011000
  word 0b00011000
  word 0b00011000
  word 0b00011000
  word 0b00011000
  word 0b00011000
  word 0b00000000

font_U:
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11111110
  word 0b00000000

font_V:
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b01111100
  word 0b00111000
  word 0b00000000

font_W:
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b11010110
  word 0b11111110
  word 0b01101100
  word 0b00000000

font_X:
  word 0b11000110
  word 0b11000110
  word 0b01101100
  word 0b00111000
  word 0b01101100
  word 0b11000110
  word 0b11000110
  word 0b00000000

font_Y:
  word 0b11000110
  word 0b11000110
  word 0b11000110
  word 0b01111100
  word 0b00011000
  word 0b00110000
  word 0b11100000
  word 0b00000000

font_Z:
  word 0b11111110
  word 0b00000110
  word 0b00001100
  word 0b00011000
  word 0b00110000
  word 0b01100000
  word 0b11111110
  word 0b00000000
