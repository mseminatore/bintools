#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include "../cpu_cisc.h"
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
	void addressOperand(int op);
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
	TV_LDX,
	TV_STA,
	TV_LAX,
	TV_LEAX,

	TV_A,
	TV_X,
	TV_CC,
	TV_SP,
	TV_PC,
	
	TV_DB,
	TV_DW,

	TV_EQU,
	TV_ORG,

	TV_CMP,

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

//
// Table of lexemes and tokens to be recognized by the lexer
//
TokenTable _tokenTable[] =
{
	{ "NOP",	TV_NOP },
	{ "ADD",	TV_ADD },
	{ "SUB",	TV_SUB },

	{ "AND",	TV_AND },
	{ "OR",		TV_OR },
	{ "NOT",	TV_NOT },
	{ "XOR",	TV_XOR },

	{ "CMP",	TV_CMP },

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
	{ "STA",	TV_STA},
	{ "LDX",	TV_LDX},
	{ "LAX",	TV_LAX},
	{ "LEAX",	TV_LEAX},

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
// a single register
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
// a set of registers
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

		auto addr = obj.addText(LOBYTE(val));
		if (isAddrOperand)
		{
			obj.addText(HIBYTE(val));

			RelocationEntry re;
			re.address	= addr;
			re.length	= 1;
			re.index	= SEG_TEXT;
			re.external = 0;

			obj.addTextRelocation(re);
		}

		match();
	}
		break;

	case TV_ID:	// named immediate value
	{
		auto val = yylval.sym->ival;

		auto addr = obj.addText(LOBYTE(val));
		if (isAddrOperand)
		{
			obj.addText(HIBYTE(val));

			RelocationEntry re;
			re.address = addr;
			re.length = 1;
			re.index = SEG_TEXT;
			re.external = 0;

			obj.addTextRelocation(re);
		}

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
// Parse a 16b address or label
//
void AsmParser::addressOperand(int op)
{
	obj.addText(op);
	match();

	if (lookahead == TV_INTVAL)
	{
		auto val = yylval.ival;

		auto addr = obj.addText(LOBYTE(val));
		obj.addText(HIBYTE(val));

		RelocationEntry re;
		re.address = addr;
		re.length = 1;
		// TODO - choose relocation list based on branchTarget (bool)
		obj.addTextRelocation(re);

		match();
	}
	else if (lookahead == TV_ID) 
	{
		auto val = yylval.sym->ival;

		auto addr = obj.addText(LOBYTE(val));
		obj.addText(HIBYTE(val));

		RelocationEntry re;
		re.address = addr;
		re.length = 1;
		// TODO - choose relocation list based on branchTarget (bool)
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

		case TV_AND:
			memOperand(OP_ANDI, OP_AND);
			break;

		case TV_OR:
			memOperand(OP_ORI, OP_OR);
			break;

		case TV_NOT:
			memOperand(OP_NOTI, OP_NOT);
			break;

		case TV_XOR:
			memOperand(OP_XORI, OP_XOR);
			break;

		case TV_CMP:
			memOperand(OP_CMPI, OP_CMP);
			break;

		case TV_LDA:
			memOperand(OP_LDAI, OP_LDA);
			break;

		case TV_STA:
			addressOperand(OP_STA);
			break;

		case TV_LDX:
			memOperand(OP_LDXI, OP_LDX);
			break;
		
		case TV_LAX:
			obj.addText(OP_LAX);
			match();
			break;

		case TV_LEAX:
			obj.addText(OP_LEAX);
			match();

			i = 
			// TODO - get/emit the imm8 parameter
			if (lookahead == TV_INTVAL)
			{

			}
			else if (lookahead == TV_CHARVAL)
			{

			}
			else
			{
				yyerror("invalid operand!");
			}
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
