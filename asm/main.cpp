#include "../aout.h"
#include <stdio.h>
#include "baseparser.h"

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

	void DoFile();
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
	TV_A,
	TV_X,
	TV_CC,
	TV_SP,
	TV_PC,
	TV_CALL,
	TV_RET,
	TV_EQU
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
void AsmParser::DoFile()
{
	//DoIncludes();
	while (lookahead != TV_DONE)
	{
		SymbolEntry *sym;

		switch (lookahead)
		{
		case TV_ID:
			// we found a new symbol name
			sym = yylval.sym;

			match();

			if (lookahead == ':')
			{
				match(':');
			}
			else if (lookahead == TV_EQU)
			{
				match();
				if (lookahead == TV_INTVAL)
				{
					sym->ival = yylval.ival;
					match();
				}
			}
			break;

		case TV_NOP:
			obj.addText(0);
			match();

			break;

		case TV_ADD:
			obj.addText(1);
			break;

		default:
			yyerror("syntax error!");
		}
	}
}

//
//
//
int AsmParser::yyparse()
{
	BaseParser::yyparse();

	DoFile();

	//if (yydebug)
	//	root.dumpAll();

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
