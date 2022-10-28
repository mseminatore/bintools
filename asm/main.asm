;==========================================
; external function refs
;==========================================
INCLUDE "rtl.inc"
INCLUDE "string.inc"
INCLUDE "io.inc"
INCLUDE "os.inc"

; define our local constants
value EQU 0x10

;==========================================
; global variables in the DATA segment
; label is the addr in the DATA segment
;==========================================
lives   DB 0
        DB 1

timer       DB 0xFF
score       DW 0x1234
prompt      DS "Welcome to the CPU emulator!\n"
finished    DS "Spinning forever!"

source DS "Hello"
target DS "there!"

;==========================================
; test uninitialized data decls
;==========================================
;uninitb DB ?
;uninitw DW ?

;==========================================
; Start of program initialization
;==========================================
PROC init
    CALL rtlDisableInterrupts       ; disable interrupts

    CALL rtlInit                    ; initialize runtime

    LDX prompt                      ; display welcome prompt
    CALL puts
    
    CALL _main                      ; jump to main program
    RET

;==========================================
; Main program start
;
;==========================================
PROC _main
    CALL test

    CALL testForLoop
    CALL testMath
    CALL testBranches
    CALL testLogic
    CALL testLoadStore
    CALL testStack

    ; output finished message
    LDX finished
    CALL puts

done:    
    JMP done   ; loop forever

;==========================================
; Main task
;==========================================
PROC task

task_top:
;    LDX source
;    CALL puts

    LDA '.'
    CALL putc

    JMP task_top

; !! task never gets here!!    
    RET


;==========================================
;
;==========================================
PROC test
    LDY task                ; create a new task
    CALL os_createTask    

    CALL os_startScheduler  ; start the scheduler

; !! we should never get here !!
    RET

;==========================================
;
;==========================================
PROC testMath

    ; arithmetic
    ADD 23      ; A <- A + 23
    ADD 'c'     ; A <- A + 'c'
    ADD 0xFF
    ADD value
    ADD [0x100]
    ADD [lives]

    SUB 23
    SUB 'a'
    SUB 0x01
    SUB value
    SUB [0xFFFF]
    SUB [value]
    SUB [timer]
    RET

;==========================================
;
;==========================================
PROC testBranches
    
    ; branching
;    CALL strlen    ; external function call
    CALL _foo
    RET

;==========================================
;
;==========================================
PROC testLogic
    
    ; logic
    AND 0xFF
    AND [value]
    OR 0x01
    XOR 0xFF
    RET

;==========================================
;
;==========================================
PROC testLoadStore
    
    ; loads/stores
    LDA [0xFFFF]  ; A <- [addr16]
    LDA [value]   ; A <- [addr16]
    LDA 0x10    ; A <- imm8
    LDX 0x100   ; X <- imm16
    LAX         ; A <- [X]
    STA 0xFFFE  ; addr16 <- A
    STA timer   ; addr16 <- A

;    LDX switch  ; load the address of 'switch'
;    LAX         ; load A with the data at the address 'switch'
;    LDA timer   ; load A the low byte of the address of 'timer'
;    LDA [timer] ; load A with byte at the address of 'timer'

    LDX 0xFFFE
    LDX [0xFFFE]        ; get the reset vector
    LDX reset_vector    ; get the addr of reset vector
    LDX [reset_vector]  ; get the reset vector
    LDX score           ; get addr of 'score' variable
    LDX [score]         ; get the score
    STX reset_vector    ; store X at the location 'reset_vector'

    RET

;==========================================
;
;==========================================
PROC testStack
    
    ; pushes and pops
    PUSH A
    POP A
    PUSH A, X
    PUSH A CC
    POP A, CC
    POP A, X
    RET

;==========================================
;
;==========================================
PROC testForLoop
    
    ; for i = 1, 10 loop
    LDA 3
loop1:
    PUSH A

    ; do something!

    POP A
    SUB 1
    JNE loop1
    RET

;==========================================
; foo
;==========================================
PROC _foo
    PUSH X
    PUSH SP     ; get the stack frame pointer in X
    POP Y

    LEAX 12     ; load the address of the value
    LEAX -12     ; load the address of the value
    LEAX value
    
    LAX         ; load the value at [X] into A

    POP X
    RET
