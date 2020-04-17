#pragma once

#ifndef __LEXER_H
#define __LEXER_H

//
// forward declarations
//
struct SymbolEntry;
class BaseParser;

//======================================================================
//
//======================================================================
#define DEFAULT_NUM_BUF		32
#define DEFAULT_TEXT_BUF	2048

// predefined token values
enum {
	TV_ERROR = 256,
	TV_DONE,

	// numeric values
	TV_INTVAL,
	TV_FLOATVAL,
	TV_CHARVAL,

	// string literal
	TV_STRING,
	
	// identifier
	TV_ID,

	TV_USER
};

//
struct TokenTable
{
	const char *lexeme;
	int token;
};

//
union YYSTYPE
{
	// literal values
	int		ival;
	float	fval;
	char	char_val;

	SymbolEntry *sym;	// ID value
	TokenTable *ptt;	// keyword
};

//
//
//
class LexicalAnalyzer
{
protected:
	// File descriptor node
	struct FDNode
	{
		FILE *fdDocument;
		char *pTextData;
		std::string filename;
		int column;
		int yylineno;
		void *pUserData;

		FDNode() : fdDocument(nullptr), pTextData(nullptr), filename(""), column(0), yylineno(1), pUserData(nullptr) {}
		virtual ~FDNode() 
		{
			if (fdDocument)
				fclose(fdDocument);
		}
	};

	int m_iTotalLinesParsed;
	
	//char m_szCurrentSourceLineText[256];
	//int m_iCurrentSourceLineIndex;

	// TODO - this should be a std::vector instead!
	using FDStack = std::vector<FDNode>;
	FDStack m_fdStack;

	BaseParser *m_pParser;

	YYSTYPE *m_yylval;

	bool m_bUnixComments;
	bool m_bCPPComments;
	bool m_bCStyleComments;
	bool m_bHexNumbers;
	bool m_bCharLiterals;

	bool m_bCaseInsensitive;

	int (*compare_function)(const char*, const char*);

	struct ltstr
	{
 		bool operator()(const std::string &s1, const std::string &s2) const
		{
			return strcmp(s1.c_str(), s2.c_str()) < 0;
		}
	};

	using TokenTableMap = std::map<std::string, int, ltstr>;
	TokenTableMap m_tokenTable;

	// methods to help with lexical processing
	// yylex() will use these to find tokens
	int skipLeadingWhiteSpace();
	int follow(int expect, int ifyes, int ifno);
	int backslash(int c);

	// comments
	void skipToEOL(void);
	void cstyle_comment(void);

	int getNumber();
	int getStringLiteral();
	int getCharLiteral();
	int getKeyword();

	int getChar();
	int ungetChar(int c);

public:
	LexicalAnalyzer(TokenTable *atokenTable, BaseParser *pParser, YYSTYPE *pyylval);
	virtual ~LexicalAnalyzer() = default;

	//const char *GetCurrentSourceText() { return m_szCurrentSourceLineText; }
	//void ClearCurrentSourceText()		{ m_szCurrentSourceLineText[0] = 0; }

	int pushFile(const char *theFile);
	int popFile();
	std::string getFile() const { return m_fdStack.back().filename; }

	int setData(char *theData, const char *fileName, void* pUserData);
	virtual void freeData(void* pUserData);

	const char *getLexemeFromToken(int token);

	void caseSensitive(bool onoff = true);

	int getColumn()					{ return m_fdStack.back().column; }
	int getLineNumber()				{ return m_fdStack.back().yylineno; }
	int getTotalLinesParsed()		{ return m_iTotalLinesParsed; }

	void setUnixComments(bool onoff)	{ m_bUnixComments = onoff; }
	void setCPPComments(bool onoff)		{ m_bCPPComments = onoff; }
	void setCStyleComments(bool onoff)	{ m_bCStyleComments = onoff; }
	void setHexNumbers(bool onoff)		{ m_bHexNumbers = onoff; }
	void setCharLiterals(bool onoff)	{ m_bCharLiterals = onoff; }

	void copyToEOF(FILE *fout);
	void copyUntilChar(int endChar, int nestChar, FILE *fout);
	void copyUntilChar(int endChar, int nestChar, char *buf);

	// functions that may typically be overridden
	virtual int yylex();
	virtual void yyerror(const char *s);
	virtual void yywarning(const char *s);
	virtual bool isidval(int c);
	virtual bool iswhitespace(int c);
	virtual int specialTokens(int chr);
};

#endif	//#ifndef __LEXER_H
