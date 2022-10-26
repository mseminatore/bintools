# ASM

The bintools assembler.

## Assembly language instructions

You can find a complete list of available instructions 
[here](https://github.com/mseminatore/bintools/tree/master/cisc#instructions)

## Using the assembler

To assemble a file:

```

bintools> asm main.asm
Warnings generated: 0
Assembly complete main.asm -> a.out

bintools>

```

## Assembly code examples

I wanted to provide a few code examples to illustrate usage. These are taken 
from a small library of routines that I've developed as part of building and
testing the bintools project. A semi-colon marks a single-line comment.

Here is a small function to check whether a character is lowercase. As the
comments show, the character is passed in the A register. The boolean result
is returned in A.

```

; define some useful constants
TRUE    EQU 1
FALSE   EQU 0

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

```

First, note the constant definitions via the `EQU` keyword. This is a way to
define symbolic names for numberic values. Next, note that functions begin with
the `PROC` statement followed by the name of the function. This defines a 
symbol so that other routines can call this one. Next, the code compares the 
character to lowercase 'a', if the character is less than 'a' we return a false
value in `A`. Similarly if the character is greater than 'z' we return false. 
Otherwise we return true in `A`.

Below is an example of a function to compute the length of a 
null-terminated string.

```
;===============================
; Desc: Return the length of string
;
; Input: X points to the string
;
; Return: A contains the length
;===============================
PROC strlen
    PUSH X              ; save X

    LDA 0               ; count = 0
    PUSH A              ; save counter on stack

strlen_loop:
    LAX                 ; load char from string using X as pointer
    JEQ strlen_done     ; if null we are done

    POP A               ; retrieve the counter
    ADD 1               ; count++
    PUSH A              ; put count back on the stack

    LEAX 1              ; inc X
    JMP strlen_loop     ; do it again

strlen_done:
    POP A, X, PC        ; restore A, X and return

```

Note the use of the `POP` instruction to pop multiple values off the stack 
including the return address. You can both `PUSH` and `POP` multiple registers
from the stack in single instructions. The order of registers is pre-determined
for consistency. Doing this avoids the need for an explicit `RET` instruction 
saving one byte of code. This is a common pattern in the libraries I've 
written.

## Assembler directives

There are a number of assembler directives. These are listed below.

Directive | Description
--------- | -----------
INCLUDE "filename.inc" | include the contents of the file
EQU | EQUates a symbol name to a value (e.g. foo EQU 0xFF)
DB | defines and optionally names a byte in the data seg (eg. size DB 1)
DW | defines and optionally names a word in the data seg (eg. count DW 1024)
DS | defines and optionally names a string in the data seg (eg. prompt DS "Hi!")
PROC | defines and names the start of a subroutine (eg. PROC _main)
