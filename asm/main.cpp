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
	stDataWord,
	stDataString
};

//
//
//
class AsmParser : public BaseParser
{
protected:
	ObjectFile obj;
	using AddressList = std::vector<uint16_t>;
	using Fixups = std::map<std::string, AddressList>;
	Fixups fixups;

	void addFixup(const std::string &str, uint16_t addr);
	void applyFixups(const std::string &str, uint16_t addr);
	void dataByte(SymbolEntry * sym);
	void dataWord(SymbolEntry * sym);
	void dataString(SymbolEntry * sym);

public:
	AsmParser();
	virtual ~AsmParser() = default;

	int yyparse() override;


	void imm8(int op);
	void imm16(int op);

	bool isDataLabel(int type);

	void memOperand(int immOp, int memOp, bool isWordValue = false);
	void addTextRelocation(uint32_t addr, uint32_t length, uint32_t index, bool external);
	void dataAddress(int op);
	void codeAddress(int op);

	void include();
	void label();
	void file();

	uint8_t reg();
	uint8_t regSet();
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
	TV_STX,
	TV_STAX,
	TV_LAX,
	TV_LEAX,
	TV_LXX,

	TV_A,
	TV_X,
	TV_CC,
	TV_SP,
	TV_PC,
	
	TV_DB,
	TV_DW,
	TV_DS,

	TV_EQU,
	TV_ORG,

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
	TV_COM,
	TV_NEG,

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
	{ "DS",		TV_DS },

	{ "LDA",	TV_LDA },
	{ "STA",	TV_STA },
	{ "STX",	TV_STX },
	{ "STAX",	TV_STAX },
	{ "LDX",	TV_LDX },
	{ "LAX",	TV_LAX },
	{ "LEAX",	TV_LEAX },
	{ "LXX",	TV_LXX },

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
// We saw a new symbol declaration. Fixup any intra-segment forward references to this symbol
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
		rom[loc + 1] = addr >> 8;
	}

	// we are done with these fixups!
	fixups.erase(iter);
}

//
// We found an inferred forward reference to a symbol, add the location to fixups list
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
// Allocate a data byte in the data segment
//
void AsmParser::dataByte(SymbolEntry *sym)
{
	SymbolEntity se;
	se.type = SET_DATA;

	match();

	if (lookahead == TV_INTVAL)
	{
		se.value = obj.addData(yylval.ival);
		match();
	}
	else
	{
		se.value = obj.addData(yylval.char_val);
		match();
	}

	if (sym)
	{
		sym->type = stDataByte;
		sym->ival = se.value;
		obj.addSymbol(sym->lexeme, se);
	}
}

//
//
//
void AsmParser::dataWord(SymbolEntry *sym)
{
	SymbolEntity se;
	se.type = SET_DATA;

	match();

	auto val = yylval.ival;

	se.value = obj.addData(LOBYTE(val));
	obj.addData(HIBYTE(val));

	match();

	if (sym)
	{
		sym->type = stDataWord;
		sym->ival = se.value;
		obj.addSymbol(sym->lexeme, se);
	}
}

//
//
//
void AsmParser::dataString(SymbolEntry *sym)
{
	SymbolEntity se;
	se.type = SET_DATA;

	match();

	if (lookahead != TV_STRING)
		yyerror("String literal expected");

	se.value = obj.getDataSize();

	// copy the data into the segment
	for (size_t i = 0; i < yylval.sym->lexeme.size(); i++)
		obj.addData(yylval.sym->lexeme[i]);

	// make sure it is null terminated
	obj.addData(0);

	if (sym)
	{
		sym->type = stDataString;
		sym->ival = se.value;
		obj.addSymbol(sym->lexeme, se);
	}

	yylval.sym->isReferenced = true;

	match(TV_STRING);
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
		sym->ival = obj.getTextSize();
		sym->type = stLabel;

		// need to apply fixups here to fix any forward references to this label
		applyFixups(sym->lexeme, obj.getTextSize());

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
		sym->isReferenced = true;
		break;

	case TV_DB:
		// data byte
		dataByte(sym);
		break;

	case TV_DW:
		// data word
		dataWord(sym);
		break;

	case TV_DS:
		dataString(sym);
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
//
//
void AsmParser::addTextRelocation(uint32_t addr, uint32_t length, uint32_t index, bool external)
{
	RelocationEntry re;

	re.address = addr;
	re.length = length;
	re.index = index;
	re.external = external ? 1 : 0;

	obj.addTextRelocation(re);
}

//
//
//
void AsmParser::imm8(int op)
{
	obj.addText(op);

	uint32_t val = 0;

	if (lookahead == TV_INTVAL)
	{
		val = yylval.ival;
	}
	else if (lookahead == TV_CHARVAL)
	{
		val = yylval.char_val;
	}
	else if (lookahead == TV_ID)
	{
		if (yylval.sym->type != stEqu)
			yyerror("EQU expected!");

		yylval.sym->isReferenced = true;

		val = yylval.sym->ival;
	}
	else
	{
		yyerror("invalid operand!");
	}

	obj.addText(LOBYTE(val));
	match();
}

//
// We are expecting a 16b immediate value or a named immediate value
//
void AsmParser::imm16(int op)
{
	bool bSegmentRelative = false;

	obj.addText(op);

	uint32_t val = 0;

	if (lookahead == TV_INTVAL)
	{
		val = yylval.ival;
	}
	else if (lookahead == TV_ID)
	{
		val = yylval.sym->ival;
		yylval.sym->isReferenced = true;

		// TODO - check for stProc here?

		if (yylval.sym->type != stEqu)
			bSegmentRelative = true;

		// handle undefined symbols
		if (yylval.sym->type == stUndef)
			yyerror("undefined symbol '%s'", yylval.sym->lexeme);
	}
	else
	{
		yyerror("invalid operand!");
	}

	auto addr = obj.addText(LOBYTE(val));
	obj.addText(HIBYTE(val));

	// if this is a data address it needs a relocation entry
	if (bSegmentRelative)
	{
		auto external = yylval.sym->type == stExternal ? true : false;
		uint32_t index = SEG_DATA;
		if (external)
		{
			// for stExternal this needs to be the index to the symbol table entry
			index = obj.indexOfSymbol(yylval.sym->lexeme);
			assert(index != UINT_MAX);
		}

		addTextRelocation(addr, 1, index, external);
	}

	match();
}

//
//
//
bool AsmParser::isDataLabel(int type)
{
	if (type == stDataByte || type == stDataWord || type == stDataString)
		return true;
	return false;
}

//
// an immediate value or an address
//
void AsmParser::memOperand(int immOp, int memOp, bool isWordValue)
{
	bool bSegmentRelative = false;

	match();

	// see if this is an immediate operation
	if (lookahead != '[')
	{
		if (isWordValue)
			return imm16(immOp);
		else
			return imm8(immOp);
	}

	match('[');
	obj.addText(memOp);

	uint32_t val = 0;

	switch (lookahead)
	{
	case TV_INTVAL:	// absolute addr
		{
			val = yylval.ival;
		}
		break;

	case TV_ID:	// named addr
		{
			// value is either an immediate or an addr
			val = yylval.sym->ival;
			yylval.sym->isReferenced = true;

			if (yylval.sym->type != stEqu)
				bSegmentRelative = true;
		}
		break;
	
	default:
		yyerror("invalid memory operand!");
	}

	auto addr = obj.addText(LOBYTE(val));
	obj.addText(HIBYTE(val));

	// if this is a data address it needs a relocation entry
	if (bSegmentRelative)
	{
		auto external = yylval.sym->type == stExternal ? true : false;
		uint32_t index = SEG_DATA;
		if (external)
		{
			// for stExternal this needs to be the index to the symbol table entry
			index = obj.indexOfSymbol(yylval.sym->lexeme);
			assert(index != UINT_MAX);
		}

		addTextRelocation(addr, 1, index, external);
	}

	match();
	match(']');
}

//
// Parse a 16b data address or label
//
void AsmParser::dataAddress(int op)
{
	bool bSegmentRelative = false;

	obj.addText(op);
	match();

	uint32_t val = 0;

	if (lookahead == TV_INTVAL)
	{
		val = yylval.ival;
	}
	else if (lookahead == TV_ID)
	{
		val = yylval.sym->ival;
		yylval.sym->isReferenced = true;

		if (yylval.sym->type != stEqu)
			bSegmentRelative = true;
	}
	else
	{
		yyerror("syntax error");
	}

	auto addr = obj.addText(LOBYTE(val));
	obj.addText(HIBYTE(val));

	if (bSegmentRelative)
		addTextRelocation(addr, 1, SEG_DATA, false);

	match();
}

//
// Parse a 16b code address or label
//
void AsmParser::codeAddress(int op)
{
	bool bSegmentRelative = false;

	obj.addText(op);
	match();

	uint32_t val = 0;

	if (lookahead == TV_INTVAL)
	{
		val = yylval.ival;
	}
	else if (lookahead == TV_ID) 
	{
		val = yylval.sym->ival;

		yylval.sym->isReferenced = true;

		if (yylval.sym->type != stEqu)
			bSegmentRelative = true;
	}
	else 
	{
		yyerror("syntax error");
	}

	auto addr = obj.addText(LOBYTE(val));
	obj.addText(HIBYTE(val));

	// see if we have to fixup a forward address
	if (yylval.sym->type == stUndef)
		addFixup(yylval.sym->lexeme, addr);

	if (bSegmentRelative)
	{
		auto external = yylval.sym->type == stExternal ? true : false;
		uint32_t index = SEG_TEXT;
		if (external)
		{
			// for stExternal this needs to be the index to the symbol table entry
			index = obj.indexOfSymbol(yylval.sym->lexeme);
			assert(index != UINT_MAX);
		}

		addTextRelocation(addr, 1, index, external);
	}

	match();
}

//
//
//
void AsmParser::include()
{
	match(TV_INCLUDE);
		yylval.sym->isReferenced = true;
		m_lexer->pushFile(yylval.sym->lexeme.c_str());
	match(TV_STRING);
}

//
// Parse file-level constructs
//
void AsmParser::file()
{
	SymbolEntity se;

	while (lookahead != TV_DONE)
	{
		auto sym = yylval.sym;

		switch (lookahead)
		{
		case TV_INCLUDE:
			include();
			break;

		case TV_EXTERN:
			match();

			// mark the symbol as an external reference
			yylval.sym->type = stExternal;

			se.type = SET_EXTERN | SET_UNDEFINED;
			se.value = 0;
			obj.addSymbol(yylval.sym->lexeme, se);

			match();
			break;

		case TV_PROC:
			match();

			yylval.sym->type = stProc;
			yylval.sym->global = true;
			yylval.sym->isReferenced = true;

			se.type = SET_TEXT;
			se.value = obj.getTextSize();
			obj.addSymbol(yylval.sym->lexeme, se);

			yylval.sym->ival = se.value;

			applyFixups(yylval.sym->lexeme, obj.getTextSize());
			match();

			break;

		case TV_ID:
			label();
			break;

		case TV_DB:
			dataByte(sym);
			break;

		case TV_DW:
			dataWord(sym);

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

		// loads
		case TV_LDA:
			memOperand(OP_LDAI, OP_LDA);
			break;

		case TV_LDX:
			memOperand(OP_LDXI, OP_LDX, true);
			break;

		case TV_LXX:
			obj.addText(OP_LXX);
			match();
			break;

		case TV_LAX:
			obj.addText(OP_LAX);
			match();
			break;

		case TV_LEAX:
			match();
			imm8(OP_LEAX);
			break;

		// stores
		case TV_STA:
			dataAddress(OP_STA);
			break;
		
		case TV_STX:
			dataAddress(OP_STX);
			break;

		case TV_STAX:
			obj.addText(OP_STAX);
			match();
			break;

		// stack
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

		// branches
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
			yyerror("unrecognized assembly instruction!");
		}
	}
}

//
//
//
int AsmParser::yyparse()
{
	BaseParser::yyparse();

	file();

	//m_pSymbolTable->dumpContents();

	// report any undefined symbols
	bool bMissingSymbols = false;
	for (auto it = fixups.begin(); it != fixups.end(); it++)
	{
		fprintf(stderr, "error: undefined symbol \"%s\"\n", it->first.c_str());
		bMissingSymbols = true;
	}

	if (bMissingSymbols)
	{
		fprintf(stderr, "error: Assembly failed!\n");
		exit(-1);
	}

	// write out the OBJ file
	obj.writeFile(g_szOutputFilename);

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
