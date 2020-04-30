#pragma once

#ifndef __CPU_CISC_H
#define __CPU_CISC_H

#define LOBYTE(val) ((val) & 0xFF)
#define HIBYTE(val) (((val) & 0xFF00) >> 8)

//
// Memory map
//
#define RAM_END 0xFF00
#define RESET_VECTOR 0xFFFE
#define INT_VECTOR 0xFFFC
#define SWI_VECTOR 0xFFFA

enum
{
	OP_NOP,

	// arithmetic
	OP_ADD,	OP_ADDI,
	OP_SUB,	OP_SUBI,
	//OP_INC,
	//OP_DEC,

	// logical
	OP_AND, OP_ANDI,
	OP_OR, OP_ORI,
	OP_NOT, OP_NOTI,
	OP_XOR, OP_XORI,
	//OP_ROL, OP_ROR,
	//OP_SHL, OP_SHR,

	// branching
	OP_CALL,
	OP_RET,
	OP_RTI,
	OP_JMP,
	OP_JNE,
	OP_JEQ,

	OP_CMP, OP_CMPI,

	// loads and stores
	OP_LDA, OP_LDAI,
	OP_LDX, OP_LDXI,
	OP_LEAX,
	OP_LAX,
	OP_STA,
	OP_STAX,

	// stack
	OP_PUSH,
	OP_POP
};

//
// Register definitions
//
enum
{
	REG_A = 1,
	REG_X = 2,
	REG_SP = 4,
	REG_CC = 8,
	REG_PC = 16
};

enum
{
	FLAG_C = 1,
	FLAG_Z = 2,
	FLAG_V = 4,
	FLAG_N = 8,
	FLAG_I = 16,
};

#endif __CPU_CISC_H
