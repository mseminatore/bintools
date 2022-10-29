#pragma once

#ifndef __CPU_CISC_H
#define __CPU_CISC_H

//
// Memory map
//

// interrupt vectors
#define RESET_VECTOR	0xFFFE	// addr of reset interrupt handler
#define INT_VECTOR		0xFFFC	// addr of timer interrupt handler
#define SWI_VECTOR		0xFFFA	// addr of software interrupt handler
#define BRK_VECTOR		0xFFF8	// addr of breakpoint interrupt

// MMIO
#define MMIO_TIMER_REG	0xFF00	// current value of timer
#define MMIO_TIMER_ENA	0xFF01	// timer enabled/disabled
#define MMIO_TIMER_LIM	0xFF02	// timer limit

#define RAM_END			0xFE00	// above this address is reserved (e.g. MMIO, interrupt vectors)

//
// Machine word sizes
//
#define BYTE_SIZE	1
#define WORD_SIZE	2
#define DWORD_SIZE	4

// Opcodes
enum
{
	OP_NOP,		// no operation

	// arithmetic
	OP_ADD,		// A = A + memory
	OP_ADDI,	// A = A + immediate
	OP_ADC,		// A = A + memory + C
	OP_ADCI,	// A = A + immediate + C

	OP_AAX,		// X = X + A
	OP_AAY,		// Y = Y + A

	OP_SUB,		// A = A - memory
	OP_SUBI,	// A = A - immediate
	OP_SBB,		// A = A - memory - C
	OP_SBBI,	// A = A - immediate - C

	OP_CMP,		// temp = A - memory
	OP_CMPI,	// temp = A - immediate

	OP_CMPX,	// temp = X - memory
	OP_CMPXI,	// temp = X - immediate

	OP_CMPY,	// temp = Y - memory
	OP_CMPYI,	// temp = Y - immediate

	// logical
	OP_AND, 
	OP_ANDI,

	OP_OR, 
	OP_ORI,

	OP_NOT,		// A = ~A

	OP_XOR, 
	OP_XORI,
	
	//OP_ROL, 
	// OP_ROR,

	OP_SHL,		// A <<= 1
	OP_SHR,		// A >>= 1

	// branching
	OP_CALL,	// branch to a function
	OP_RET,		// return from function
	OP_RTI,		// return from interrupt
	OP_JMP,		// unconditional jump
	OP_JNE,		// jump if not equal
	OP_JEQ,		// jump if equal
	OP_JGT,		// jump if greater than
	OP_JLT,		// jump if less than

	// loads and stores
	OP_LDA,		// load A from memory
	OP_LDAI,	// load A from immediate value

	OP_LDX,		// load X from memory
	OP_LDY,		// load Y from memory
	OP_LDXI,	// load X from immediate value
	OP_LDYI,	// load Y from immediate value

	OP_LEAX,	// load X = X + immediate value
	OP_LEAY,	// load Y = Y + immediate value
	OP_LAX,		// load A from [X]
	OP_LAY,		// load A from [Y]

	OP_LXX,		// load X from [X]
	OP_LYY,		// load X from [Y]

	OP_STA,		// store A to memory
	OP_STX,		// store X to memory
	OP_STY,		// store Y to memory

	OP_STAX,	// store A to [X]
	OP_STAY,	// store A to [Y]

	OP_STYX,	// store Y to [X]
	OP_STXY,	// store X to [Y]

	// stack
	OP_PUSH,	// push one or more registers on the stack
	OP_POP,		// pop one or more registers from the stack

	// IO
	OP_OUT,		// output a byte to a port
	OP_IN,		// input a byte from a port

	// software interrupts
	OP_BRK,		// breakpoint interrupt
	OP_SWI,		// software interrupt
};

//
// Register bit definitions
//
enum
{
	REG_A = 1,		// bit 0
	REG_X = 2,		// bit 1
	REG_Y = 4,		// bit 2
	REG_SP = 8,		// bit 3
	REG_CC = 16,	// bit 4
	REG_PC = 32		// bit 5
};

//
// Flag bit definitions
//
enum
{
	FLAG_C = 1,		// carry flag, bit 0
	FLAG_Z = 2,		// zero flag, bit 1
	FLAG_V = 4,		// overflow flag, bit 2
	FLAG_N = 8,		// negative flag, bit 3
	FLAG_I = 16,	// interrupt flag, bit 4
	FLAG_S = 32,	// single step flag, bit 5

	FLAG_ALL = 0xFF
};

//
// Interrupt definitions
//
enum
{
	INT_TIMER = 1
};

#endif // __CPU_CISC_H
