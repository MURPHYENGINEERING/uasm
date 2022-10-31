main:
  ldi a message
  push a
  call print_string
  halt


; Calling convention: string address is on the stack
print_string:
  ; Get the string address argument
  pop a
  ; Save the current cursor position so we can increment it by one character
  ld b cursor
  push b
print_string_loop:
  rld b a                     ; Retrieve the character from the string
  jz b print_string_loop_end  ; escape at the null terminator

  ; Decode the character into a font glyph
  ldi c 0x30    ; Translate from ASCII to our table
  sub b c
  ldi c font_0  ; Point at the glyph in the font table
  add b c

  ; Print the font glyph
  push b
  call print_glyph
  
  ; Go to the next character
  inc a     
  jmp print_string_loop   
print_string_loop_end:
  ; Restore the old cursor position
  pop b
  ; Increment horizontally to the next character position
  inc b
  store cursor b
  ret ; print_string


; Calling convention: glyph address is on the stack
print_glyph:
  ; Get the glyph address argument into A
  pop a

  ldi c 7           ; A font glyph comprises 8 words (7..0)
print_glyph_loop:
  rld b a           ; Load the glyph data at memory location A into B

  ; TODO: Copy glyph word from B into framebuffer
  ; The glyph words are arranged vertically on the screen, so we need to
  ; write a word, then advance the cursor by one whole screen width

  inc a             ; Go to the next glyph word
  dec c             ; count down
  jnz c print_glyph_loop

  ret ; print_glyph


; ---- DATA ----
cursor:
  word 0xc0   ; The cursor points into the framebuffer at the point where the
              ; next font glyph will be written.

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
