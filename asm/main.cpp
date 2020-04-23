#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include <stdio.h>
#include "./ParserKit/baseparser.h"

#define LOBYTE(val) ((val) & 0xFF)
#define HIBYTE(val) (((val) & 0xFF00) >> 8)

//
// Command line switches
//
bool g_bDebug = false;

//
//
//
class AsmParser : public BaseParser
{
protected:
	AoutFile obj;

public:
	AsmParser();
	virtual ~AsmParser() = default;

	int yyparse() override;

	void memOperand(int immOp, int memOp);
	void address();
	void label();
	void file();
	uint8_t reg();
	uint8_t regSet();

	void writeFile(const std::string &name);
};

//
// Token values
//
enum
{
	// pre-processor
	TV_NOP = TV_USER,
	TV_ADD,
	TV_SUB,

	TV_LDA,
	TV_LDI,
	TV_LDX,
	TV_STA,
	TV_LAX,

	TV_A,
	TV_X,
	TV_CC,
	TV_SP,
	TV_PC,
	
	TV_DB,
	TV_DW,

	TV_EQU,
	TV_ORG,

	TV_CALL,
	TV_RET,
	TV_JMP,
	TV_JNE,
	TV_JEQ,

	TV_AND,
	TV_OR,
	TV_XOR,
	TV_NOT,

	TV_PUSH,
	TV_POP
};

enum
{
	OP_NOP,

	OP_ADD,
	OP_ADDI,

	OP_SUB,
	OP_SUBI,

	OP_CALL,
	OP_RET,
	OP_JMP,
	OP_JNE,
	OP_JEQ,

	OP_LDA,
	OP_LDI,
	OP_LDX,
	OP_LAX,
	OP_STA,

	OP_PUSH,
	OP_POP
};

//
// Table of lexemes and tokens to be recognized by the lexer
//
TokenTable _tokenTable[] =
{
	{ "NOP",	TV_NOP },
	{ "ADD",	TV_ADD },
	{ "SUB",	TV_SUB },

	{ "CALL",	TV_CALL},
	{ "JMP",	TV_JMP},
	{ "JNE",	TV_JNE},
	{ "JEQ",	TV_JEQ},
	{ "RET",	TV_RET},

	{ "ORG",	TV_ORG},
	{ "EQU",	TV_EQU },
	{ "DB",		TV_DB},
	{ "DW",		TV_DW},
	
	{ "LDA",	TV_LDA},
	{ "LDI",	TV_LDI},
	{ "STA",	TV_STA},
	{ "LDX",	TV_LDX},
	{ "LAX",	TV_LAX},

	{ "A",		TV_A},
	{ "X",		TV_X},
	{ "CC",		TV_CC},
	{ "SP",		TV_SP},
	{ "PC",		TV_PC},

	{ "PUSH",	TV_PUSH},
	{ "POP",	TV_POP},

	{ nullptr,	TV_DONE }
};

//
//
//
AsmParser::AsmParser() : BaseParser(std::make_unique<SymbolTable>())
{
	m_lexer = std::make_unique<LexicalAnalyzer>(_tokenTable, this, &yylval);

	// setup our lexical options
	m_lexer->setCharLiterals(true);
	m_lexer->setCPPComments(true);
	m_lexer->setHexNumbers(true);
}

//
//
//
void AsmParser::label()
{
	// we found a new symbol
	auto sym = yylval.sym;

	match();

	switch (lookahead)
	{
	case ':':
		// code address label
		match();
		break;

	case TV_EQU:
		// named constant
		match();
		if (lookahead == TV_INTVAL)
		{
			sym->ival = yylval.ival;
			match();
		}
		break;

	case TV_DB:
		// data byte
		match();

		// 
		if (lookahead == TV_INTVAL)
		{
			sym->ival = obj.addData(yylval.ival);
			match();
		}
		else
		{
			sym->ival = obj.addData(yylval.char_val);
			match();
		}
		break;

	case TV_DW:
	{
		// data word
		match();

		auto val = yylval.ival;

		sym->ival = obj.addData(LOBYTE(val));
		obj.addData(HIBYTE(val));

		match();
	}
		break;

	default:
		yyerror("syntax error");
		break;
	}
}

//
//
//
//void AsmParser::imm8()
//{
//
//}

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
	FLAG_Z = 1,
	FLAG_C = 2,
	FLAG_I = 4
};

//
//
//
uint8_t AsmParser::reg()
{
	uint8_t reg = 0;

	switch (lookahead)
	{
	case TV_A:
		reg = REG_A;
		match();
		break;

	case TV_X:
		reg = REG_X;
		match();
		break;

	case TV_SP:
		reg = REG_SP;
		match();
		break;

	case TV_CC:
		reg = REG_CC;
		match();
		break;

	case TV_PC:
		reg = REG_PC;
		match();
		break;
	}
	
	return reg;
}

//
//
//
uint8_t AsmParser::regSet()
{
	uint8_t val, regs = 0;

	while (val = reg())
	{
		regs |= val;
		if (lookahead == ',')
			match();
	}

	return regs;
}

//
// an 8b immediate value or an address
//
void AsmParser::memOperand(int immOp, int memOp)
{
	bool isAddrOperand = false;

	match();

	if (lookahead == '[')
	{
		obj.addText(memOp);
		isAddrOperand = true;
		match();
	}
	else
	{
		obj.addText(immOp);
	}

	switch (lookahead)
	{
	case TV_CHARVAL:	// immediate value
	{
		if (isAddrOperand)
			yyerror("character literal not allowed!");

		auto val = yylval.char_val;

		obj.addText(LOBYTE(val));
		match();
	}
		break;

	case TV_INTVAL:	// immediate value
	{
		auto val = yylval.ival;

		obj.addText(LOBYTE(val));
		if (isAddrOperand)
			obj.addText(HIBYTE(val));

		match();
	}
		break;

	case TV_ID:	// named immediate value
	{
		auto val = yylval.sym->ival;

		obj.addText(LOBYTE(val));
		if (isAddrOperand)
			obj.addText(HIBYTE(val));

		match();
	}
		break;
	
	default:
		yyerror("invalid memory operand!");
	}

	if (isAddrOperand)
		match(']');
}

//
// Parse an address or label
//
void AsmParser::address()
{
	if (lookahead == TV_INTVAL)
	{
		auto val = yylval.ival;

		// TODO - reloc needed!
		auto addr = obj.addText(LOBYTE(val));
		obj.addText(HIBYTE(val));

		RelocationEntry re;
		re.address = addr;
		re.length = 2;
		obj.addTextRelocation(re);

		match();
	}
	else if (lookahead == TV_ID) 
	{
		auto val = yylval.sym->ival;

		// TODO - reloc needed!
		auto addr = obj.addText(LOBYTE(val));
		obj.addText(HIBYTE(val));

		RelocationEntry re;
		re.address = addr;
		re.length = 2;
		obj.addTextRelocation(re);

		match();
	}
	else 
	{
		yyerror("syntax error");
	}
}

//
//
//
void AsmParser::file()
{
	//DoIncludes();

	while (lookahead != TV_DONE)
	{
		switch (lookahead)
		{
		case TV_ID:
			label();
			break;

		case TV_NOP:
			obj.addText(OP_NOP);
			match();

			break;

		case TV_ADD:
			memOperand(OP_ADDI, OP_ADD);
			break;

		case TV_SUB:
			memOperand(OP_SUBI, OP_SUB);
			break;

		case TV_JMP:
			obj.addText(OP_JMP);
			match();

			address();
			break;

		case TV_JNE:
			obj.addText(OP_JNE);
			match();

			address();
			break;

		case TV_JEQ:
			obj.addText(OP_JEQ);
			match();

			address();
			break;

		case TV_CALL:
			obj.addText(OP_CALL);
			match();

			address();
			break;

		case TV_RET:
			obj.addText(OP_RET);
			match();

			break;

		case TV_LDI:
			obj.addText(OP_LDI);
			match();
			
			obj.addText(yylval.ival);
			match();
			break;

		case TV_LDA:
			obj.addText(OP_LDA);
			match();
			
			address();
			break;

		case TV_STA:
			obj.addText(OP_STA);
			match();

			address();
			break;

		case TV_LDX:
			obj.addText(OP_STA);
			match();

			obj.addText(yylval.ival);
			match();
			break;
		
		case TV_LAX:
			obj.addText(OP_LAX);
			match();

			break;

		case TV_PUSH:
			obj.addText(OP_PUSH);
			match();

			obj.addText(regSet());
			break;

		case TV_POP:
			obj.addText(OP_POP);
			match();

			obj.addText(regSet());
			break;

		default:
			yyerror("syntax error!");
		}
	}
}

//
//
//
void AsmParser::writeFile(const std::string &name)
{
	FILE *f = fopen(name.c_str(), "wb");
	if (nullptr == f)
		return;

	obj.writeFile(f);
}

//
//
//
int AsmParser::yyparse()
{
	BaseParser::yyparse();

	file();

	//if (yydebug)
	//	root.dumpAll();

	writeFile("a.out");

	return 0;
}

//
// show usage
//
void usage()
{
	printf("usage: asm [options] filename\n");
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
		if (args[i][1] == 'v')
			g_bDebug = true;

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

	AsmParser parser;

	parser.yydebug = g_bDebug;

	parser.parseFile(argv[iFirstArg]);

	return 0;
}
