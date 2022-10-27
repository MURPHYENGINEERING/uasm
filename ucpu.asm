  ; Demonstrated here:
  ; - comments
  ; - binary, hex, and decimal literals
  ; - named registers
  ; - jump to labels
  ; - named variables

  loadi a 1     ; Load A so we know where we started
  call fun1     ; Push the PC and jump to fun
  loadi c 0x27  ; Load C so we know we returned from fun
  halt

fun1:
  loadi b 0x2   ; Load B so we know we reached this function
  call fun2
  ret           ; Go back to start
  loadi d 0x34  ; This will never run!

fun2:
  loadi d 0x17  ; Load D so we know we got here
  ret           ; Go back into fun1

; End result:
; A = 0x1
; B = 0x2
; C = 0x27
; D = 0x17