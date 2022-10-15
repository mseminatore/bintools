# CISC

The bintools CPU emulator and debug monitor.

## Design

Originally I had intended to define a simple 8-bit RISC processor. However,
programming a RISC processor in assembly is not always enjoyable. RISC 
processors are generally designed assuming that a high-level language compiler 
is involved.

So I chose to design a simple 8-bit CISC processor and emulator. This design 
was inspired by classic processors like the `6502`, `6800` and `6809`.

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

Instruction | Description
----------- | -----------

## Interrupts

The processor supports interrupts.

## Debug monitor