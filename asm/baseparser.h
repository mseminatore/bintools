#pragma once

#ifndef __BASEPARSER_H
#define __BASEPARSER_H



#include <stdio.h>
#include <assert.h>
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <list>
#include "lexer.h"
#include "symboltable.h"

#define SMALL_BUFFER	512

//
class BaseParser
{
public:
	bool yydebug = false;
	FILE *yyout = stdout;
	FILE *yyhout = stdout;

protected:
	// the lexical analyzer
	std::unique_ptr<LexicalAnalyzer> m_lexer;

	// the value filled in by the lexical analyzer
	YYSTYPE yylval;
	
	// next token in the parse stream
	int lookahead;
	
	// total error count
	unsigned m_errorCount;

	// total warning count
	unsigned m_warningCount;

	// our symbol table
	std::unique_ptr<SymbolTable> m_pSymbolTable;

	std::string outputFileName;

public:
	BaseParser(std::unique_ptr<SymbolTable> symbolTable);
	virtual ~BaseParser();

	unsigned getErrorCount()		{ return m_errorCount; }
	unsigned getWarningCount()		{ return m_warningCount; }

	virtual int parseFile(const char *filename);
	virtual int parseData(char *textToParse, const char *fileName, void *pUserData);
	virtual int yyparse();

	virtual void yyerror(const char *fmt, ...);
	virtual void yyerror(const Position &pos, const char *fmt, ...);

	virtual void yywarning(const char *fmt, ...);
	virtual void yywarning(const Position &pos, const char *fmt, ...);

	virtual void yylog(const char *fmt, ...);

	virtual void expected(int token);
	virtual int match(int token);
	virtual int match() { return match(lookahead); }

	void setFileName(std::string &str)
	{
		outputFileName = str;
	}

	// these methods delegate their work to the symbol table object
	SymbolEntry *installSymbol(char *lexeme, SymbolType st = stUndef)
	{
		return m_pSymbolTable->install(lexeme, st);
	}

	SymbolEntry *lookupSymbol(char *lexeme)
	{
		return m_pSymbolTable->lookup(lexeme);
	}
};

#endif	//__BASEPARSER_H

