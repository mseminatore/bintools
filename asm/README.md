# ASM

The bintools assembler.

## Assembly language instructions

You can find a complete list of available instructions [here](https://github.com/mseminatore/bintools/tree/master/cisc#instructions)

## Using the assembler

To assemble a file 

```
bintools> asm main.asm

```

## Assembler directives

There are a number of assembler directives

Directive | Description
--------- | -----------
INCLUDE "filename.inc" | include the contents of the file
EQU | EQUates a symbol name to a value (e.g. foo EQU 0xFF)
DB | defines and optionally names a byte in the data seg (eg. size DB 1)
DW | defines and optionally names a word in the data seg (eg. count DW 1024)
DS | defines and optionally names a string in the data seg (eg. prompt DS "Hi!")
PROC | defines and names the start of a subroutine (eg. PROC _main)
