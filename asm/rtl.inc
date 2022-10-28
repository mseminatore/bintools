;===============================
; Constant defns
;===============================
reset_vector EQU 0xFFFE
int_vector EQU 0xFFFC
swi_vector EQU 0xFFFA
brk_vector EQU 0xFFF8

NULL    EQU 0
TRUE    EQU 1
FALSE   EQU 0

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
