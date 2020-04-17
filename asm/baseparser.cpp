#include "baseparser.h"
#include <assert.h>
#include <stdarg.h>
#include <direct.h>

//======================================================================
//
//======================================================================
BaseParser::BaseParser(std::unique_ptr<SymbolTable> symbolTable)
{
	m_lexer			= nullptr;
	m_errorCount	= 0;
	m_warningCount	= 0;
	m_pSymbolTable = std::move(symbolTable);
}

//
BaseParser::~BaseParser()
{
	m_pSymbolTable->dumpUnreferencedSymbolsAtCurrentLevel();
}

// the parser calls this method to report errors
void BaseParser::yyerror(const Position &pos, const char *fmt, ...)
{
	char buf[SMALL_BUFFER], s[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
	vsprintf_s(buf, fmt, argptr);
	va_end(argptr);

	sprintf_s(s, "%s(%d) : error near column %d: %s\r\n", pos.srcFile.c_str(), pos.srcLine, pos.srcColumn, buf);

	m_errorCount++;

	// delegate error messages to the lexical analyzer
	m_lexer->yyerror(s);
}

// the parser calls this method to report errors
void BaseParser::yyerror(const char *fmt, ...)
{
	char buf[SMALL_BUFFER], s[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
		vsprintf_s(buf, fmt, argptr);
	va_end(argptr);

	sprintf_s(s, "%s(%d) : error near column %d: %s\r\n", m_lexer->getFile().c_str(), m_lexer->getLineNumber(), m_lexer->getColumn(), buf);

	m_errorCount++;

	// delegate error messages to the lexical analyzer
	m_lexer->yyerror(s);
}

// print a warning message
void BaseParser::yywarning(const Position &pos, const char *fmt, ...)
{
	char buf[SMALL_BUFFER], s[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
	vsprintf_s(buf, fmt, argptr);
	va_end(argptr);

	sprintf_s(s, "%s(%d) : warning near column %d: %s\r\n", pos.srcFile.c_str(), pos.srcLine, pos.srcColumn, buf);

	m_warningCount++;

	// delegate error messages to the lexical analyzer
	m_lexer->yywarning(s);
}

// print a warning message
void BaseParser::yywarning(const char *fmt, ...)
{
	char buf[SMALL_BUFFER], s[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
		vsprintf_s(buf, fmt, argptr);
	va_end(argptr);

	sprintf_s(s, "%s(%d) : warning near column %d: %s\r\n", m_lexer->getFile().c_str(), m_lexer->getLineNumber(), m_lexer->getColumn(), buf);

	m_warningCount++;

	// delegate error messages to the lexical analyzer
	m_lexer->yywarning(s);
}

//
void BaseParser::yylog(const char *fmt, ...)
{
	if (!yydebug)
		return;

	char buf[SMALL_BUFFER];
	va_list argptr;

	va_start(argptr, fmt);
		vsprintf_s(buf, fmt, argptr);
	va_end(argptr);

	puts(buf);
}

//
// show an error on unexpected token
//
void BaseParser::expected(int token)
{
	if (token < 256)
		yyerror("expected to see '%c'", token);
	else
		yyerror("expected to see '%s'", m_lexer->getLexemeFromToken(token));
}

//
// attempt to match the given token
//
int BaseParser::match(int token)
{
	if (lookahead == token)
	{
		lookahead = m_lexer->yylex();
	}
	else
	{
		expected(token);
	}

	return lookahead;
}

//
// this method does nothing and is meant to be overridden in sub-classes 
//
int BaseParser::yyparse()
{
	lookahead = m_lexer->yylex();
	return 0;
}

//
int BaseParser::parseFile (const char *filename)
{
	int rv;
	char szFullPath[_MAX_PATH];
	char workingDir[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], file[_MAX_FNAME], ext[_MAX_EXT];
	char oldWorkdingDir[_MAX_PATH];

	assert(filename);

	_fullpath(szFullPath, filename, sizeof(szFullPath));
	_splitpath_s(szFullPath, drive, dir, file, ext);
	sprintf_s(workingDir, "%s%s", drive, dir);
	_getdcwd(_getdrive(), oldWorkdingDir, sizeof(oldWorkdingDir));
	_chdir(workingDir);

	rv = m_lexer->pushFile(filename);
	if (rv != 0)
	{
		yyerror("Couldn't open file: %s", filename);
		_chdir(oldWorkdingDir);
		return rv;
	}

	// TODO - should return the value from yypars()
	yyparse();

	_chdir(oldWorkdingDir);
	return 0;
}

//
int BaseParser::parseData(char *textToParse, const char *fileName, void *pUserData)
{
	int rv;

	assert(textToParse);

	rv = m_lexer->setData(textToParse, fileName, pUserData);
	if (rv != 0)
	{
		yyerror("Couldn't parse text");		//open file: %s", filename);
		return rv;
	}

	// TODO - should return the value from yypars()
	yyparse();

	return 0;
}
