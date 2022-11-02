;==================================================
; rtl.asm - run-time library routines
;
; Copyright 2022 by Mark Seminatore
; See LICENSE.md for rights and obligations
;===================================================

INCLUDE "rtl.inc"
INCLUDE "error.inc"

EXTERN __ram_start

systick     DW 0 
heapPointer DW 0
errno       DW 0

;=======================================
; Update our timer tick count
;=======================================
PROC intHandler
    LDX [systick]   ; get current tick count
    LEAX 1          ; increment it
    STX systick     ; save it
    RTI

;=======================================
; SWI handler
;=======================================
PROC swiHandler
    RTI

;=======================================
; BRK handler
;=======================================
PROC brkHandler
    POP A           ; get CC flags in A

    OR FLAG_S       ; set S flag

    PUSH A          ; put CC back on stack
    RTI

;=======================================
; Desc: Enable interrupts
;
; Inputs: none
;
; Return: none
;=======================================
PROC rtlEnableInterrupts
    PUSH CC
    POP A           ; get CC flags in A

    AND 0xEF        ; clear I flag

    PUSH A
    POP CC          ; move A to CC and return
    RET

;=======================================
; Desc: Disable interrupts
;
; Inputs: none
;
; Return: none
;=======================================
PROC rtlDisableInterrupts
    PUSH CC
    POP A           ; get CC flags in A

    OR FLAG_I       ; set I flag

    PUSH A
    POP CC          ; move A to CC and return
    RET

;=======================================
; Desc: Initialize the run-time
;
; Inputs: none
;
; Return: none
;=======================================
PROC rtlInit
    PUSH X

    ; setup SYSTICK handler
    LDX intHandler  ; note this is a CODE PTR!
    STX int_vector

    ; setup SWI handler
    LDX swiHandler
    STX swi_vector

    ; setup BRK handler
    LDX brkHandler
    STX brk_vector

    ; initialize the memory heap
    CALL _rtlHeapInit

    POP X           ; restore X and return
    RET

;=======================================
; Desc: Free all heap memory
;
; Inputs:
; X - points to the memory location
; A - holds the count of bytes
;
; Return: none
;=======================================
PROC rtlZeroMemory
    PUSH A, X

zero_top:
    CMP 0           ; check for zero length
    JEQ zerodone    ; if so done

    PUSH A          ; save count

    LDA 0           ; write 0 to mem
    STAX
    LEAX 1          ; increase ptr

    POP A           ; retrieve count
    SUB 1           ; dec count
    JMP zero_top    ; repeat

zerodone:
    POP A, X        ; restore A, X and return
    RET

;=======================================
; Desc: initialize the run-time heap
;
; Inputs: none
;
; Return: none
;=======================================
PROC _rtlHeapInit
    ; reset the heap to initial empty state
    CALL free_all
    RET

;=======================================
; Desc: copy source memory to destination
;
; Input: A has length. X, Y point to src, dest
;
; Return: none
;=======================================
PROC rtlMemcpy
    PUSH A, X, Y

    CMP 0               ; check for zero length
    JEQ memcpy_done     ; if so done

memcpy_top:
    PUSH A          ; save count
    LAX             ; get next source byte
    STAY            ; write to destination
    LEAX 1          ; inc src/dest pointers
    LEAY 1
    POP A           ; retrieve count
    SUB 1           ; dec count
    JNE memcpy_top  ; while not done, repeat

memcpy_done:
    POP A, X, Y     ; restore registers and return
    RET

;==========================================
; Input: A - size of memory block requested
; 
; Return: X - ptr to memory or NULL
;==========================================
PROC rtlMalloc
    CMP 0
    JEQ malloc_fail     ; fail on zero size alloc request

    LDX [heapPointer]   ; get the next free memory
    PUSH X              ; save new ptr on stack

    AAX                 ; increment heap pointer by block size
    STX heapPointer     ; save the new heap location

    POP X               ; restore X and return
    RET

malloc_fail:
    PUSH A
    LDA E_MEMORY        ; set error number
    STA errno
    LDX NULL            ; set return to NULL
    POP A               ; restore A and return
    RET

;=======================================
; Desc: Free heap memory
;
; Input: X - ptr to memory
;
; Return: none
;=======================================
PROC rtlFree
    ;
    ; we are currently a stack allocator, do nothing here!
    ;
    RET

;=======================================
; Desc: Free all heap memory
;
; Input: none
;
; Return: none
;=======================================
PROC free_all
    PUSH X

    ; get address of start or free RAM
    LDX __ram_start

    ; make heapPointer point to that address 
    STX heapPointer

    POP X      ; restore X and return
    RET

;=======================================
; Desc: Add A to 16b number in memory
;
; Input: X points to number, A has addend
;
; Return: none
;=======================================
PROC add16
    PUSH X

    LAX             ; load lobyte of number
    ADD 1           ; add 1
    STAX            ; store new lobyte
    LEAX 1          ; point to hibyte
    LAX             ; load hibyte
    ADC 0           ; add in carry bit
    STAX            ; store result

    POP X          ; restore X and return
    RET
