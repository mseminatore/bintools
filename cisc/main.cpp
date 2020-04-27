#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include "../cpu_cisc.h"
#include <stdio.h>

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

public:
	Cisc() {
		reset();
	}

	virtual ~Cisc() {}

	void load(const std::string &filename);
	void reset() 
	{ 
		A = CC = opcode = 0; 
		PC = X = 0;
		SP = 0xFFFF;
	}

	void push(uint8_t val);
	uint8_t pop();
	void panic();
	uint16_t getAddress();
	uint8_t fetch();
	void decode();
	void exec();
	void tick();
	void registers()
	{
		printf("A = %02X X = %04X CC = %02X SP = %04X PC = %04X\n", A, X, CC, SP, PC);
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
	memcpy(ram, obj.dataPtr(), obj.getDataAddress());

	// populate rom
	memset(rom, 0, 0xFFFF);
	memcpy(rom, obj.textPtr(), obj.getTextAddress());
}

//
//
//
uint16_t Cisc::getAddress()
{
	uint16_t address;

	address = fetch();
	address |= (fetch() << 8);

	return address;
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

//
// TODO - update flags (CC)
//
void Cisc::exec()
{
	uint8_t operand;

	switch (opcode)
	{
	case OP_NOP:
		break;

	case OP_ADD:
		A = A + ram[getAddress()];
		break;

	case OP_ADDI:
		operand = fetch();
		A = A + operand;
		break;

	case OP_SUB:
		A = A - ram[getAddress()];
		break;

	case OP_SUBI:
		operand = fetch();
		A = A - operand;
		break;

	case OP_AND:
		A = A & ram[getAddress()];
		break;
	
	case OP_ANDI:
		operand = fetch();
		A = A & operand;
		break;

	case OP_OR:
		A = A | ram[getAddress()];
		break;

	case OP_ORI:
		operand = fetch();
		A = A | operand;
		break;

	case OP_XOR:
		A = A % ram[getAddress()];
		break;

	case OP_XORI:
		operand = fetch();
		A = A % operand;
		break;

	case OP_CALL:
		push(LOBYTE(PC));
		push(HIBYTE(PC));
		PC = getAddress();
		break;
	
	case OP_RET:
		PC = (pop() << 8) | pop();
		break;

	case OP_JMP:
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
void Cisc::decode()
{
}

//
//
//
void Cisc::panic()
{
	registers();
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
	if (argc == 1)
		usage();

	int iFirstArg = getopt(argc, argv);

	Cisc cpu;

	cpu.load(argv[iFirstArg]);

	bool done = false;
	char buf[256];
	while (!done)
	{
		printf(">");
		fgets(buf, 255, stdin);
		cpu.tick();
		cpu.registers();
	}

	return 0;
}