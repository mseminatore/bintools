# CISC

The bintools CPU emulator and debug monitor.

## Design

Originally I had intended to define a simple 8-bit RISC processor. However,
programming a RISC processor in assembly is not always enjoyable. RISC 
processors are generally designed assuming that a high-level language compiler
is involved.

So instead I chose to design a simple 8-bit CISC processor and emulator. This 
design was inspired by classic microprocessors like the `6502`, `6800` and 
the `6809`. Like those processors the addressable memory space is 64KB.

However, like many microcontrollers, this CPU has a 
[Harvard architecture](https://en.wikipedia.org/wiki/Harvard_architecture). 
Which means that there are separate 64KB address spaces for instruction code 
`I` and data `D`. The `I` space is read-only memory and the `D` is read-write 
memory. You can think of these two memory spaces as representing ROM and RAM.

## Registers

I wanted to keep the CPU as simple as possible. So there are a very small 
number of registers. They are:

Register | Description | Size
-------- | ----------- | ----
A | Accumulator | 8-bit
CC | Condition Codes | 8-bit
X | Index register | 16-bit
Y | Index register | 16-bit
SP | Stack Pointer | 16-bit
PC | Program counter | 16-bit

The `A` register is the arithmetic accumulator. Most operations involve the 
use of the accumulator. Like the `6502` and unlike the `6800` and `6809` there
is only a single accumulator register.

The `X` and `Y` registers are index registers. They are usually used as an 
index of, or pointer to, memory locations.

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
C | Carry flag - last arithmetic operation caused a carry/borrow
Z | Zero flag - last operation produced a zero result
V | Overflow flag - last arithmetic operation caused a signed overflow
N | Negative flag - last operation produced a negative number
I | Interrupt flag - interrupts are disabled

## Instruction Design

To keep things simple I tried to avoid including any extra instructions which 
were not strictly necessary. For example, rather than include a compare 
instruction `CMP` to compare a value to the accumulator `A` a typical 
replacement is to `SUB` the value from `A` and branch based on the result. The
tradeoff then is the need to frequently preserve `A` via a `PUSH` to the stack
followed by a restore of `A` via a subsequent `POP`.

Similarly, there are no special register transfer functions. All such moves can
be accomplished by an apropriate `PUSH` and `POP` via the stack.

Finally, many conditional branches we left out because they can be implemented
in terms of other conditional branches. For example, a jump if greater or equal
to `JGE` is not needed as it can be implmented by the appropriate logic with a
jump if less than `JLT`. And Likewise with `JLE` and `JGT`.

> Hopefully it is clear that if a value is not less than a value, then it must 
> therefore be greater than or equal to that value.

## Instruction Set

Instruction opcodes are all 8-bits in length. Each opcode is followed by zero 
or more bytes containing operands.

Instruction | Description | Flags
----------- | ----------- | -----
AAX | add A to X | CZNV
AAY | add A to Y | CZNV
ADD | add memory/immediate byte into A | CZNV
ADC | add memory/immediate byte into A with carry | | CZNV
AND | logical AND of A and memory/immediate | ZNV
BRK | breakpoint interrupt | I
CALL | branch to a subroutine | (none)
IN | input a byte to A from an IO port | (none)
LAX | load A using X as a pointer | ZNV
LAY | load A using Y as a pointer | ZNV
LDA | load A from memory/immediate | ZNV
LDX | load X from memory/immediate | ZNV
LDY | load X from memory/immediate | ZNV
LXX | load X from addres pointed to by X | ZNV
LYY | load Y from addres pointed to by Y | ZNV
JEQ | jump on equal | (none)
JGT | jump if greater than | (none)
JLT | jump if less than | (none)
JMP | unconditional jump | (none)
JNE | jump if not equal | (none)
NOP | no or null operation | (none)
NOT | (not yet implemented) |
OR | logical OR of A and memory/immediate | ZNV
OUT | output byte in A to an IO port | (none)
POP | pop one or more registers from the stack | (none)
PUSH | push one or more registers onto the stack | (none)
RET | return from subroutine | (none)
RTI | return from interrupt | (none)
SBB | subtract with borrow | CZNV
STA | store A to memory | (none)
STAX | store A using X as a pointer | (none)
STAY | store A using Y as a pointer | (none)
STX | store X to memory | (none)
STY | store Y to memory | (none)
SUB | subtract memory/immediate from A | CZNV
SWI | software interrupt | I
XOR | logical XOR of A and memory/immediate | ZNV

## Interrupts

The processor supports interrupts. Interrupts save the current processor state
and branch to an interrupt handler. Interrupts are vectored through a table at
the top of RAM. Entering an interrupt sets the `I` or interrupt flag in `CC`.

> A new interrupt cannot occur while an interrupt is being serviced so 
> interrupt handlers must be short to avoid missing an interrupt.

Interrupt | Vector address
--------- | --------------
BRK | 0xFFF8
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
| $FFFA - SWI software interrupt vector                    |
+----------------------------------------------------------+
| $FFF8 - BRK software interrupt vector                    |
+----------------------------------------------------------+
| $FF01 - $FFF7 (reserved for interrupt vectors)           |
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
s | single step the processor
y | clear all breakpoints
y name | clear breakpoint at name
