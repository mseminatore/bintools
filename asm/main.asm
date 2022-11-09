;==========================================
; external function refs
;==========================================
INCLUDE "rtl.inc"
INCLUDE "string.inc"
INCLUDE "io.inc"
INCLUDE "os.inc"

;==========================================
; global variables in the DATA segment
; label is the addr in the DATA segment
;==========================================
prompt      DS "Welcome to the CPU emulator!\n"
finished    DS "Spinning forever!"
;in_buf      DM 32

;==========================================
; Start of program initialization
;==========================================
PROC init
    CALL rtlInit                    ; initialize runtime

    LDX prompt                      ; display welcome prompt
    CALL puts
    
    CALL _main                      ; jump to main program
    RET

;==========================================
; Main program start
;==========================================
PROC _main
;    LDX in_buf
;    CALL gets

    LDY task                ; create a new task
    CALL os_createTask    

    CALL os_startScheduler  ; start the scheduler

; !! Note that execution never gets here!!    

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
    LDA '1'
    CALL putc

    JMP task_top

; !! task never gets here!!    
    RET
