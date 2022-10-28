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
dot         DS "."

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

    LDX dot
    CALL puts

    JMP idle_top

; !!code should never get here!!
    RET

;==========================================
; Desc: switch to next running task
;
;
;==========================================
PROC os_schedule
    LDX [current]           ; get current task
    CALL os_getContext      ; save its stack to the TCB

    ; get ptr to next task in the runnable queue
    ; make it the current
    ; restore its context
    ; return to the new task    
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

;    LDY idleTask                ; create idle task
;    CALL os_createTask          ; TODO - check for error?

    ; TODO - this might not be needed since CC for new tasks are 0?
    CALL rtlEnableInterrupts    ; start pre-emptive scheduling

    LDX [runnable]                ; make the first runnable task the current
    STX current

    ; setup stack for the current task
    CALL os_setContext

    BRK

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
    POP A, X, Y
    RET

;==========================================
; Desc: get current execution context from stack
;
; Input: Y points to destination TCB
;
; Return: none
;==========================================
PROC os_getContext
    LEAY 2              ; advance to point to registers in TCB

    PUSH SP             ; get ptr to start of stack regs
    POP X
;    LEAX 2              ; account for return PC for this function

    LDA TCB_REG_SIZE    ; 10 bytes of registers
    CALL rtlMemcpy      ; copy from stack to TCB

    RET

;==========================================
; Desc: set current execution context on stack
;
; Input: X points to source TCB
;==========================================
PROC os_setContext
    LEAX 2              ; advance to point to registers in TCB

    PUSH SP             ; get ptr to start of stack regs
    POP Y
    LEAY 2              ; account for return PC for this function

    LDA TCB_REG_SIZE    ; 10 bytes of registers
    CALL rtlMemcpy      ; copy from TCB to stack

    RET
