INCLUDE "io.inc"

;=======================================
; Desc: print a character on terminal
;
; Input: character in A
;
; Return: none
;=======================================
PROC putc
    OUT IO_TERMINAL
    RET

;=======================================
; Desc: get a character from keyboard
;
; Input: none
;
; Return: character in A
;=======================================
PROC getc
    IN IO_TERMINAL
    RET

;=======================================
; Desc: get a string from keyboard
;
; Input: pointer to buffer in X
;
; Return:
;=======================================
PROC gets
    PUSH A, X

gets_top:
    CALL getc       ; get input char
    CMP '\n'        ; compare char to newline
    JEQ gets_done

    STAX            ; write char to buffer
    LEAX 1          ; advance buffer ptr
    JMP gets_top    ; repeat

gets_done:
    LDA 0           ; null terminate the string
    STAX

    POP A, X, PC    ; restore A, X and return

;=======================================
; Desc: print string on terminal
;
; Input: pointer to string in X
;
; Return: none
;=======================================
PROC puts
    PUSH A, X

puts_loop:
    LAX                 ; get next char
    JEQ puts_done       ; if null we're done

    OUT IO_TERMINAL     ; output the char
    LEAX 1              ; advance string pointer
    JMP puts_loop       ; repeat

puts_done:
    LDA '\n'            ; write a newline
    OUT IO_TERMINAL

    POP A, X, PC        ; restore A, X and return

;=======================================
; Desc: open a file
;
; Input: X points to filename
;
; Return: A has file descriptor or EOF
;=======================================
PROC open
    LDA -1
    RET
