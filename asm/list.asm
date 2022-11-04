;==================================================
; list.asm - routines for singly linked list
;
; Copyright 2022 by Mark Seminatore
; See LICENSE.md for rights and obligations
;===================================================

INCLUDE "rtl.inc"

;==========================================
; Desc: initialize an empty list
;
; Input: X points to list header
;
; Return: none
;==========================================
PROC list_init
    PUSH Y      ; save Y

    LDY NULL    ; set next ptr to null
    STYX
    
    POP Y       ; restore Y
    RET

;==========================================
; Input: 
;   X points to list head
;   Y points to new entry
;
; Return: none
;==========================================
PROC list_add
    PUSH X, Y       ; save X, Y

    PUSH X          ; save ptr to list head
    LXX             ; get list head next ptr
    STXY            ; new entry next points to current list head
    POP X           ; restore ptr to list head
    STYX            ; new entry becomes new list head

    POP X, Y        ; restore X, Y
    RET

;==========================================
; Input: 
;   X points to list header
;   Y points to entry to remove
;==========================================
PROC list_remove
    PUSH X, Y       ; save X, Y

    POP X, Y        ; restore X, Y and return
    RET

;==========================================
; Input: X points to list header
;
; Return: length in A
;==========================================
PROC list_length
    PUSH X      ; save X

    LDA 0       ; initialize count

list_length_top:
    LXX         ; get next ptr
;    LEAX 0      ; test whether ptr is null
    JEQ list_length_done

    ADD 1       ; inc count
    JMP list_length_top

list_length_done:
    POP X
    RET
    