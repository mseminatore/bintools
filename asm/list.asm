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

    LDY NULL
    STYX
    
    POP Y       ; restore Y
    RET

;==========================================
; Input: X points to list header, Y points to entry
;==========================================
PROC list_add
    RET

;==========================================
; Input: X points to list header, Y points to entry
;==========================================
PROC list_remove
    RET

;==========================================
; Input: X points to list header, Y points to entry
;
; Return: length in A
;==========================================
PROC list_length
    RET
    