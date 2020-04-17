#include "baseparser.h"

//
static const char *_internalTokenLexemes[] = 
{
	"error",
	"EOF",
	"integer value",
	"float value",
	"char value",
	"string literal",
	"identifier"
};

static_assert(ARRAY_SIZE(_internalTokenLexemes) == (TV_USER - TV_ERROR), "lexeme table mismatch");

//======================================================================
//
//======================================================================
LexicalAnalyzer::LexicalAnalyzer(TokenTable *aTokenTable, BaseParser *pParser, YYSTYPE *pyylval)
{
	assert(aTokenTable);
	assert(pParser);
	assert(pyylval);

	m_pParser = pParser;
	m_yylval = pyylval;

	m_iTotalLinesParsed = 0;

	// add tokens to table
	for (; aTokenTable->lexeme; aTokenTable++)
		m_tokenTable[aTokenTable->lexeme] = aTokenTable->token;

	compare_function	= _stricmp;
	m_bCaseInsensitive	= true;

	// setup lexical analysis defaults
	m_bUnixComments		= false;
	m_bCPPComments		= false;
	m_bCStyleComments	= false;
	m_bHexNumbers		= false;
	m_bCharLiterals		= false;

	//m_iCurrentSourceLineIndex = -1;
}

//======================================================================
// Return the next character from the input
//======================================================================
int LexicalAnalyzer::getChar()
{
	m_fdStack.back().column++;

	// get new line if necessary
//	if (m_iCurrentSourceLineIndex == -1)
//		fgets(m_szCurrentSourceLineText, sizeof(m_szCurrentSourceLineText), m_fdStack.back().fdDocument);

	// if parsing files get next file char
	if (m_fdStack.back().fdDocument)
	{
		return fgetc(m_fdStack.back().fdDocument);
	}

	// otherwise return data from memory ptr
	int c = *m_fdStack.back().pTextData;
	m_fdStack.back().pTextData++;
	
	return c;
}

//======================================================================
// Put the character back to the input
//======================================================================
int LexicalAnalyzer::ungetChar(int c)
{
	m_fdStack.back().column--;

	// if parsing files put back file char
	if (m_fdStack.back().fdDocument)
		return ungetc(c, m_fdStack.back().fdDocument);

	// otherwise put back data to memory ptr
	m_fdStack.back().pTextData--;
	return 0;
}

//
//
//
void LexicalAnalyzer::yyerror(const char *s)
{
	puts(s);
	fflush(stdout);
	exit(-1);
}

//
//
//
void LexicalAnalyzer::yywarning(const char *s)
{
	puts(s);
	fflush(stdout);
}

//
//
//
void LexicalAnalyzer::caseSensitive(bool onoff /*= true*/)
{
	compare_function	= (onoff) ? strcmp : _stricmp;
	m_bCaseInsensitive	= onoff;
}

//===============================================================
//
//===============================================================
const char *LexicalAnalyzer::getLexemeFromToken(int token)
{
	// look for single char tokens
	if (token < 256)
		return (char*)token;

	if (token > 255 && token < TV_USER)
		return _internalTokenLexemes[token - 256];

	auto iter = m_tokenTable.begin();
	for (; iter != m_tokenTable.end(); iter++)
	{
		if (iter->second == token)
			return iter->first.c_str();
	}

	return "(unknown token)";
}

//======================================================================
//
//======================================================================
int LexicalAnalyzer::popFile()
{
	// if we were processing a file, close it
	if (m_fdStack.back().fdDocument)
	{
		m_fdStack.pop_back();
		if (m_fdStack.size() == 0)
			return EOF;

		return 0;
	}

	// if we were processing in-memory data, release it
	freeData(m_fdStack.back().pUserData);
	m_fdStack.back().pUserData = nullptr;
	
	// we assume the pTextData is a subset of the pUserData,
	// and was freed when the data was freed
	m_fdStack.back().pTextData = nullptr;

	m_fdStack.pop_back();
	if (m_fdStack.size() == 0)
		return EOF;

	return 0;
}

// this is a no-op  meant to be overridden in derived classes
void LexicalAnalyzer::freeData(void *pUserData)
{
	if (pUserData)
		assert(false);
}

//======================================================================
// Begin processing the given file, pushing the current file onto the
// file descriptor stack.
//======================================================================
int LexicalAnalyzer::pushFile(const char *theFile)
{
	assert(theFile);

	m_fdStack.push_back(FDNode());

	assert(m_fdStack.back().fdDocument == nullptr);
	
	FILE *pFile = nullptr;
	if (fopen_s(&pFile, theFile, "rt"))
		return -1;

	m_fdStack.back().fdDocument = pFile;

	if (!m_fdStack.back().fdDocument)
		return -1;

	m_fdStack.back().filename = theFile;
	m_fdStack.back().yylineno = 1;

	return 0;
}

//======================================================================
//
//======================================================================
int LexicalAnalyzer::setData(char *theData, const char *fileName, void *pUserData)
{
	assert(theData);

	m_fdStack.push_back(FDNode());

	assert(m_fdStack.back().pTextData == nullptr);
	m_fdStack.back().pTextData = theData;

	if (!m_fdStack.back().pTextData)
		return -1;

	// hold onto this for later
	m_fdStack.back().pUserData	= pUserData;
	m_fdStack.back().filename	= fileName;
	m_fdStack.back().yylineno	= 1;

	return 0;
}

//======================================================================
// valid characters for Identifiers
//======================================================================
bool LexicalAnalyzer::isidval(int c)
{
	// removed: c == '.' || c == '!' || c == '&' || ((char)c) == '�'
	if (isalnum(c) || c == '_')
		return true;

	return false;
}

//======================================================================
// characters which are considered to be whitespace
//======================================================================
bool LexicalAnalyzer::iswhitespace(int c)
{
	return (c == ' ' || c == '\t' || c == '\n' || c == '\r') ? true : false;
}

// copy until EOF
void LexicalAnalyzer::copyToEOF(FILE *fout)
{
	int chr;

	assert(fout);

	while ((chr = getChar()) != EOF)
	{
		fputc(chr, fout);
	}
}

// copy until char with nesting
void LexicalAnalyzer::copyUntilChar(int endChar, int nestChar, FILE *fout)
{
	int chr;
	int nestCount = 1;

	assert(fout);

	do
	{
		chr = getChar();
	
		if (chr == nestChar)
			nestCount++;
		if (chr == endChar)
			nestCount--;

		if (nestCount)
			fputc(chr, fout);

	} while (nestCount);

	// put back the last character
	ungetChar(chr);
}

//
void LexicalAnalyzer::copyUntilChar(int endChar, int nestChar, char *buf)
{
	int chr;
	int nestCount = 1;

	assert(buf);

	do
	{
		chr = getChar();

		if (chr == nestChar)
			nestCount++;
		if (chr == endChar)
			nestCount--;

		if (nestCount)
			*buf++ = chr;

	} while (nestCount);

	// make sure its ASCIIZ
	*buf = 0;

	// put back the last character
	ungetChar(chr);
}

// skip any leading WS
int LexicalAnalyzer::skipLeadingWhiteSpace()
{
	int chr;

	do
	{
		chr = getChar();
		if (chr == '\n')
		{
			m_fdStack.back().column = 0;
			m_fdStack.back().yylineno++;
			m_iTotalLinesParsed++;
		}
	} while (iswhitespace(chr));

	return chr;
}

//
//
//
int LexicalAnalyzer::backslash(int c)
{
	static char translation_tab[] = "b\bf\fn\nr\rt\t";

	if (c != '\\')
		return c;

	c = getChar();
	if (islower(c) && strchr(translation_tab, c))
		return strchr(translation_tab, c)[1];

	return c;
}

//
//
//
void LexicalAnalyzer::skipToEOL(void)
{
	int c;

	// skip to EOL
	while ((c = getChar()) != '\n');

	// put last character back
	ungetChar(c);
}

//
//
//
void LexicalAnalyzer::cstyle_comment(void)
{
	int c;

	// skip to EOL
	for (c = getChar(); c != EOF; c = getChar())
	{
		if (c == '*')
		{
			if (follow('/', 1, 0))
				return;
		}
	}
}

//
//
//
int LexicalAnalyzer::follow(int expect, int ifyes, int ifno)
{
	int chr;

	chr = getChar();
	if (chr == expect)
		return ifyes;

	ungetChar(chr);
	return ifno;
}

//
//
//
int LexicalAnalyzer::getNumber()
{
	int c;
	char buf[DEFAULT_NUM_BUF];
	char *bufptr = buf;
	int base = 10;

	// look for hex numbers
 	c = getChar();
	if (c == '0' && (follow('X', 1, 0) || follow('x', 1, 0)))
		base = 16;
	else
		ungetChar(c);

	if (base == 16)
	{
		while (isxdigit(c = getChar()))
			*bufptr++ = c;
	}
	else
	{
		while (isdigit((c = getChar())) || c == '.')
			*bufptr++ = c;
	}
	
	// need to put back the last character
	ungetChar(c);

	// make sure string is asciiz
	*bufptr = '\0';

	// handle floats and ints
	if (!strchr(buf, '.'))
	{
		m_yylval->ival = strtoul(buf, nullptr, base);
		return TV_INTVAL;
	}
	else
	{
		m_yylval->fval = (float)atof(buf);
		return TV_FLOATVAL;
	}
}

//
//
//
int LexicalAnalyzer::getCharLiteral()
{
	int c;
	
	c = backslash(getChar());
	m_yylval->char_val = c;
	c = getChar();
	if (c != '\'')
		yyerror("missing single quote");

	return TV_CHARVAL;
}

//
//
//
int LexicalAnalyzer::getStringLiteral()
{
	SymbolEntry *sym;
	int c;
	char buf[DEFAULT_TEXT_BUF];
	char *cptr = buf;

	c = getChar();

	while (c != '"' && cptr < &buf[sizeof(buf)])
	{
		if (c == '\n' || c == EOF)
			yyerror("missing quote");

		// build up our string, translating escape chars
		*cptr++ = backslash(c);
		c = getChar();
	}

	// make sure its asciiz
	*cptr = '\0';

	sym = m_pParser->lookupSymbol(buf);
	if (!sym)
	{
		sym = m_pParser->installSymbol(buf, stStringLiteral);
		if (!sym)
			yyerror("making symbol table entry");

		sym->srcLine = getLineNumber();
		sym->srcFile = getFile();
	}

	m_yylval->sym = sym;
	return TV_STRING;
}

//
//
//
int LexicalAnalyzer::getKeyword()
{
	return 0;
}

//
//
//
int LexicalAnalyzer::specialTokens(int chr)
{
	switch (chr)
	{
		// we reached end of current file, but could be a nested include so pop
		// file descriptor stack and try to continue
	case 0:
	case EOF:
		if (popFile() == EOF)
			return TV_DONE;

		// call ourselves again to get the next token
		return yylex();

	default:
		return chr;
	}

}

//======================================================================
// Generic Lexical analyzer routine
//======================================================================
int LexicalAnalyzer::yylex()
{
	int chr;
	char buf[DEFAULT_TEXT_BUF];
	char *pBuf = buf;
	SymbolEntry *sym;

yylex01:
	// skip any leading WS
	chr = skipLeadingWhiteSpace();
	
	// process Unix conf style comments
	if (m_bUnixComments && chr == '#')
	{
		skipToEOL();
		goto yylex01;
	}

	// handle C++ style comments
	if (m_bCPPComments && chr == '/')
	{
		if (follow('/', 1, 0))
		{
			skipToEOL();
			goto yylex01;
		}
	}

	// handle C style comments
	if (m_bCStyleComments && chr == '/')
	{
		if (follow('*', 1, 0))
		{
			cstyle_comment();
			goto yylex01;
		}
	}

	// look for a number value
	if (isdigit(chr))
	{
		ungetChar(chr);
		return getNumber();
	}

	// look for char literals
	if (chr == '\'')
	{
		return getCharLiteral();
	}

	// look for string literals
	if (chr == '"')
	{
		return getStringLiteral();
	}

	// look for keywords or ID
	if (isalpha(chr) || chr == '_')
	{
		// get the token
		do 
		{
			*pBuf++ = ((char)chr);
		} while ((chr = getChar()) != EOF && isidval(chr));
		
		ungetChar(chr);
	
		// make sure its asciiz
		*pBuf = 0;

		// search token table for possible match
		auto iterTokens = m_tokenTable.find(buf);
		if (iterTokens != m_tokenTable.end())
		{
			return iterTokens->second;
		}

		// see if symbol is already in symbol table
		sym = m_pParser->lookupSymbol(buf);
		if (!sym)
		{
			sym = m_pParser->installSymbol(buf, stUndef);
			if (!sym)
				yyerror("making symbol table entry");

			sym->srcLine = getLineNumber();
			sym->srcFile = getFile();
		}

		m_yylval->sym = sym;

		return TV_ID;
	}

	return specialTokens(chr);
}

