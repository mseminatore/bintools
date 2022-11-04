;==================================================
; os.asm - minimal pre-emptive multi-tasking kernel
;
; Copyright 2022 by Mark Seminatore
; See LICENSE.md for rights and obligations
;===================================================

INCLUDE "rtl.inc"
INCLUDE "error.inc"
INCLUDE "io.inc"
INCLUDE "list.inc"

;==========================================
; Desc: Task Control Block (TCB)
;
; TCB structure - 12 bytes total
;   0 next ptr
;   2 SP
;   4 CC
;   5 A
;   6 Y
;   8 X
;   10 PC
;==========================================

TCB_SIZE        EQU 12
TCB_REG_SIZE    EQU 8
STACK_SIZE      EQU 127
TCB_SP_OFFSET   EQU 2
TCB_PC_OFFSET   EQU 10

;==========================================
; Scheduler state variables
;
;==========================================
current     DW 0    ; ptr to current task TCB
runnable    DW 0    ; ptr to start of runnable queue
sleeping    DW 0    ; ptr to sleep queue

task_msg    DS "New task created, TCB is at: 0x"
task_len    DS "Tasks: 0x"

;=======================================
; Timer interrupt handler
;=======================================
PROC timerIntHandler
    CALL _os_schedule       ; schedule the next runnable task

    RTI

;==========================================
; Idle task
; TODO - count idle cycles?
;==========================================
PROC idleTask

idle_top:
    LDA '.'
    CALL putc

    JMP idle_top

; !!code should never get here!!
    RET

;==========================================
; Desc: switch to the next runnable task
;
; Input: none
;
; Return: none
;==========================================
PROC _os_schedule
    PUSH X, Y               ; save X and Y
    LDY [current]           ; get ptr to current task TCB

    PUSH SP                 ; get current SP in X
    POP X
    LEAX 6                  ; adjust for caller PC and local pushes
    CALL os_getContext      ; save its stack to the TCB

    LDY [current]           ; get ptr to current task TCB
    LYY                     ; get ptr to next task in the runnable queue
    JNE os_schedule1        ; if ptr is valid continue

    LDY [runnable]          ; otherwise start at head of queue

os_schedule1:
    STY current             ; make next task TCB the current

    PUSH X                  ; get old task SP in Y
    POP Y
    LDX [current]           ; get the new task TCB in X
    LEAX TCB_SP_OFFSET      ; get ptr to new task SP in TCB
    LXX                     ; get the new task SP
    STXY                    ; store the SP at bottom of interrupt return frame

    ;
    ; Note - we don't need restore next task context here because it is 
    ; already saved on its own stack from its last run!
    ;
;;;    CALL os_setContext

    POP X, Y                ; restore X, Y and return
    RET

;====================================================================
; Desc: copy current execution context from stack to TCB
;
; Input: X points to registers on stack, Y points to destination TCB
;
; Return: none
;=====================================================================
PROC os_getContext
    PUSH A, X, Y        ; save A, X and Y
    LEAY 2              ; advance Y to point to registers in TCB

    LDA TCB_REG_SIZE    ; 10 bytes of registers
    CALL rtlMemcpy      ; copy from stack to TCB

    POP A, X, Y         ; restore A, X, Y and return
    RET

;================================================================
; Desc: copy current execution context from TCB to stack
;
; Input: X points to source TCB, Y points to registers on stack
;================================================================
PROC os_setContext
    PUSH A, X, Y        ; save A, X and Y
    LEAX 2              ; advance X to point to registers in TCB

    LDA TCB_REG_SIZE    ; 10 bytes of registers
    CALL rtlMemcpy      ; copy from TCB to stack

    POP A, X, Y            ; restore X, Y and return
    RET

;==========================================
; Desc: put current task to sleep
;
; Input: none
;
; Return: none
;==========================================
PROC os_sleep
    CALL _os_schedule
    RET

;==========================================
; Desc: yield rest of timelice of current task
;
; Input: none
;
; Return: none
;==========================================
PROC os_yield
    CALL _os_schedule
    RET

;==========================================
; Desc: Start the task scheduler
;
; Input: none
;
; Return: none - never returns to caller
;==========================================
PROC os_startScheduler
    ;
    ; don't bother to save Y because we never return to the caller!
    ;

    LDX [current]               ; get current task TCB ptr
    JEQ os_start1               ; see if there is a task running
    RET                         ; just return if so

os_start1:
    LDX 1
    STX MMIO_TIMER_ENA          ; enable timer interrupts
    
    LDX timerIntHandler         ; setup our timer ISR
    STX int_vector

    LDY idleTask                ; create idle task
    CALL os_createTask          ; TODO - check for error?

    LDX [runnable]              ; make the first runnable task the current
    STX current

    ; setup stack for the first (current) task
    PUSH X                      ; get task TCB ptr in Y
    POP Y
    LEAY TCB_SP_OFFSET          ; point to the new task SP
    LYY                         ; get the new task SP

    LEAY -2
    CALL os_setContext          ; setup context on the first task stack

    PUSH Y                      ; put new task SP on bottom of stack
    POP SP

    RTI                         ; fake a return from interrupt

; !! code never gets here !!
    RET

;==========================================
; Desc: Alloc TCB and add to runnable queue
;
; Input: Y points to start of task CODE
;
; Return: none
;==========================================
PROC os_createTask
    PUSH A, X, Y

    LDA TCB_SIZE            ; size of TCB
    CALL rtlMalloc          ; allocate a new TCB
    JEQ os_createTask_done  ; check for malloc fail

    CALL rtlZeroMemory      ; zero the TCB block

    ; add task to runnable queue
    PUSH Y                  ; save Y (task PC) on stack

    LDY [runnable]          ; get ptr to next task on runnable queue
    STX runnable            ; make new task the head of runnable queue
    STYX                    ; save ptr to former head of queue to new task next

    ;
    ; setup new task PC and SP in the TCB
    ;
    LEAX TCB_PC_OFFSET      ; point to task PC in TCB
    POP Y                   ; restore task PC
    PUSH Y                  ; but keep a copy on the stack for later
    STYX                    ; store it in TCB PC

    LDX [runnable]
    LEAX TCB_SP_OFFSET      ; point to task SP in TCB
    PUSH X                  ; save ptr to task SP in TCB
    POP Y                   ; and move it to Y

    LDA STACK_SIZE          ; size of stack to allocate
    CALL rtlMalloc          ; allocate new stack, returned in X
    CALL rtlZeroMemory      ; zero the stack memory

    SUB 1                   ; X + A - 1 is top of stack
    AAX                 
    LEAX -8                 ; adjust for registers that will be saved on stack
    STXY                    ; save task SP to TCB

    ;
    ; setup new task stack with PC
    ;
    LEAX 6                  ; point to PC in the new stack
    POP Y                   ; get task PC from earlier
    STYX                    ; store task PC in the new stack

os_createTask_done:
    LDX task_msg            ; print new task info
    CALL puts
    LDX [runnable]          ; get ptr to new task TCB
    PUSH X
    POP A
    POP A                   ; get top byte of new task
    CALL printHexByte

    PUSH X                  ; get lo byte of new task
    POP A
    CALL printHexByte       ; print it out
    POP A

    LDA '\n'                ; newline
    CALL putc

    POP A, X, Y             ; restore A, X, Y and return
    RET
