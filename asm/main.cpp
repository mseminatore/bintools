#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include "../cpu_cisc.h"
#include <stdio.h>
#include "./ParserKit/baseparser.h"

//
// Command line switches
//
bool g_bDebug = false;
const char *g_szOutputFilename = "a.out";

enum
{
	stEqu = stUser,
	stLabel,
	stProc,
	stExternal,
	stDataByte,
	stDataWord,
	stDataString,
	stDataMemory
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
	void dataMemory(SymbolEntry *sym);

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
	
//	void parseSTX();

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
	TV_ADC,
	TV_SUB,
	TV_SBB,
	TV_AAX,
	TV_AAY,

	TV_SHL,
	TV_SHR,

	TV_CMP,
	TV_CMPX,
	TV_CMPY,

	TV_LDA,
	TV_LDX,
	TV_LDY,

	TV_STA,
	TV_STX,
	TV_STY,
	TV_STAX,
	TV_STAY,

	TV_STYX,
	TV_STXY,

	TV_LAX,
	TV_LEAX,
	TV_LXX,

	TV_LAY,
	TV_LEAY,
	TV_LYY,

	TV_A,
	TV_X,
	TV_Y,
	TV_CC,
	TV_SP,
	TV_PC,
	
	TV_DB,		// reserve data byte
	TV_DW,		// reserve data word
	TV_DS,		// reserve data string
	TV_DM,		// reserve memory bytes

	TV_EQU,
	TV_ORG,

	TV_CALL,
	TV_RET,
	TV_JMP,
	TV_JNE,
	TV_JEQ,
	TV_JGT,
	TV_JLT,

	TV_RTI,

	TV_AND,
	TV_OR,
	TV_XOR,
	TV_NOT,
	TV_COM,
	TV_NEG,

	TV_PUSH,
	TV_POP,

	TV_OUT,
	TV_IN,

	TV_BRK,
	TV_SWI,

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
	{ "ADC",	TV_ADC },
	{ "SUB",	TV_SUB },
	{ "SBB",	TV_SBB },
	{ "AAX",	TV_AAX },
	{ "AAY",	TV_AAY },

	{ "SHL",	TV_SHL },
	{ "SHR",	TV_SHR },

	{ "CMP",	TV_CMP },
	{ "CMPX",	TV_CMPX },
	{ "CMPY",	TV_CMPY },

	{ "AND",	TV_AND },
	{ "OR",		TV_OR },
	{ "NOT",	TV_NOT },
	{ "XOR",	TV_XOR },

	{ "CALL",	TV_CALL },
	{ "JMP",	TV_JMP },
	{ "JNE",	TV_JNE },
	{ "JEQ",	TV_JEQ },
	{ "JGT",	TV_JGT },
	{ "JLT",	TV_JLT },

	{ "RET",	TV_RET },
	{ "RTI",	TV_RTI },

	{ "ORG",	TV_ORG},
	{ "EQU",	TV_EQU },
	{ "DB",		TV_DB},
	{ "DW",		TV_DW},
	{ "DS",		TV_DS },
	{ "DM",		TV_DM },

	// stores
	{ "STA",	TV_STA },
	{ "STX",	TV_STX },
	{ "STY",	TV_STY },
	{ "STAX",	TV_STAX },
	{ "STAY",	TV_STAY },
	{ "STYX",	TV_STYX },
	{ "STXY",	TV_STXY },

	// loads
	{ "LDA",	TV_LDA },
	{ "LDX",	TV_LDX },
	{ "LDY",	TV_LDY },
	{ "LAX",	TV_LAX },
	{ "LAY",	TV_LAY },
	{ "LEAX",	TV_LEAX },
	{ "LEAY",	TV_LEAY },
	{ "LXX",	TV_LXX },
	{ "LYY",	TV_LYY },

	{ "A",		TV_A },
	{ "X",		TV_X },
	{ "Y",		TV_Y },
	{ "CC",		TV_CC },
	{ "SP",		TV_SP },
	{ "PC",		TV_PC },

	{ "PUSH",	TV_PUSH },
	{ "POP",	TV_POP },

	{ "BRK",	TV_BRK},
	{ "SWI",	TV_SWI},

	{ "OUT",	TV_OUT },
	{ "IN",		TV_IN },

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
	puts("\nusage: asm [options] filename\n");
	puts("-v\tverbose output");
	puts("-o file\tset output filename\n");

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
AsmParser::AsmParser() : BaseParser(std::make_unique<SymbolTable>())
{
	m_lexer = std::make_unique<LexicalAnalyzer>(_tokenTable, this, &yylval);

	// setup our lexical options
	m_lexer->setCharLiterals(true);
	m_lexer->setASMComments(true);
	m_lexer->setHexNumbers(true);
	m_lexer->caseSensitive(false);
}

// We saw a new symbol declaration. Fixup any intra-segment forward references to this symbol
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

// We found an inferred forward reference to a symbol, add the location to fixups list
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
void AsmParser::dataMemory(SymbolEntry *sym)
{
	SymbolEntity se;

	match();

	if (lookahead != TV_INTVAL)
		expected(TV_INTVAL);
	else
	{
		se.type = SET_BSS;
		se.value = obj.allocBSS(yylval.ival);
		match();
	}

	// if the data byte was named, add to the symtable
	if (sym)
	{
		sym->type = stDataMemory;
		sym->ival = se.value;
		obj.addSymbol(sym->lexeme, se);
	}
}

// Allocate a data byte in the data segment
void AsmParser::dataByte(SymbolEntry *sym)
{
	SymbolEntity se;

	match();

	// handle uninitilized data decl
	if (lookahead == '?')
	{
		match();

		se.type		= SET_BSS;
		se.value	= obj.allocBSS(BYTE_SIZE);
	}
	else
	{
		se.type = SET_DATA;
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
	}

	// if the data byte was named, add to the symtable
	if (sym)
	{
		sym->type = stDataByte;
		sym->ival = se.value;
		obj.addSymbol(sym->lexeme, se);
	}
}

// Allocate a data word in the data segment
void AsmParser::dataWord(SymbolEntry *sym)
{
	SymbolEntity se;

	match();

	if (lookahead == '?')
	{
		match();

		se.type = SET_BSS;
		se.value = obj.allocBSS(WORD_SIZE);
	}
	else
	{
		se.type = SET_DATA;

		auto val = yylval.ival;

		se.value = obj.addData(LOBYTE(val));
		obj.addData(HIBYTE(val));

		match();
	}

	// if the data word was named, add to the symtable
	if (sym)
	{
		sym->type = stDataWord;
		sym->ival = se.value;
		obj.addSymbol(sym->lexeme, se);
	}
}

// Allocate a data string in the data segment
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

	// make sure it is null terminated in the data segment
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

// Parse a label or symbol name
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
	
	case TV_DM:
		dataMemory(sym);
		break;

	default:
		yyerror("syntax error");
		break;
	}
}

// Parse a single register
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

	case TV_Y:
		reg = REG_Y;
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

// Parse a set of registers
uint8_t AsmParser::regSet()
{
	uint8_t val, regs = 0;

	while ((val = reg()))
	{
		regs |= val;
		if (lookahead == ',')
			match();
	}

	return regs;
}

// Add a code segment relocation entry
void AsmParser::addTextRelocation(uint32_t addr, uint32_t length, uint32_t index, bool external)
{
	RelocationEntry re;

	re.address = addr;
	re.length = length;
	re.index = index;
	re.external = external ? 1 : 0;

	obj.addTextRelocation(re);
}

// We are expecting an 8-bit immediate value or named immediate value
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

// We are expecting a 16-bit immediate value or a named immediate value
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
		//if (yylval.sym->type == stUndef)
		//{
		//	yyerror("undefined symbol '%s'", yylval.sym->lexeme.c_str());
		//}
	}
	else
	{
		yyerror("invalid operand!");
	}

	auto addr = obj.addText(LOBYTE(val));
	obj.addText(HIBYTE(val));

	// see if we have to fixup a forward address
	if (lookahead == TV_ID && yylval.sym->type == stUndef)
		addFixup(yylval.sym->lexeme, addr);

	// if this is a data address it needs a relocation entry
	if (bSegmentRelative)
	{
		auto external = yylval.sym->type == stExternal ? true : false;

		uint32_t index = SEG_DATA;

		// handle CODE PTR's
		if (yylval.sym->type == stProc)
			index = SEG_TEXT;

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
bool AsmParser::isDataLabel(int type)
{
	if (type == stDataByte || type == stDataWord || type == stDataString || type == stDataMemory)
		return true;
	return false;
}

// Parse an immediate value or an address
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

// Parse a 16b data address or label
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
		yyerror("Syntax error: expected 16b data address or label.");
	}

	auto addr = obj.addText(LOBYTE(val));
	obj.addText(HIBYTE(val));

	if (bSegmentRelative)
		addTextRelocation(addr, 1, SEG_DATA, false);

	match();
}

// Parse a 16b code address or label
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
		yyerror("Syntax error: expected 16b code address or label.");
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

// Parse include files
void AsmParser::include()
{
	match(TV_INCLUDE);
		yylval.sym->isReferenced = true;
		m_lexer->pushFile(yylval.sym->lexeme.c_str());
	match(TV_STRING);
}

//
//void AsmParser::parseSTX()
//{	
//	uint8_t offset = 0;
//
//	if (lookahead != TV_INTVAL && lookahead != TV_ID && lookahead != ',')
//	{
//		dataAddress(OP_STX);
//		return;
//	}
//
//	// look for an offset
//	if (lookahead == TV_INTVAL)
//	{
//		offset = yylval.ival;
//		match();
//	} else if (lookahead == TV_ID)
//	{
//		assert(yylval.sym->type == stEqu);
//		offset = yylval.sym->ival;	// TODO - check for range error?
//		match();
//	}
//
//	match(',');
//	obj.addText(OP_STXR);
//}

// Parse file-level constructs
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
			// external references don't generate unreferenced warnings
			yylval.sym->type = stExternal;
			yylval.sym->isReferenced = true;

			se.type = SET_EXTERN | SET_UNDEFINED;
			se.value = 0;
			obj.addSymbol(yylval.sym->lexeme, se);

			match();
			break;

		case TV_PROC:
			match();

			yylval.sym->type			= stProc;
			yylval.sym->global			= true;
			yylval.sym->isReferenced	= true;	// OK if procedures aren't referenced

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
			break;

		case TV_DM:
			dataMemory(sym);
			break;

		case TV_NOP:
			obj.addText(OP_NOP);
			match();
			break;

		case TV_SHL:
			match();
			imm8(OP_SHL);
			break;

		case TV_SHR:
			match();
			imm8(OP_SHR);
			break;

		case TV_ADD:
			memOperand(OP_ADDI, OP_ADD);
			break;

		case TV_ADC:
			memOperand(OP_ADCI, OP_ADC);
			break;

		case TV_AAX:
			obj.addText(OP_AAX);
			match();
			break;

		case TV_AAY:
			obj.addText(OP_AAY);
			match();
			break;

		case TV_SUB:
			memOperand(OP_SUBI, OP_SUB);
			break;

		case TV_SBB:
			memOperand(OP_SBBI, OP_SBB);
			break;

		case TV_CMP:
			memOperand(OP_CMPI, OP_CMP);
			break;

		case TV_CMPX:
			memOperand(OP_CMPXI, OP_CMPX);
			break;

		case TV_CMPY:
			memOperand(OP_CMPYI, OP_CMPY);
			break;

		case TV_AND:
			memOperand(OP_ANDI, OP_AND);
			break;

		case TV_OR:
			memOperand(OP_ORI, OP_OR);
			break;

		case TV_NOT:
			obj.addText(OP_NOT);
			match();
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

		case TV_LDY:
			memOperand(OP_LDYI, OP_LDY, true);
			break;

		case TV_LXX:
			obj.addText(OP_LXX);
			match();
			break;

		case TV_LYY:
			obj.addText(OP_LYY);
			match();
			break;

		case TV_LAX:
			obj.addText(OP_LAX);
			match();
			break;

		case TV_LAY:
			obj.addText(OP_LAY);
			match();
			break;

		case TV_LEAX:
			match();
			imm8(OP_LEAX);
			break;

		case TV_LEAY:
			match();
			imm8(OP_LEAY);
			break;

		// stores
		case TV_STA:
			dataAddress(OP_STA);
			break;
		
		case TV_STX:
			dataAddress(OP_STX);
			break;

		case TV_STY:
			dataAddress(OP_STY);
			break;

		case TV_STYX:
			obj.addText(OP_STYX);
			match();
			break;

		case TV_STXY:
			obj.addText(OP_STXY);
			match();
			break;

		case TV_STAX:
			obj.addText(OP_STAX);
			match();
			break;

		case TV_STAY:
			obj.addText(OP_STAY);
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

		case TV_JGT:
			codeAddress(OP_JGT);
			break;

		case TV_JLT:
			codeAddress(OP_JLT);
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

		case TV_OUT:
			match();
			imm8(OP_OUT);
			break;

		case TV_IN:
			match();
			imm8(OP_IN);
			break;

		case TV_SWI:
			obj.addText(OP_SWI);
			match();
			break;

		case TV_BRK:
			obj.addText(OP_BRK);
			match();
			break;

		default:
			yyerror("unrecognized assembly instruction!");
		}
	}
}

// Top-level of parser
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
int main(int argc, char* argv[])
{
	if (argc == 1)
		usage();

	int iFirstArg = getopt(argc, argv);

	// TODO - #21 handle multiple files on the command line?
	{
		AsmParser parser;

		parser.yydebug = g_bDebug;

		parser.parseFile(argv[iFirstArg]);

		parser.addWarningCount(parser.reportUnreferencedSymbols());

		if (parser.getWarningCount())
			printf("\nWarnings generated: %d\n", parser.getWarningCount());

		printf("\nAssembly complete %s -> %s\n", argv[iFirstArg], g_szOutputFilename);
	}


	return 0;
}
