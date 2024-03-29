;==================================================
; rtl.inc - include file for rtl.asm
;
; Copyright 2022 by Mark Seminatore
; See LICENSE.md for rights and obligations
;===================================================

;===============================
; Constant defns
;===============================
reset_vector    EQU 0xFFFE
int_vector      EQU 0xFFFC
swi_vector      EQU 0xFFFA
brk_vector      EQU 0xFFF8

MMIO_TIMER_REG	EQU 0xFF00	; current value of timer
MMIO_TIMER_ENA	EQU 0xFF01	; timer enabled/disabled
MMIO_TIMER_LIM	EQU 0xFF02	; timer limit

NULL    EQU 0
TRUE    EQU 1
FALSE   EQU 0

; CPU condition code flags
FLAG_C  EQU 1
FLAG_Z  EQU 2
FLAG_V  EQU 4
FLAG_N  EQU 8
FLAG_I  EQU 16
FLAG_S  EQU 32

;===============================
; External procs
;===============================
EXTERN rtlMemcpy
EXTERN rtlZeroMemory
EXTERN rtlInit
EXTERN rtlMalloc
EXTERN rtlFree
EXTERN rtlDisableInterrupts
EXTERN rtlEnableInterrupts

;===============================
; external data decls
;===============================
EXTERN systick
