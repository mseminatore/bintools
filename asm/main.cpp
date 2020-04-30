#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include "../cpu_cisc.h"
#include <stdio.h>
#include "./ParserKit/baseparser.h"

//
// Command line switches
//
bool g_bDebug = false;
char *g_szOutputFilename = "a.out";

enum
{
	stEqu = stUser,
	stLabel,
	stProc,
	stExternal,
	stDataByte,
	stDataWord
};

//
//
//
class AsmParser : public BaseParser
{
protected:
	AoutFile obj;
	using AddressList = std::vector<uint16_t>;
	using Fixups = std::map<std::string, AddressList>;
	Fixups fixups;

	void addFixup(const std::string &str, uint16_t addr);
	void applyFixups(const std::string &str, uint16_t addr);

public:
	AsmParser();
	virtual ~AsmParser() = default;

	int yyparse() override;


	void imm8(int op);
	void imm16(int op);

	void memOperand(int immOp, int memOp, bool isWordValue = false);
	void dataAddress(int op);
	void codeAddress(int op);

	void include();
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
	TV_STAX,
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
	TV_RTI,

	TV_AND,
	TV_OR,
	TV_XOR,
	TV_NOT,

	TV_PUSH,
	TV_POP,

	TV_PUBLIC,
	TV_EXTERN,
	TV_PROC,
	TV_INCLUDE
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

	{ "CALL",	TV_CALL },
	{ "JMP",	TV_JMP },
	{ "JNE",	TV_JNE },
	{ "JEQ",	TV_JEQ },
	{ "RET",	TV_RET },
	{ "RTI",	TV_RTI },

	{ "ORG",	TV_ORG},
	{ "EQU",	TV_EQU },
	{ "DB",		TV_DB},
	{ "DW",		TV_DW},
	
	{ "LDA",	TV_LDA },
	{ "STA",	TV_STA },
	{ "STAX",	TV_STAX },
	{ "LDX",	TV_LDX },
	{ "LAX",	TV_LAX },
	{ "LEAX",	TV_LEAX },

	{ "A",		TV_A },
	{ "X",		TV_X },
	{ "CC",		TV_CC },
	{ "SP",		TV_SP },
	{ "PC",		TV_PC },

	{ "PUSH",	TV_PUSH },
	{ "POP",	TV_POP },

	{ "PUBLIC",	TV_PUBLIC },
	{ "EXTERN",	TV_EXTERN },
	{ "PROC",	TV_PROC },
	{ "INCLUDE", TV_INCLUDE },

	{ nullptr,	TV_DONE }
};

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

		if (args[i][1] == 'o')
		{
			g_szOutputFilename = args[i + 1];
			i++;
		}
	}

	return i;
}

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
		
		// set the labels value to be its address in the code segment
		sym->ival = obj.getTextAddress();
		sym->type = stLabel;
		break;

	case TV_EQU:
		// named constant
		match();
		if (lookahead == TV_INTVAL)
		{
			sym->ival = yylval.ival;
			sym->type = stEqu;
			match();
		}
		else if (lookahead == TV_CHARVAL)
		{
			sym->ival = yylval.char_val;
			sym->type = stEqu;
			match();
		}
		break;

	case TV_DB:
		// data byte
		match();
		sym->type = stDataByte;

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

		sym->type = stDataWord;
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
void AsmParser::memOperand(int immOp, int memOp, bool isWordValue)
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
		if (isWordValue)
			yyerror("word value expected!");

		auto val = yylval.char_val;

		obj.addText(LOBYTE(val));
		match();
	}
		break;

	case TV_INTVAL:	// immediate value
	{
		auto val = yylval.ival;

		auto addr = obj.addText(LOBYTE(val));
		if (isAddrOperand || isWordValue)
		{
			obj.addText(HIBYTE(val));

			if (isAddrOperand)
			{
				RelocationEntry re;
				re.address = addr;
				re.length = 1;
				re.index = SEG_DATA;
				re.external = 0;

				obj.addTextRelocation(re);
			}
		}

		match();
	}
		break;

	case TV_ID:	// named immediate value
	{
		auto val = yylval.sym->ival;

		auto addr = obj.addText(LOBYTE(val));
		if (isAddrOperand || isWordValue)
		{
			obj.addText(HIBYTE(val));

			if (isAddrOperand)
			{
				RelocationEntry re;
				re.address = addr;
				re.length = 1;
				re.index = SEG_DATA;
				re.external = 0;
				
				obj.addTextRelocation(re);
			}
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
// Parse a 16b data address or label
//
void AsmParser::dataAddress(int op)
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
		re.index = SEG_DATA;
		re.external = 0;
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
		re.index = SEG_DATA;
		re.external = 0;
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
void AsmParser::applyFixups(const std::string &str, uint16_t addr)
{
	auto iter = fixups.find(str);

	// if there were no forward refs there will be no fixups
	if (iter == fixups.end())
		return;

	for (auto i = iter->second.begin(); i != iter->second.end(); i++)
	{
		auto loc = *i;
		auto rom = obj.textPtr();
		rom[loc] = addr & 0xFF;
		rom[loc+1] = addr >> 8;
	}

	// we are done with these fixups!
	fixups.erase(iter);
}

//
//
//
void AsmParser::addFixup(const std::string &str, uint16_t addr)
{
	auto iter = fixups.find(str);
	if (iter != fixups.end())
	{
		iter->second.push_back(addr);
	}
	else
	{
		AddressList list;
		list.push_back(addr);
		fixups.insert(Fixups::value_type(str, list));
	}
}

//
// Parse a 16b code address or label
//
void AsmParser::codeAddress(int op)
{
	obj.addText(op);
	match();

	if (lookahead == TV_INTVAL)
	{
		auto val = yylval.ival;

		auto addr = obj.addText(LOBYTE(val));
		obj.addText(HIBYTE(val));

		RelocationEntry re;
		re.address	= addr;
		re.length	= 1;
		re.index	= SEG_TEXT;
		re.external = 0;
		obj.addTextRelocation(re);

		match();
	}
	else if (lookahead == TV_ID) 
	{
		auto val = yylval.sym->ival;

		auto addr = obj.addText(LOBYTE(val));
		obj.addText(HIBYTE(val));

		// see if we have to fixup this address
		if (yylval.sym->type == stUndef)
			addFixup(yylval.sym->lexeme, addr);

		RelocationEntry re;
		re.address	= addr;
		re.length	= 1;
		re.index	= SEG_TEXT;
		re.external = 0;
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
void AsmParser::imm16(int op)
{
	obj.addText(op);
	match();

	if (lookahead == TV_INTVAL)
	{
		auto val = yylval.ival;
		obj.addText(LOBYTE(val));
		obj.addText(HIBYTE(val));
		match();
	}
	else if (lookahead == TV_ID)
	{
		if (yylval.sym->type != stEqu)
			yyerror("EQU expected!");

		auto val = yylval.sym->ival;
		obj.addText(LOBYTE(val));
		obj.addText(HIBYTE(val));
		match();
	}
	else
	{
		yyerror("invalid operand!");
	}
}

//
//
//
void AsmParser::imm8(int op)
{
	obj.addText(op);
	match();

	if (lookahead == TV_INTVAL)
	{
		auto val = yylval.ival;
		obj.addText(LOBYTE(val));
		match();
	}
	else if (lookahead == TV_CHARVAL)
	{
		auto val = yylval.char_val;
		obj.addText(val);
		match();
	}
	else if (lookahead == TV_ID)
	{
		if (yylval.sym->type != stEqu)
			yyerror("EQU expected!");

		auto val = yylval.sym->ival;
		obj.addText(LOBYTE(val));
		match();
	}
	else
	{
		yyerror("invalid operand!");
	}
}

//
//
//
void AsmParser::include()
{
	match(TV_INCLUDE);
	m_lexer->pushFile(yylval.sym->lexeme.c_str());
	match(TV_STRING);
}

//
// Parse file-level constructs
//
void AsmParser::file()
{
	//DoIncludes();

	while (lookahead != TV_DONE)
	{
		switch (lookahead)
		{
		case TV_INCLUDE:
			include();
			break;

		case TV_EXTERN:
			match();

			// TODO - mark the symbol as an external reference??
			yylval.sym->type = stExternal;
			match();
			break;

		case TV_PROC:
			match();

			yylval.sym->type = stProc;
			yylval.sym->global = true;
			applyFixups(yylval.sym->lexeme, obj.getTextAddress());
			match();

			break;

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
			dataAddress(OP_STA);
			break;

		case TV_LDX:
			memOperand(OP_LDXI, OP_LDX, true);
			break;
		
		case TV_STAX:
			obj.addText(OP_STAX);
			match();
			break;

		case TV_LAX:
			obj.addText(OP_LAX);
			match();
			break;

		case TV_LEAX:
			imm8(OP_LEAX);
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
			codeAddress(OP_JMP);
			break;

		case TV_JNE:
			codeAddress(OP_JNE);
			break;

		case TV_JEQ:
			codeAddress(OP_JEQ);
			break;

		case TV_CALL:
			codeAddress(OP_CALL);
			break;

		case TV_RET:
			obj.addText(OP_RET);
			match();
			break;

		case TV_RTI:
			obj.addText(OP_RTI);
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

	m_pSymbolTable->dumpContents();

	// update symbols
	auto sym = m_pSymbolTable->getFirstGlobal();

	while (sym)
	{
		SymbolEntity se;
		obj.addSymbol(sym->lexeme, se);
		
		sym = m_pSymbolTable->getNextGlobal();
	}

	writeFile(g_szOutputFilename);

	return 0;
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
