;==================================================
; io.asm - I/O routines
;
; Copyright 2022 by Mark Seminatore
; See LICENSE.md for rights and obligations
;===================================================

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
    ;LDA '\n'            ; write a newline
    OUT IO_TERMINAL

    POP A, X, PC        ; restore A, X and return

;=============================================
; Input: A has value in lower nibble
;=============================================
PROC printHexNibble
    PUSH A              ; save A

    CMP 9               ; check if a letter
    JGT printHex_letter

    ADD '0'             ; make number a printable char
    CALL putc           ; print it
    JMP printHex_done

printHex_letter:
    ADD 'A'             ; shift number to [A-F]
    SUB 10
    CALL putc           ; print it

printHex_done:
    POP A               ; restore A
    RET

;=============================================
; Desc: print given byte in hex form
;
; Input: A byte to print
;
; Return: none
;=============================================
PROC printHexByte
    PUSH A              ; save A
    PUSH A              ; make a copy of A

    AND 0xF0            ; get top nibble
    SHR 4               ; shift to lower nibble
    AND 0x7F            ; mask off top bit
    CALL printHexNibble ; print hex digit

    POP A               ; get original copy of A
    AND 0x0F            ; get lower nibble
    CALL printHexNibble ; print hex digit

    POP A               ; restore A and return
    RET

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

