INCLUDE "rtl.inc"
INCLUDE "error.inc"
INCLUDE "io.inc"

;==========================================
; Desc: Task Control Block (TCB)
;
; TCB structure - 12 bytes total
;   0 next ptr
;   2 CC
;   3 A
;   4 Y
;   6 X
;   8 SP
;   10 PC
;==========================================

;
TCB_SIZE        EQU 12
TCB_REG_SIZE    EQU 10
STACK_SIZE      EQU 127

;==========================================
; Scheduler state variables
;
;==========================================
current     DW 0    ; ptr to current task TCB
runnable    DW 0    ; ptr to start of runnable queue
task_msg    DS "New task created, TCB is at: 0x"

;=======================================
; Update our timer tick count
;=======================================
PROC timerIntHandler
;    LDX [systick]   ; get current tick count
;    LEAX 1          ; increment it
;    STX systick     ; save it

;    CALL os_schedule       ; schedule the next runnable task

    RTI

;==========================================
; Idle task
;==========================================
PROC idleTask

idle_top:
    ; TODO - count idle cycles?

    LDA 'X'
    CALL putc

    JMP idle_top

; !!code should never get here!!
    RET

;==========================================
; Desc: switch to next runnable task
;
; Input: none
;
; Return: none
;==========================================
PROC os_schedule
    PUSH X, Y
    LDY [current]           ; get ptr to current task TCB

    PUSH SP                 ; get current SP in X
    POP X
    LEAX 6                  ; adjust for PC and pushes
    CALL os_getContext      ; save its stack to the TCB

    BRK

    ; get ptr to next task in the runnable queue

    ; make it the current

    ; restore its context
    CALL os_setContext

    ; return to the new task    
    POP X, Y
    RET

;==========================================
; Desc: put current task to sleep
;
;
;==========================================
PROC os_sleep
    CALL os_schedule
    RET

;==========================================
; Desc: Start the task scheduler
;
;==========================================
PROC os_startScheduler
    ;
    ; don't bother to save Y because we never return to the caller!
    ;

    ; setup timer interrupt handler
    LDX timerIntHandler         ; note this is a CODE PTR!
    STX int_vector

    LDY idleTask                ; create idle task
    CALL os_createTask          ; TODO - check for error?

    ; not required since the CC for new tasks are 0
;    CALL rtlEnableInterrupts    ; start pre-emptive scheduling

    LDX [runnable]                ; make the first runnable task the current
    STX current

    ; setup stack for the current task
    CALL os_setContext

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

    CALL rtlZeroMemory      ; zero the TCB

    ; add task to runnable queue
    PUSH Y                  ; save Y (task PC) on stack
    LDY [runnable]          ; get ptr to next task on runnable queue
    STX runnable            ; make new task the head of runnable queue
    PUSH Y                  ; save next ptr to stack

    POP A                   ; get lobyte of next ptr
    STAX                    ; store it in TCB next

    LEAX 1                  ; point to TCB next + 1
    POP A                   ; get hibyte of next ptr
    STAX                    ; store it in TCB next + 1

    ;
    ; setup new task PC and SP in the TCB
    ;
    LEAX 7                  ; point to lobyte of task SP

    PUSH X                  ; save X
    LDA STACK_SIZE          ; size of stack to allocate
    CALL rtlMalloc          ; allocate new stack
    PUSH X                  ; move new stack ptr in X to Y
    POP Y
    AAY                     ; Y + A is top of stack
    LEAY -1
    POP X                   ; restore X

    PUSH Y                  ; save new task SP
    POP A                   ; get lobyte of task SP
    STAX                    ; store it
    LEAX 1                  ; point to hibyte of task SP
    POP A                   ; get hibyte of task SP
    STAX                    ; store it

    LEAX 1                  ; point to the TCB PC
    POP A                   ; get lobyte of task PC
    STAX                    ; store it
    LEAX 1                  ; point to TCB PC + 1
    POP A                   ; get hibyte of task PC
    STAX                    ; store it

os_createTask_done:
    LDX task_msg            ; print new task info
    CALL puts
    LDX [runnable]          ; get ptr to new task TCB
    PUSH X
    POP A
    POP A                   ; get top byte of new task
    CALL printHexByte

    PUSH X
    POP A
    CALL printHexByte
    POP A

    LDA '\n'
    CALL putc

    POP A, X, Y
    RET

;====================================================================
; Desc: copy current execution context from stack to TCB
;
; Input: X points to registers on stack, Y points to destination TCB
;
; Return: none
;=====================================================================
PROC os_getContext
    PUSH X, Y           ; save X and Y
    LEAY 2              ; advance Y to point to registers in TCB

    LEAX 6              ; account for return PC and pushes for this function

    LDA TCB_REG_SIZE    ; 10 bytes of registers
    CALL rtlMemcpy      ; copy from stack to TCB

    POP X, Y            ; restore X, Y and return
    RET

;================================================================
; Desc: copy current execution context from TCB to stack
;
; Input: X points to source TCB, Y points to registers on stack
;================================================================
PROC os_setContext
    LEAX 2              ; advance X to point to registers in TCB

    PUSH SP             ; get ptr to start of stack regs
    POP Y               ; Y points to the stack
    LEAY 2              ; account for return PC for this function

    LDA TCB_REG_SIZE    ; 10 bytes of registers
    CALL rtlMemcpy      ; copy from TCB to stack

    LEAY 6              ; point Y to SP on stack
    PUSH Y              ; save Y
    LYY                 ; get saved SP
    LEAY -2             ; adjust stack value to make room for the PC
    POP X               ; get saved ptr to the stacked SP
    STYX                ; update the stacked SP

    LEAX 2              ; point X to saved PC
    LXX                 ; get the saved PC
    STXY                ; put saved PC on the new stack

    RET
