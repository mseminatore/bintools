#pragma once

#ifndef __CPU_CISC_H
#define __CPU_CISC_H

//
// Memory map
//
#define RAM_END			0xFF00	// above this address is reserved for interrupt vectors
#define RESET_VECTOR	0xFFFE	// addr of reset interrupt handler
#define INT_VECTOR		0xFFFC	// addr of timer interrupt handler
#define SWI_VECTOR		0xFFFA	// addr of software interrupt handler
#define BRK_VECTOR		0xFFF8	// addr of breakpoint interrupt

enum
{
	OP_NOP,		// no operation

	// arithmetic
	OP_ADD,		// A = A + memory
	OP_ADDI,	// A = A + immediate
	OP_AAX,		// X = X + A

	OP_SUB,		// A = A - memory
	OP_SUBI,	// A = A - immediate

	// logical
	OP_AND, OP_ANDI,
	OP_OR, OP_ORI,
	OP_NOT, OP_NOTI,
	OP_XOR, OP_XORI,
	//OP_ROL, OP_ROR,
	//OP_SHL, OP_SHR,

	// branching
	OP_CALL,	// branch to a function
	OP_RET,		// return from function
	OP_RTI,		// return from interrupt
	OP_JMP,		// unconditional jump
	OP_JNE,		// jump if not equal
	OP_JEQ,		// jump if equal

	// loads and stores
	OP_LDA,		// load A from memory
	OP_LDAI,	// load A from immediate value

	OP_LDX,		// load X from memory
	OP_LDXI,	// load X from immediate value

	OP_LEAX,	// load X = X + immediate value
	OP_LAX,		// load A from [X]

	OP_STA,		// store A to memory
	OP_STX,		// store X to memory

	OP_STAX,	// store A to [X]

	OP_LXX,		// load X from [X]

	// stack
	OP_PUSH,	// push one or more registers on the stack
	OP_POP,		// pop one or more registers from the stack
	
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
	FLAG_C = 1,		// bit 0
	FLAG_Z = 2,		// bit 1
	FLAG_V = 4,		// bit 2
	FLAG_N = 8,		// bit 3
	FLAG_I = 16,	// bit 4

	FLAG_ALL = 0xFF
};

//
// Interrupt definitions
//
enum
{
	INT_TIMER = 1
};

#endif __CPU_CISC_H
