# CISC

The bintools CPU emulator and debug monitor.

## Design

Originally I had intended to define a simple 8-bit RISC processor. However,
programming a RISC processor in assembly is not always enjoyable. RISC 
processors are generally designed assuming that a high-level language compiler 
is involved.

So I chose to design a simple 8-bit CISC processor and emulator. This design 
was inspired by classic microprocessors like the `6502`, `6800` and `6809`.

## Registers

I wanted to keep the CPU as simple as possible. So there are a very small 
number of registers. They are

Register | Description | Size
-------- | ----------- | ----
A | Accumulator | 8-bit
CC | Condition Codes | 8-bit
X | Index | 16-bit
SP | Stack Pointer | 16-bit
PC | Program counter | 16-bit

The `A` register is the arithmetic accumulator. Most operations involve the 
use of the accumulator.

The `X` register is the index register. It is used as an index or pointer to
memory locations.

The `CC` register holds the condition codes or flags. Individual instructions
set or query the condition flags as needed. Note that this register 
cannot be directly addressed by instructions. However, you can modify it 
indirectly by pushing the `CC` register onto the stack, popping it into the
accumulator and then restoring it via the stack.

The `SP` or stack pointer register holds a pointer to the current position of 
the stack. The stack is a full descending type.

The `PC` or program counter register holds a pointer to the current 
instruction. It updates automatically after each instruction or branch is 
executed.

The various condition codes are described below.

Flag | Description
---- | -----------
C | Carry flag
Z | Zero flag
V | Overflow flag
N | Negative flag
I | Interrupt flag

## Instructions

Instruction opcodes are 8-bits in length and include zero or more bytes 
containing operands.

Instruction | Description
----------- | -----------
ADD | A <- A + memory
ADDI | A <- A + immediate
SUB | 
AND | 
OR | 
NOT | 
XOR |
LDA | load A from memory
LDAI | load A from immediate
LDX | load X from 
CALL | branch to a subroutine
PUSH | push one or more registers onto the stack
STA | store A
STX | store X
POP | pop one or more registers from the stack
RET | return from subroutine
RTI | return from interrupt
JMP | unconditional branch
JNE | branch on not equal
JEQ | branc on equal

## Interrupts

The processor supports interrupts. Interrupts save the processor state and 
branch to an interrupt handler. Interrupts are vectored through a table at
the top of RAM. Entering an interrupt set the `I` or interrupt flag in `CC`.

> A new interrupt cannot occur while an interrupt is being services so 
> interrupt handlers must be short to avoid missing an interrupt.

Interrupt | Vector address
--------- | --------------
SWI | 0xFFFA
Timer | 0xFFFC
Reset | 0xFFFE

On a return from interrupt `RTI` the processor state is restored.

## Memory map

The default memory map is shown below. The start of code can be changed by 
setting a non-zero base address in the linker.

```
+----------------------------------------------------------+
| $FFFE - reset vector                                     |
+----------------------------------------------------------+
| $FFFC - interrupt vector                                 |
+----------------------------------------------------------+
| $FFFA - software interrupt vector                        |
+----------------------------------------------------------+
| $FF01 - $FFF9 (reserved for interrupt vectors)           |
+----------------------------------------------------------+
| $FF00 - top of stack                                     |
+----------------------------------------------------------+
| $E000 - top of heap, bottom of stack (__brk)             |
+----------------------------------------------------------+
| __ram_start - bottom of heap                             |
+----------------------------------------------------------+
| $0000 - start of code segment, data segment, bss segment |
+----------------------------------------------------------+
```

## Debug monitor

The debug monitor supports a number of commands. Where practical I tried to 
follow gdb or lldb conventions.

Command | Description
------- | -----------
b | list breakpoints
b name | set breakpoint at name
db name | dump byte at name
dw name | dump word at name
g | go, run the program
m name | dump memory at name
q | quit
r | print registers
s | single step
y | clear breakpoints
y name | clear breakpoint at name
