#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include "../cpu_cisc.h"
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <set>
#include <signal.h>

// Flag bit helper functions
#define SETF(flag) (CC |= flag)
#define CLRF(flag) (CC &= ~flag)
#define TSTF(flag) ((CC & flag) != 0)

//
static const int SMALL_BUFFER = 256;

// define our CPU arch
class Cisc
{
protected:
	// 8-bit registers
	uint8_t A, CC;

	// 16-bit registers
	uint16_t PC, SP, X, Y;
	
	// memory
	uint8_t ram[0x10000];
	uint8_t rom[0x10000];

	// processor state
	uint16_t maxStack;
	uint16_t __brk;

	// instruction buffer
	uint8_t opcode;

	void log(const char *fmt, ...);

	void pushRegs();
	void popRegs();

	using BreakpointList = std::set<uint32_t>;
	BreakpointList breakpoints;

	ObjectFile obj;
	
public:
	Cisc() {
		reset();
	}

	virtual ~Cisc() {}

	void load(const std::string &filename);

	void reset() 
	{ 
		A = CC = opcode = 0; 
		X = Y = 0;

		ram[RESET_VECTOR] = 0;
		ram[RESET_VECTOR + 1] = 0;

		PC = ram[RESET_VECTOR];
		SP = RAM_END;

		maxStack = SP;
		ram[MMIO_TIMER_REG] = 0;
		ram[MMIO_TIMER_ENA] = 0;
		ram[MMIO_TIMER_LIM] = 0;
	}

	uint32_t checkOverflow(uint16_t val);
	void inputByte();
	void outputByte();

	uint16_t getMaxStack() { return RAM_END - maxStack; }
	void push(uint8_t val);
	uint8_t pop();
	void getRegisterList(uint8_t operand, std::string&);
	void panic();
	uint16_t fetchW();
	uint8_t fetch();
	void decode();
	void pushAll();
	void popAll();
	void updateFlag(uint32_t result, uint8_t flag);

	uint8_t exec();
	uint8_t tick();

	void interrupt(uint32_t vector);

	uint8_t getCC() const	{ return CC;  }
	void setCC(uint8_t cc)	{ CC = cc; }

	// breakpoints
	uint16_t getPC() const { return PC; }
	bool getSymbolAddress(const std::string &name, uint16_t &addr);
	bool getCodeSymbolName(uint16_t addr, std::string &name);
	void addBreakpoint(uint16_t addr) { breakpoints.insert(addr); }

	void clearAllBreakpoints() { breakpoints.clear(); }

	bool clearBreakpoint(const std::string &name)
	{
		uint16_t addr = 0;
		if (getSymbolAddress(name, addr))
		{
			breakpoints.erase(addr);
			log("breakpoint deleted @ %s (0x%04X)", name.c_str(), addr);

			return true;
		}

		return false;
	}

	void listBreakpoints();

	bool setBreakpoint(const std::string &name)
	{
		uint16_t addr = 0;
		if (getSymbolAddress(name, addr))
		{
			addBreakpoint(addr);
			log("breakpoint set @ %s (0x%04X)", name.c_str(), addr);

			return true;
		}
		
		return false;
	}

	bool isBreakpoint(uint16_t addr)
	{
		if (breakpoints.find(addr) != breakpoints.end())
			return true;
		
		return false;
	}

	uint16_t getAddressFromToken(char *tok);

	//
	void printRegisters()
	{
		printf("A: %02X X: %04X Y: %04X CC: %02X SP: %04X PC: %04X\n", A, X, Y, CC, SP, PC);
		printf("Flags C: %d Z: %d V: %d N: %d I: %d S: %d\n", TSTF(FLAG_C), TSTF(FLAG_Z), TSTF(FLAG_V), TSTF(FLAG_N), TSTF(FLAG_I), TSTF(FLAG_S));
	}

	void printByte(uint16_t addr)
	{
		printf("%d (0x%04X) points to -> %d (0x%02X)\n", addr, addr, ram[addr], ram[addr]);
	}

	void printWord(uint16_t addr)
	{
		uint16_t value = ram[addr] + (ram[addr + 1] << 8);

		printf("%d (0x%04X) points to -> %d (0x%04X)\n", addr, addr, value, value);
	}

	void dumpMemoryAt(uint32_t addr)
	{
		hexDumpLine(stdout, addr, &ram[addr]);
		hexDumpLine(stdout, addr + 16, &ram[addr + 16]);
	}

	void reportLocation()
	{
		std::string name;
		uint16_t addr;

		if (obj.findNearestCodeSymbolToAddr(PC, name, addr))
		{
			if (PC > addr)
				log("stopped @ %s + %d (0x%04X)", name.c_str(), PC - addr, PC);
			else
				log("stopped @ %s (0x%04X)", name.c_str(), PC);
		}
		else
			log("stopped @ 0x%X", PC);
	}
};

Cisc cpu;

// print out all current breakpoints
void Cisc::listBreakpoints()
{
	std::string name;

	for (auto it = breakpoints.begin(); it != breakpoints.end(); it++)
	{
		if (getCodeSymbolName(*it, name))
			printf("breakpoint @ %s (0x%04X)\n", name.c_str(), *it);
		else
			printf("breakpoint @ 0x%04X\n", *it);
	}
}

//
bool Cisc::getSymbolAddress(const std::string &name, uint16_t &addr)
{
	SymbolEntity se;

	if (!obj.findSymbol(name, se))
		return false;

	addr = se.value;
	return true;
}

//
bool Cisc::getCodeSymbolName(uint16_t addr, std::string &name)
{
	return obj.findCodeSymbolByAddr(addr, name);
}

// load an executable file into ROM/RAM
void Cisc::load(const std::string &filename)
{
	printf("Loading file: %s\n", filename.c_str());

	obj.readFile(filename);

	PC = obj.getEntryPoint();

	// populate ram
	memset(ram, 0, 0xFFFF);
	memcpy(ram, obj.dataPtr(), obj.getDataSize());

	// populate rom
	memset(rom, 0, 0xFFFF);
	memcpy(rom, obj.textPtr(), obj.getTextSize());

	SymbolEntity se;
	if (obj.findSymbol("__brk", se))
	{
		__brk = ram[se.value] + (ram[se.value + 1] << 8);
		printf("Found stack __brk limit of: 0x%04X\n", __brk);
	}
}

// stack is full descending
// push a value onto the stack
void Cisc::push(uint8_t val)
{
	SP--;

	if (SP < maxStack)
		maxStack = SP;

	//if (SP < __brk)
	//{
	//	puts("Stack overflow!\n");
	//	panic();
	//}

	ram[SP] = val;
}

//
uint8_t Cisc::pop()
{
	uint8_t val = ram[SP++];
	return val;
}

//
void Cisc::getRegisterList(uint8_t operand, std::string &str)
{
	if (operand & REG_A)
		str = "A ";
	if (operand & REG_X)
		str += "X ";
	if (operand & REG_Y)
		str += "Y ";
	if (operand & REG_CC)
		str += "CC ";
	if (operand & REG_SP)
		str += "SP ";
	if (operand & REG_PC)
		str += "PC ";
}

//
void Cisc::log(const char *fmt, ...)
{
	if (!TSTF(FLAG_S))
		return;

	char buf[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
		vsprintf(buf, fmt, argptr);
	va_end(argptr);

	printf("%s\n", buf);
}

//
void Cisc::pushRegs()
{
	uint8_t operand = fetch();
	uint16_t addr;

	if (operand & REG_PC)
	{
		push(HIBYTE(PC));
		push(LOBYTE(PC));
	}

	if (operand & REG_SP)
	{
		addr = SP;
		push(HIBYTE(addr));
		push(LOBYTE(addr));
	}

	if (operand & REG_X)
	{
		push(HIBYTE(X));
		push(LOBYTE(X));
	}

	if (operand & REG_Y)
	{
		push(HIBYTE(Y));
		push(LOBYTE(Y));
	}

	if (operand & REG_A)
		push(A);

	if (operand & REG_CC)
		push(CC);

	std::string s;
	getRegisterList(operand, s);
	log("PUSH %s", s.c_str());
}

//
void Cisc::popRegs()
{
	uint8_t operand = fetch();

	if (operand & REG_CC)
		CC = pop();

	if (operand & REG_A)
		A = pop();

	if (operand & REG_Y)
	{
		Y = pop() | (pop() << 8);
	}

	if (operand & REG_X)
	{
		X = pop() | (pop() << 8);
	}

	if (operand & REG_SP)
	{
		SP = pop() | (pop() << 8);
	}

	if (operand & REG_PC)
	{
		PC = pop() | (pop() << 8);
	}

	std::string s;
	getRegisterList(operand, s);
	log("POP %s", s.c_str());
}

//
void Cisc::updateFlag(uint32_t result, uint8_t flag)
{
	if (result)
		SETF(flag);
	else
		CLRF(flag);
}

//
uint32_t Cisc::checkOverflow(uint16_t val)
{
	auto check = val & 0x180;
	if (check == 0x100 || check == 0x80)
		return 1;

	return 0;
}

// Handle IO input
void Cisc::inputByte()
{
	auto port = fetch();

	switch (port)
	{
	case 1:
		A = getchar();
		break;

	default:
		// do nothing!
		break;
	}

	log("IN %d", port);
}

// Handle IO output
void Cisc::outputByte()
{
	auto port = fetch();

	switch (port)
	{
	case 1:
		putchar(A);
		break;

	default:
		// do nothing!
		break;
	}

	log("OUT %d", port);
}

//
char makePrintable(char c)
{
	if (isprint(c))
		return c;

	return '?';
}

// execute the next instruction
uint8_t Cisc::exec()
{
	std::string name;
	uint8_t operand;
	uint16_t addr;
	uint16_t temp16;

	switch (opcode)
	{
	case OP_NOP:
		log("NOP");
		break;

	case OP_SHL:
		operand = fetch();

		temp16 = A << operand;

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(checkOverflow(temp16), FLAG_V);

		A = temp16 & 0xFF;

		log("SHL %d", operand);
		break;

	case OP_SHR:
		operand = fetch();

		temp16 = A & 0x80;	// save top bit 7

		updateFlag(A & 1, FLAG_C);

		A = A >> operand;
		A = A | temp16;		// restore top bit 7

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);

		log("SHR %d", operand);
		break;

	case OP_ADD:
		addr = fetchW();
		temp16 = A + ram[addr];

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(checkOverflow(temp16), FLAG_V);

		A = temp16 & 0xFF;

		if (obj.findDataSymbolByAddr(addr, name))
			log("ADD [%s]", name.c_str());
		else
			log("ADD [0x%X]", addr);
		break;

	case OP_ADDI:
		operand = fetch();
		temp16 = A + operand;

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(checkOverflow(temp16), FLAG_V);

		A = temp16 & 0xFF;

		log("ADD 0x%X (%d)", operand, operand);
		break;

	case OP_ADC:
		addr = fetchW();
		temp16 = A + ram[addr] + (TSTF(FLAG_C) ? 1 : 0);

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(checkOverflow(temp16), FLAG_V);

		A = temp16 & 0xFF;

		if (obj.findDataSymbolByAddr(addr, name))
			log("ADC [%s]", name.c_str());
		else
			log("ADC [0x%X]", addr);
		break;

	case OP_ADCI:
		operand = fetch();
		temp16 = A + operand + (TSTF(FLAG_C) ? 1 : 0);

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(checkOverflow(temp16), FLAG_V);

		A = temp16 & 0xFF;

		log("ADC 0x%X (%d)", operand, operand);
		break;

	case OP_AAX:
		X = X + A;

		log("AAX");
		break;

	case OP_AAY:
		Y = Y + A;

		log("AAY");
		break;

	case OP_CMP:
		addr = fetchW();
		temp16 = A - ram[addr];

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(checkOverflow(temp16), FLAG_V);

		// Note: we discard the result!

		if (obj.findDataSymbolByAddr(addr, name))
			log("CMP [%s]", name.c_str());
		else
			log("CMP [0x%X]", addr);
		break;

	case OP_CMPI:
		operand = fetch();
		temp16 = A - operand;

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(checkOverflow(temp16), FLAG_V);

		// Note: dwe iscard the result!

		log("CMP %d\t;'%c'\t0x%X", operand, makePrintable(operand), operand);
		break;

	case OP_SUB:
		addr = fetchW();
		temp16 = A - ram[addr];

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(checkOverflow(temp16), FLAG_V);

		A = temp16 & 0xFF;

		if (obj.findDataSymbolByAddr(addr, name))
			log("SUB [%s]", name.c_str());
		else
			log("SUB [0x%X]", addr);
		break;

	case OP_SUBI:
		operand = fetch();
		temp16 = A - operand;

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(checkOverflow(temp16), FLAG_V);

		A = temp16 & 0xFF;

		log("SUB 0x%X", operand);
		break;

	case OP_SBB:
		addr = fetchW();
		temp16 = A - ram[addr] - (TSTF(FLAG_C) ? 1 : 0);

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(checkOverflow(temp16), FLAG_V);

		A = temp16 & 0xFF;

		if (obj.findDataSymbolByAddr(addr, name))
			log("SBB [%s]", name.c_str());
		else
			log("SBB [0x%X]", addr);
		break;

	case OP_SBBI:
		operand = fetch();
		temp16 = A - operand - (TSTF(FLAG_C) ? 1 : 0);

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(checkOverflow(temp16), FLAG_V);

		A = temp16 & 0xFF;

		log("SBB 0x%X", operand);
		break;

	case OP_AND:
		addr = fetchW();
		A = A & ram[addr];

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		if (obj.findDataSymbolByAddr(addr, name))
			log("AND [%s]", name.c_str());
		else
			log("AND [0x%X]", addr);
		break;
	
	case OP_ANDI:
		operand = fetch();
		A = A & operand;

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("AND 0x%X", operand);
		break;

	case OP_OR:
		addr = fetchW();
		A = A | ram[addr];

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		if (obj.findDataSymbolByAddr(addr, name))
			log("OR [%s]", name.c_str());
		else
			log("OR [0x%X]", addr);
		break;

	case OP_ORI:
		operand = fetch();
		A = A | operand;

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("OR 0x%X", operand);
		break;

	case OP_XOR:
		addr = fetchW();
		A = A % ram[addr];

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		if (obj.findDataSymbolByAddr(addr, name))
			log("XOR [%s]", name.c_str());
		else
			log("XOR [0x%X]", addr);
		break;

	case OP_XORI:
		operand = fetch();
		A = A % operand;

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("XOR 0x%X", operand);
		break;

	case OP_NOT:
		operand = fetch();
		A = ~A;

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);
		updateFlag(1, FLAG_C);

		log("NOT");
		break;

	case OP_CALL:
		addr = fetchW();

		push(HIBYTE(PC));
		push(LOBYTE(PC));

		PC = addr;

		if (obj.findCodeSymbolByAddr(addr, name))
			log("CALL %s", name.c_str());
		else
			log("CALL %s (0x%X)", name.c_str(), PC);
		break;
	
	case OP_RET:
		PC = pop() | (pop() << 8);

		log("RET");
		break;

	case OP_RTI:
		popAll();

		log("RTI");
		break;

	case OP_JMP:
		PC = fetchW();

		if (obj.findCodeSymbolByAddr(PC, name))
			log("JMP %s", name.c_str());
		else
			log("JMP 0x%X", PC);
		break;

	case OP_JNE:
		addr = fetchW();
		if (!TSTF(FLAG_Z))
			PC = addr;

		if (obj.findCodeSymbolByAddr(addr, name))
			log("JNE %s", name.c_str());
		else
			log("JNE 0x%X", addr);
		break;

	case OP_JEQ:
		addr = fetchW();
		if (TSTF(FLAG_Z))
			PC = addr;

		if (obj.findCodeSymbolByAddr(addr, name))
			log("JEQ %s", name.c_str());
		else
			log("JEQ 0x%X", addr);
		break;

	case OP_JGT:
		addr = fetchW();
		if (!TSTF(FLAG_Z) && ( (TSTF(FLAG_N) && TSTF(FLAG_V)) || (!TSTF(FLAG_N) && !TSTF(FLAG_V)) ) )
			PC = addr;

		if (obj.findCodeSymbolByAddr(addr, name))
			log("JGT %s", name.c_str());
		else
			log("JGT 0x%X", addr);
		break;

	case OP_JLT:
		addr = fetchW();
		if ( (TSTF(FLAG_N) || TSTF(FLAG_V)) && !(TSTF(FLAG_N) && TSTF(FLAG_V)) )
			PC = addr;

		if (obj.findCodeSymbolByAddr(addr, name))
			log("JLT %s", name.c_str());
		else
			log("JLT 0x%X", addr);
		break;

	case OP_LAX:
		A = ram[X];

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LAX");
		break;

	case OP_LAY:
		A = ram[Y];

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LAY");
		break;

	case OP_LDA:
		addr = fetchW();
		A = ram[addr];

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		if (obj.findDataSymbolByAddr(addr, name))
			log("LDA [%s]", name.c_str());
		else
			log("LDA [0x%X]", addr);
		break;

	case OP_LDAI:
		operand = fetch();
		A = operand;

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LDA 0x%X", operand);
		break;

	case OP_LDX:
		addr = fetchW();
		X = ram[addr] + (ram[addr + 1] << 8);

		updateFlag(X == 0, FLAG_Z);
		updateFlag(X & 0x8000, FLAG_N);
		updateFlag(0, FLAG_V);

		if (obj.findDataSymbolByAddr(addr, name))
			log("LDX [%s]", name.c_str());
		else
			log("LDX [0x%X]", addr);
		break;

	case OP_LDXI:
		X = fetchW();

		updateFlag(X == 0, FLAG_Z);
		updateFlag(X & 0x8000, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LDX 0x%X", X);
		break;

	case OP_LDY:
		addr = fetchW();
		Y = ram[addr] + (ram[addr + 1] << 8);

		updateFlag(Y == 0, FLAG_Z);
		updateFlag(Y & 0x8000, FLAG_N);
		updateFlag(0, FLAG_V);

		if (obj.findDataSymbolByAddr(addr, name))
			log("LDY [%s]", name.c_str());
		else
			log("LDY [0x%X]", addr);
		break;

	case OP_LDYI:
		Y = fetchW();

		updateFlag(Y == 0, FLAG_Z);
		updateFlag(Y & 0x8000, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LDY 0x%X", Y);
		break;

	case OP_LEAX:
		operand = fetch();
		X = (int)X + (char)operand;

		updateFlag(X == 0, FLAG_Z);

		log("LEAX %d", (char)operand);
		break;

	case OP_LEAY:
		operand = fetch();
		Y = (int)Y + (char)operand;

		updateFlag(Y == 0, FLAG_Z);

		log("LEAY %d", (char)operand);
		break;

	case OP_LXX:
		X = ram[X] + (ram[X + 1] << 8);

		updateFlag(X == 0, FLAG_Z);
		updateFlag(X & 0x8000, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LXX");
		break;

	case OP_LYY:
		Y = ram[Y] + (ram[Y + 1] << 8);

		updateFlag(Y == 0, FLAG_Z);
		updateFlag(Y & 0x8000, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LYY");
		break;

	case OP_STA:
		addr = fetchW();
		ram[addr] = A;

		if (obj.findDataSymbolByAddr(addr, name))
			log("STA %s", name.c_str());
		else
			log("STA 0x%X", addr);
		break;

	case OP_STX:
		addr = fetchW();
		ram[addr] = LOBYTE(X);
		ram[addr + 1] = HIBYTE(X);

		if (obj.findDataSymbolByAddr(addr, name))
			log("STX %s", name.c_str());
		else
			log("STX 0x%X", addr);
		break;

	case OP_STY:
		addr = fetchW();
		ram[addr] = LOBYTE(Y);
		ram[addr + 1] = HIBYTE(Y);

		if (obj.findDataSymbolByAddr(addr, name))
			log("STY %s", name.c_str());
		else
			log("STY 0x%X", addr);
		break;

	case OP_STAX:
		ram[X] = A;

		log("STAX");
		break;

	case OP_STAY:
		ram[Y] = A;

		log("STAY");
		break;

	case OP_STYX:
		ram[X] = LOBYTE(Y);
		ram[X + 1] = HIBYTE(Y);

		log("STYX");
		break;

	case OP_STXY:
		ram[Y] = LOBYTE(X);
		ram[Y + 1] = HIBYTE(X);

		log("STXY");
		break;

	case OP_PUSH:
		pushRegs();
		break;

	case OP_POP:
		popRegs();
		break;

	case OP_OUT:
		outputByte();
		break;

	case OP_IN:
		inputByte();
		break;

	case OP_SWI:
		interrupt(SWI_VECTOR);

		log("SWI");
		break;

	case OP_BRK:
		interrupt(BRK_VECTOR);

		log("BRK");
		break;

	default:
		panic();
	}

	return opcode;
}

// update a single CPU instruction clock tick
uint8_t Cisc::tick()
{
	// timer increments only if enabled
	if (ram[MMIO_TIMER_ENA])
	{
		ram[MMIO_TIMER_REG]++;

		// check for timer interrupts
		if (ram[MMIO_TIMER_LIM] == ram[MMIO_TIMER_REG])
			interrupt(INT_VECTOR);
	}

	opcode = fetch();

	decode();

	return exec();
}

// fetch a code byte value
uint8_t Cisc::fetch()
{
	return rom[PC++];
}

// fetch a code word value
uint16_t Cisc::fetchW()
{
	uint16_t word;

	word = fetch();
	word |= (fetch() << 8);

	return word;
}

// decode an instruction
void Cisc::decode()
{
}

// push all registers onto the stack
void Cisc::pushAll()
{
	push(HIBYTE(PC));
	push(LOBYTE(PC));

	push(HIBYTE(X));
	push(LOBYTE(X));

	push(HIBYTE(Y));
	push(LOBYTE(Y));

	push(A);

	push(CC);

	auto addr = SP;
	push(HIBYTE(addr));
	push(LOBYTE(addr));
}

// pop all registers from the stack
void Cisc::popAll()
{
	SP = pop() | (pop() << 8);

	CC = pop();

	A = pop();

	Y = pop() | (pop() << 8);
	X = pop() | (pop() << 8);

	PC = pop() | (pop() << 8);
}

// process an interrupt request
void Cisc::interrupt(uint32_t vector)
{
	// no re-entrant interrupts by default
	if (vector == INT_VECTOR && TSTF(FLAG_I))
		return;

	// save the current context
	pushAll();

	// set interrupt flag to disable interrupts
//	if (vector == INT_VECTOR)	
		SETF(FLAG_I);

	// jump to the interrupt vector
	PC = ram[vector] + (ram[vector + 1] << 8);
}

// something seriously unexpected happened
void Cisc::panic()
{
	puts("Panic!!!!!");
	printRegisters();

	while (true);

	exit(-1);
}

// show usage
void usage()
{
	puts("\nusage: cisc [options] filename\n\n");
	exit(0);
}

// get options from the command line
int getopt(int n, char *args[])
{
	int i;
	for (i = 1; args[i][0] == '-'; i++)
	{
		//if (args[i][1] == 'v')
		//	g_bDebug = true;

		//if (args[i][1] == 'o')
		//	g_bDebug = true;
	}

	return i;
}

//
uint16_t Cisc::getAddressFromToken(char *tok)
{
	uint16_t addr = 0;
	auto base = 10;
	if (!tok)
	{
		panic();
		return 0;
	}

	if (tok[0] == '$')
	{
		base = 16;
		tok++;
	}

	if (isxdigit(tok[0]))
		addr = (uint16_t)strtoul(tok, nullptr, base);
	else
	{
		if (!getSymbolAddress(tok, addr))
			printf("Symbol '%s' not found!\n", tok);
	}

	return addr;
}

// Ctrl-C pressed
void sigint(int val)
{
	cpu.setCC(cpu.getCC() | FLAG_S);
}

// Break key pressed
void sigbreak(int val)
{
	cpu.setCC(cpu.getCC() | FLAG_S);
}

//
int main(int argc, char* argv[])
{
	bool done = false;
	char buf[SMALL_BUFFER];

	if (argc == 1)
		usage();

	int iFirstArg = getopt(argc, argv);

	cpu.load(argv[iFirstArg]);

	signal(SIGINT, sigint);

#ifdef _WIN32
	signal(SIGBREAK, sigbreak);
#endif

	// start simulation in single step mode
	cpu.setCC(cpu.getCC() | FLAG_S);

	while (!done)
	{
		if (FLAG_S & cpu.getCC())
		{
			cpu.reportLocation();

			printf(">");
			fgets(buf, SMALL_BUFFER - 1, stdin);

			char *pToken = strtok(buf, " \n");

			if (!pToken || !strcmp(pToken, "s"))	// single step
				cpu.tick();
			else if (!strcmp(pToken, "r"))			// print registers
				cpu.printRegisters();
			else if (!strcmp(pToken, "q"))			// quit debugger
				done = true;
			else if (!strcmp(pToken, "g"))			// go, run program
			{
				cpu.setCC(cpu.getCC() & ~FLAG_S);
				cpu.tick();
			}
			else if (!strcmp(pToken, "n"))			// step over
			{
//				singleStep = false;
//				cpu.tick();
			}
			else if (!strcmp(pToken, "fi"))			// finish current function
			{
				cpu.setCC(cpu.getCC() & ~FLAG_S);

				while (OP_RET != cpu.tick());

				cpu.setCC(cpu.getCC() | FLAG_S);
			}
			else if (!strcmp(pToken, "m"))			// dump memory
			{
				auto tok = strtok(nullptr, " \n");
				
				if (tok)
				{
					auto addr = cpu.getAddressFromToken(tok);

					// dump memory bytes
					cpu.dumpMemoryAt(addr);
				}
			}
			else if (!strcmp(pToken, "b"))
			{
				auto tok = strtok(nullptr, " \n");

				if (tok)
					cpu.setBreakpoint(tok);
				else
					cpu.listBreakpoints();
			}
			else if (!strcmp(pToken, "y"))
			{
				auto tok = strtok(nullptr, " \n");

				if (tok)
					cpu.clearBreakpoint(tok);
				else
					cpu.clearAllBreakpoints();
			}
			else if (!strcmp(pToken, "db"))
			{
				auto tok = strtok(nullptr, " \n");
				uint16_t addr = 0;
				auto base = 10;
				if (tok)
				{
					if (tok[0] == '$')
					{
						base = 16;
						tok++;
					}

					if (isdigit(tok[0]))
						addr = (uint16_t)strtoul(tok, nullptr, base);
					else
						cpu.getSymbolAddress(tok, addr);

					cpu.printByte(addr);
				}
			}
			else if (!strcmp(pToken, "dw"))
			{
				auto tok = strtok(nullptr, " \n");
				uint16_t addr = 0;
				auto base = 10;
				if (tok)
				{
					if (tok[0] == '$')
					{
						base = 16;
						tok++;
					}

					if (isxdigit(tok[0]))
						addr = (uint16_t)strtoul(tok, nullptr, base);
					else
						cpu.getSymbolAddress(tok, addr);

					cpu.printWord(addr);
				}
			}
		}
		else
		{
			while (!(FLAG_S & cpu.getCC()))
			{ 
				auto pc = cpu.getPC();
				if (cpu.isBreakpoint(pc))
				{
					std::string name;

					cpu.getCodeSymbolName(pc, name);
					fprintf(stdout, "breakpoint hit @ %s (0x%04X)\n", name.c_str(), pc);

					cpu.setCC(cpu.getCC() | FLAG_S);
				}
				else
					cpu.tick();
			}
		}
		
	}

	printf("Max stack depth: %d\n", cpu.getMaxStack());

	return 0;
}