#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include "../cpu_cisc.h"
#include <stdio.h>
#include <stdarg.h>
//#include <vector>
#include <set>

//
//
//
#define SETF(flag) (CC |= flag)
#define CLRF(flag) (CC &= ~flag)
#define TSTF(flag) (CC ^ flag) 

//
//
//
class Cisc
{
protected:
	uint8_t A, CC;
	uint16_t PC, SP, X;
	
	uint8_t ram[0x10000];
	uint8_t rom[0x10000];

	uint8_t opcode;
	static const int SMALL_BUFFER = 256;

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
		X = 0;

		ram[RESET_VECTOR] = 0;
		ram[RESET_VECTOR + 1] = 0;

		PC = ram[RESET_VECTOR];
		SP = RAM_END;
	}

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
	void exec();
	void tick();
	void interrupt(uint32_t vector);

	// breakpoints
	uint16_t getPC() { return PC; }
	bool getSymbolAddress(const std::string &name, uint16_t &addr);
	bool getSymbolName(uint16_t addr, std::string &name);
	void addBreakpoint(uint16_t addr) { breakpoints.insert(addr); }

	void clearAllBreakpoints() { breakpoints.clear(); }
	bool clearBreakpoint(const std::string &name)
	{
		uint16_t addr = 0;
		if (getSymbolAddress(name, addr))
		{
			breakpoints.erase(addr);
			log("removed breakpoint @ 0x%04X", addr);

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
			log("breakpoint @ 0x%04X", addr);

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

	void printRegisters()
	{
		printf("A = %02X X = %04X CC = %02X SP = %04X PC = %04X\n", A, X, CC, SP, PC);
	}

	void printMemory(uint32_t addr)
	{
		printf("%d (0x%04X): %d (0x%04X)\n", addr, addr, ram[addr], ram[addr]);
	}
};

//
//
//
void Cisc::listBreakpoints()
{
	for (auto it = breakpoints.begin(); it != breakpoints.end(); it++)
	{
		printf("breakpoint @ 0x%04X\n", *it);
	}
}

//
//
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
//
//
bool Cisc::getSymbolName(uint16_t addr, std::string &name)
{
	return obj.findSymbolByAddr(addr, name);
}

//
//
//
void Cisc::load(const std::string &filename)
{
	obj.readFile(filename);

	PC = obj.getEntryPoint();

	// populate ram
	memset(ram, 0, 0xFFFF);
	memcpy(ram, obj.dataPtr(), obj.getDataSize());

	// populate rom
	memset(rom, 0, 0xFFFF);
	memcpy(rom, obj.textPtr(), obj.getTextSize());
}

//
//
//
void Cisc::push(uint8_t val)
{
	SP--;
	ram[SP] = val;
}

//
//
//
uint8_t Cisc::pop()
{
	uint8_t val = ram[SP++];
	return val;
}

void Cisc::getRegisterList(uint8_t operand, std::string &str)
{
	if (operand & REG_A)
		str = "A ";
	if (operand & REG_X)
		str += "X ";
	if (operand & REG_CC)
		str += "CC ";
	if (operand & REG_SP)
		str += "SP ";
	if (operand & REG_PC)
		str += "PC ";
}

//
//
//
void Cisc::log(const char *fmt, ...)
{
	char buf[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
		vsprintf(buf, fmt, argptr);
	va_end(argptr);

	printf("%s\n", buf);
}

//
//
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

	if (operand & REG_A)
		push(A);

	if (operand & REG_CC)
		push(CC);

	std::string s;
	getRegisterList(operand, s);
	log("PUSH %s", s.c_str());
}

//
//
//
void Cisc::popRegs()
{
	uint8_t operand = fetch();

	if (operand & REG_CC)
		CC = pop();

	if (operand & REG_A)
		A = pop();

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
//
//
void Cisc::updateFlag(uint32_t result, uint8_t flag)
{
	if (result)
		SETF(flag);
	else
		CLRF(flag);
}

//
// TODO - update flags (CC)
//
void Cisc::exec()
{
	std::string name;
	uint8_t operand;
	uint16_t addr, temp16;

	switch (opcode)
	{
	case OP_NOP:
		log("NOP");
		break;

	case OP_ADD:
		addr = fetchW();
		temp16 = A + ram[addr];

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(temp16 & 0xFF00, FLAG_V);

		A = temp16 & 0xFF;
		
		log("ADD [0x%X]", addr);
		break;

	case OP_ADDI:
		operand = fetch();
		temp16 = A + operand;

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(temp16 & 0xFF00, FLAG_V);

		A = temp16 & 0xFF;

		log("ADD %d", operand);
		break;

	case OP_SUB:
		addr = fetchW();
		temp16 = A - ram[addr];

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(temp16 & 0xFF00, FLAG_V);

		A = temp16 & 0xFF;

		log("SUB [0x%X]", addr);
		break;

	case OP_SUBI:
		operand = fetch();
		temp16 = A - operand;

		updateFlag(temp16 & 0xFF00, FLAG_C);
		updateFlag(temp16 == 0, FLAG_Z);
		updateFlag(temp16 & 0x80, FLAG_N);
		updateFlag(temp16 & 0xFF00, FLAG_V);

		A = temp16 & 0xFF;

		log("SUB %d", operand);
		break;

	case OP_AND:
		addr = fetchW();
		A = A & ram[addr];

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("AND [0x%X]", addr);
		break;
	
	case OP_ANDI:
		operand = fetch();
		A = A & operand;

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("AND %d", operand);
		break;

	case OP_OR:
		addr = fetchW();
		A = A | ram[addr];

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("OR [0x%X]", addr);
		break;

	case OP_ORI:
		operand = fetch();
		A = A | operand;

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("OR %d", operand);
		break;

	case OP_XOR:
		addr = fetchW();
		A = A % ram[addr];

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("XOR [0x%X]", addr);
		break;

	case OP_XORI:
		operand = fetch();
		A = A % operand;

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("XOR %d", operand);
		break;

	case OP_CALL:
		addr = fetchW();

		push(LOBYTE(PC));
		push(HIBYTE(PC));

		PC = addr;

		obj.findSymbolByAddr(PC, name);

		log("CALL %s (0x%X)", name.c_str(), PC);
		break;
	
	case OP_RET:
		PC = (pop() << 8) | pop();
		log("RET");
		break;

	case OP_RTI:
		popAll();
		CLRF(FLAG_I);

		log("RTI");
		break;

	case OP_JMP:
		PC = fetchW();
		log("JMP 0x%X", PC);
		break;

	case OP_JNE:
		addr = fetchW();
		if (!(CC & FLAG_Z))
			PC = addr;

		log("JNE 0x%X", addr);
		break;

	case OP_JEQ:
		addr = fetchW();
		if ((CC & FLAG_Z))
			PC = addr;

		log("JEQ 0x%X", addr);
		break;

	case OP_LAX:
		A = ram[X];

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LAX");
		break;

	case OP_LDA:
		addr = fetchW();
		A = ram[addr];

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LDA [0x%X]", addr);
		break;

	case OP_LDAI:
		operand = fetch();
		A = operand;

		updateFlag(A == 0, FLAG_Z);
		updateFlag(A & 0x80, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LDA %d", operand);
		break;

	case OP_LDX:
		addr = fetchW();
		X = ram[addr] + (ram[addr + 1] << 8);

		updateFlag(X == 0, FLAG_Z);
		updateFlag(X & 0x8000, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LDX [0x%X]", addr);
		break;

	case OP_LDXI:
		X = fetchW();

		updateFlag(X == 0, FLAG_Z);
		updateFlag(X & 0x8000, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LDX %d", X);
		break;

	case OP_LEAX:
		operand = fetch();
		X = (int)X + (char)operand;

		updateFlag(X == 0, FLAG_Z);
		updateFlag(X & 0x8000, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LEAX %d", (char)operand);
		break;

	case OP_LXX:
		X = ram[X] + (ram[X + 1] << 8);

		updateFlag(X == 0, FLAG_Z);
		updateFlag(X & 0x8000, FLAG_N);
		updateFlag(0, FLAG_V);

		log("LXX");
		break;

	case OP_STA:
		addr = fetchW();
		ram[addr] = A;

		log("STA 0x%X", addr);
		break;

	case OP_STX:
		addr = fetchW();
		ram[addr] = LOBYTE(X);
		ram[addr + 1] = HIBYTE(X);

		log("STX 0x%X", addr);
		break;

	case OP_STAX:
		ram[X] = A;

		log("STAX");
		break;

	case OP_PUSH:
		pushRegs();
		break;

	case OP_POP:
		popRegs();
		break;

	default:
		panic();
	}
}

//
//
//
void Cisc::tick()
{
	opcode = fetch();
	decode();
	exec();
}

//
//
//
uint8_t Cisc::fetch()
{
	return rom[PC++];
}

//
//
//
uint16_t Cisc::fetchW()
{
	uint16_t word;

	word = fetch();
	word |= (fetch() << 8);

	return word;
}

//
//
//
void Cisc::decode()
{
}

//
//
//
void Cisc::pushAll()
{
	push(LOBYTE(PC));
	push(HIBYTE(PC));

	auto addr = SP;
	push(LOBYTE(addr));
	push(HIBYTE(addr));

	push(LOBYTE(X));
	push(HIBYTE(X));

	push(A);

	push(CC);
}

//
//
//
void Cisc::popAll()
{
	CC = pop();

	A = pop();

	X = (pop() << 8) | pop();
	SP = (pop() << 8) | pop();
	PC = (pop() << 8) | pop();
}

//
//
//
void Cisc::interrupt(uint32_t vector)
{
	// no re-entrant interrupts by default
	if (TSTF(FLAG_I))
		return;

	// save the current context
	pushAll();

	// jump to the interrupt vector
	PC = ram[vector] + (ram[vector + 1] << 8);
}

//
//
//
void Cisc::panic()
{
	printRegisters();

	while (true);

	exit(-1);
}

//
// show usage
//
void usage()
{
	printf("usage: cisc [options] filename\n");
	exit(0);
}

//
// get options from the command line
//
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
//
//
int main(int argc, char* argv[])
{
	bool done = false;
	bool singleStep = true;
	char buf[256];

	if (argc == 1)
		usage();

	int iFirstArg = getopt(argc, argv);

	Cisc cpu;

	cpu.load(argv[iFirstArg]);

	while (!done)
	{
		if (singleStep)
		{
			printf(">");
			fgets(buf, 255, stdin);

			char *pToken = strtok(buf, " \n");
			if (!strcmp(pToken, "s"))
				cpu.tick();
			else if (!strcmp(pToken, "r"))
				cpu.printRegisters();
			else if (!strcmp(pToken, "q"))
				exit(0);
			else if (!strcmp(pToken, "g"))
			{
				singleStep = false;
				cpu.tick();
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
			else if (!strcmp(pToken, "d"))
			{
				auto tok = strtok(nullptr, " \n");
				uint32_t addr = 0;
				auto base = 10;
				if (tok)
				{
					if (tok[0] == '$')
					{
						base = 16;
						tok++;
					}
					addr = strtoul(tok, nullptr, base);

					cpu.printMemory(addr);
				}
			}
		}
		else
		{
			while (!singleStep)
			{ 
				auto pc = cpu.getPC();
				if (cpu.isBreakpoint(pc))
				{
					std::string name;

					cpu.getSymbolName(pc, name);
					fprintf(stdout, "hit breakpoint @ 0x%04X %s\n", pc, name.c_str());
					singleStep = true;
				}
				else
					cpu.tick();
			}
		}
		
	}

	return 0;
}