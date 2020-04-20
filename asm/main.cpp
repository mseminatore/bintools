#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include <stdio.h>
#include "./ParserKit/baseparser.h"

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

	void doLabel();
	void doFile();
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
	TV_STO,

	TV_A,
	TV_X,
	TV_CC,
	TV_SP,
	TV_PC,
	
	TV_DB,
	TV_DW,

	TV_CALL,
	TV_RET,
	TV_EQU,
	TV_JMP,
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
	OP_SUB,
	OP_CALL,
	OP_RET,
	OP_JMP,
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
	{ "A",		TV_A},
	{ "EQU",	TV_EQU},
	{ "JMP",	TV_JMP},
	{ "DB",		TV_DB},
	{ "DW",		TV_DW},

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
void AsmParser::doLabel()
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
		// data word
		match();

		// 
		//sym->ival = obj.addData(yylval.char_val);
		match(TV_INTVAL);
		break;

	default:
		yyerror("syntax error");
		break;
	}
}

//
//
//
void AsmParser::doFile()
{
	//DoIncludes();
	while (lookahead != TV_DONE)
	{
		switch (lookahead)
		{
		case TV_ID:
			doLabel();
			break;

		case TV_NOP:
			obj.addText(OP_NOP);
			match();

			break;

		case TV_ADD:
			obj.addText(OP_ADD);
			match();

			// operand
			match();
			break;
		
		case TV_JMP:
			obj.addText(OP_JMP);
			match();

			// label
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

	doFile();

	//if (yydebug)
	//	root.dumpAll();

	writeFile("a.out");

	return 0;
}

//
// Command line switches
//
bool g_bDebug = false;

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
