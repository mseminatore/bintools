#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include "../cpu_cisc.h"
#include <stdio.h>
#include <stdarg.h>

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
	
	uint8_t ram[0xFFFF];
	uint8_t rom[0xFFFF];

	uint8_t opcode;
	static const int SMALL_BUFFER = 256;

	void log(const char *fmt, ...);

	void pushRegs();
	void popRegs();

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
	void interrupt();

	void printRegisters()
	{
		printf("A = %02X X = %04X CC = %02X SP = %04X PC = %04X\n", A, X, CC, SP, PC);
	}

	void printMemory(uint32_t addr)
	{
		printf("%d (0x%04X): %d\n", addr, addr, ram[addr]);
	}
};

//
//
//
void Cisc::load(const std::string &filename)
{
	AoutFile obj;

	FILE *f = fopen(filename.c_str(), "rb");
	if (!f)
		return;

	obj.readFile(f);

	// populate ram
	memset(ram, 0, 0xFFFF);
	memcpy(ram, obj.dataPtr(), obj.getDataSize());

	// populate rom
	memset(rom, 0, 0xFFFF);
	memcpy(rom, obj.textPtr(), obj.getTextSize());

	fclose(f);
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

	printf("\n%s\n", buf);
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
		push(LOBYTE(PC));
		push(HIBYTE(PC));
	}

	if (operand & REG_SP)
	{
		addr = SP;
		push(LOBYTE(addr));
		push(HIBYTE(addr));
	}

	if (operand & REG_X)
	{
		push(LOBYTE(X));
		push(HIBYTE(X));
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
		X = (pop() << 8) | pop();
	}

	if (operand & REG_SP)
	{
		SP = (pop() << 8) | pop();
	}

	if (operand & REG_PC)
	{
		PC = (pop() << 8) | pop();
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

		log("AND [0x%X]", addr);
		break;
	
	case OP_ANDI:
		operand = fetch();
		A = A & operand;

		log("AND %d", operand);
		break;

	case OP_OR:
		addr = fetchW();
		A = A | ram[addr];

		log("OR [0x%X]", addr);
		break;

	case OP_ORI:
		operand = fetch();
		A = A | operand;

		log("OR %d", operand);
		break;

	case OP_XOR:
		addr = fetchW();
		A = A % ram[addr];

		log("XOR [0x%X]", addr);
		break;

	case OP_XORI:
		operand = fetch();
		A = A % operand;

		log("XOR %d", operand);
		break;

	case OP_CALL:
		addr = fetchW();

		push(LOBYTE(PC));
		push(HIBYTE(PC));

		PC = addr;
		log("CALL 0x%X", PC);
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
		X = ram[addr];
		log("LDX [0x%X]", addr);
		break;

	case OP_LDXI:
		X = fetchW();
		log("LDX %d", X);
		break;

	case OP_LEAX:
		operand = fetch();
		X = (int)X + (char)operand;
		log("LEAX %d", operand);
		break;

	case OP_STA:
		addr = fetchW();
		ram[addr] = A;

		log("STA 0x%X", addr);
		break;

	case OP_STX:
		addr = fetchW();
		ram[addr] = X && 0xFF;
		ram[addr + 1] = X >> 8;

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
void Cisc::interrupt()
{
	// no re-entrant interrupts by default
	if (TSTF(FLAG_I))
		return;

	// save the current context
	pushAll();
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
	char buf[256];

	if (argc == 1)
		usage();

	int iFirstArg = getopt(argc, argv);

	Cisc cpu;

	cpu.load(argv[iFirstArg]);

	while (!done)
	{
		printf(">");
		fgets(buf, 255, stdin);

		char *pToken = strtok(buf, " \n");
		if (!pToken)
			cpu.tick();
		else if (!strcmp(pToken, "r"))
			cpu.printRegisters();
		else if (!strcmp(pToken, "d"))
		{
			auto tok = strtok(nullptr, " \n");
			uint32_t addr = 0;
			if (tok)
				addr = strtoul(tok, nullptr, 10);
			
			cpu.printMemory(addr);
		}

		
	}

	return 0;
}