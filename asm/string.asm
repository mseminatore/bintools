INCLUDE "rtl.inc"

;===============================
; Desc: Return the length of string
;
; Input: X points to the string
;
; Return: A contains the length
;===============================
PROC strlen
    PUSH X              ; save X

    LDA 0
    PUSH A              ; init length counter

strlen_loop:
    LAX
    JEQ strlen_done

    POP A               ; retrieve the counter
    ADD 1               ; count++
    PUSH A              ; put count back on the stack

    LEAX 1              ; inc X
    JMP strlen_loop

strlen_done:
    POP A, X, PC           ; restore A, X and return

;=======================================
; Desc: Copy one string to another
;
; Input: X, Y point to s1 and s2
;
; Return: none
;=======================================
PROC strcpy
    PUSH A, X, Y

strcpy_start:
    LAX                 ; get next char from s1
    STAY                ; copy it to s2
    LEAX 1              ; inc X
    LEAY 1              ; inc Y
    JEQ strcpy_start    ; null char?

    POP A, X, Y, PC     ; restore A, X, Y and return

;============================================
; Desc: Compare two strings
;
; Input: X, Y point to s1 and s2
;
; Return: -1 if less, 0 if equal, 1 if greater
;=============================================
;PROC strcmp
;    PUSH X, Y           ; save X and Y

;    LAX                 ; get next char from s1

;    LDA 0

;    POP X, Y, PC        ; restore X, Y and return

;=============================================
; Desc: check if character is lowercase
;
; Input: character in A
;
; Return: A true if lower case, else false
;=============================================
PROC islower
    CMP 'a'             ; compare A to 'a'
    JLT islower_fail    ; if less, return false

    CMP 'z'             ; compare A to 'z'
    JGT islower_fail    ; if greater, return false

    LDA TRUE            ; return true
    RET

islower_fail:
    LDA FALSE           ; return false
    RET

;=============================================
; Desc: check if character is uppercase
;
; Input: character in A
;
; Return: A true if upper case, else false
;=============================================
PROC isupper
    CMP 'A'             ; compare A to 'a'
    JLT isupper_fail    ; if less, return false

    CMP 'Z'             ; compare A to 'z'
    JGT isupper_fail    ; if greater, return false

    LDA TRUE            ; return true
    RET

isupper_fail:
    LDA FALSE           ; return false
    RET

;=============================================
; Desc: Convert character to uppercase
;
; Input: character in A
;
; Return: if lower, return upper case
;=============================================
PROC toupper
    PUSH A              ; save char in A
    CALL isupper        ; determine if char is upper
       
    JNE toupper_done    ; if char already upper, done

    POP A               ; retrieve char to A
    ADD 32              ; make it uppercase
    RET

toupper_done:
    POP A               ; restore A
    RET

;=============================================
; Desc: Convert character to lowercase
;
; Input: character in A
;
; Return: if upper, return lower case
;=============================================
PROC tolower
    PUSH A              ; save char in A
    CALL islower        ; determine if char is lower
        
    JNE tolower_done    ; if char already lower, done

    POP A               ; retrieve char to A
    SUB 32              ; make it lowercase
    RET

tolower_done:
    POP A               ; restore A
    RET

;=============================================
; Desc: convert string to uppercase 
;
; Input: X points to string
;
; Return: none
;=============================================
PROC strupr
    PUSH A              ; save A reg

strupr_top:
    LAX                 ; get char from string
    JEQ strupr_done     ; if null we are done

    CALL toupper        ; convert char to uppercase
    STAX                ; write it back to string
    LEAX 1              ; point to next char
    JMP strupr_top      ; repeat

strupr_done:
    POP A, PC           ; restore A and return


;=============================================
; Desc: convert string to lowercase 
;
; Input: X points to string
;
; Return: none
;=============================================
PROC strlwr
    PUSH A              ; save A reg

strlwr_top:
    LAX                 ; get char from string
    JEQ strlwr_done     ; if null we are done

    CALL tolower        ; convert char to lowercase
    STAX                ; write it back to string
    LEAX 1              ; point to next char
    JMP strlwr_top      ; repeat

strlwr_done:
    POP A, PC           ; restore A and return
