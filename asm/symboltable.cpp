#include <assert.h>
#include <list>
#include <map>
#include "symboltable.h"

//
//
//
static const char *_typeStrings[] = 
{
	"undef", 
	"enum",
	"string literal",
	"float",
	"int", 
	"char", 
	"define",
	"user-defined"
};

// ensure that the array matches the enumeration
static_assert(ARRAY_SIZE(_typeStrings) == stNumSymbolTypes, "array size mismatch");

//
//
//
SymbolTable::SymbolTable()
{
	// add the first level to the table
	m_symbolTable.push_back(SymbolMap());
}

//======================================================================
// Add another depth level to the symbol table
//======================================================================
void SymbolTable::push()
{
	m_symbolTable.push_back(SymbolMap());
}

//======================================================================
// Pop a level off the symbol table stack
//======================================================================
void SymbolTable::pop()
{
	m_symbolTable.pop_back();

	// ensure that we don't underflow the stack!
	assert(m_symbolTable.size() >= 0);
}

//
//
//
SymbolEntry *SymbolTable::getFirstGlobal()
{
	stack_iterator iter = m_symbolTable.begin();
	m_globalIter = (*iter).begin();
	return &m_globalIter->second;
}

//
//
//
SymbolEntry *SymbolTable::getNextGlobal()
{
	m_globalIter++;
	stack_iterator iter = begin_stack();

	if (m_globalIter == (*iter).end())
		return nullptr;

	return &m_globalIter->second;
}

//
//
//
const char *SymbolTable::getTypeName(SymbolType st)
{
	assert(st < stNumSymbolTypes);
	return _typeStrings[st];
}

//
void SymbolTable::dumpContents()
{
	char szText[256];
	SymbolMap::iterator iter;
	SymbolStack::reverse_iterator riter = m_symbolTable.rbegin();

	// for each level of table
	for (; riter != m_symbolTable.rend(); riter++)
	{
		iter = (*riter).begin();
		for (; iter != (*riter).end(); iter++)
		{
			if (iter->second.type == stInteger)
				sprintf_s(szText, "%s\t(%u, 0x%08X)\n", iter->first.c_str(), iter->second.ival, iter->second.ival);
			else
				sprintf_s(szText, "%s\t%f\n", iter->first.c_str(), iter->second.fval);

			printf(szText);
		}
	}
}

//===============================================================
// Look for a symbol in the nested symbol stack
//===============================================================
SymbolEntry *SymbolTable::lookup(const char *lexeme)
{
	SymbolMap::iterator iter;
	SymbolStack::reverse_iterator riter = m_symbolTable.rbegin();

	for (; riter != m_symbolTable.rend(); riter++)
	{
		// if we are done with this level, search next highest level
		iter = (*riter).find(lexeme);
		if (iter == (*riter).end())
			continue;

		return &(iter->second);
	}

	// symbol was not found anywhere in the table
	return nullptr;
}

//===============================================================
// Look for a symbol in the nested symbol stack
//===============================================================
SymbolEntry *SymbolTable::reverse_lookup(int ival)
{
	SymbolMap::iterator iter;
	SymbolStack::reverse_iterator riter = m_symbolTable.rbegin();

	for (; riter != m_symbolTable.rend(); riter++)
	{
		iter = (*riter).begin();
		for (; iter != (*riter).end(); iter++)
		{
			if (iter->second.ival == ival)
				return &(iter->second);
		}
	}

	// symbol was not found anywhere in the table
	return nullptr;
}

//======================================================================
// Install given lexeme in the symbol table at the current level.
// Duplicates are not allowed.
//======================================================================
SymbolEntry *SymbolTable::install(const char *lexeme, SymbolType type)
{
	SymbolMap &currentMap = m_symbolTable.back();

	// see if already in table
	SymbolEntry se;
	se.type = type;
	se.lexeme = lexeme;
	std::pair<SymbolMap::iterator, bool> result = currentMap.insert(SymbolMap::value_type(lexeme, se));

	// if symbol already exist in the table at this level, validate it
	if (!result.second)
		assert(result.first->second.type == type);

	return &(result.first->second);
}

//
void SymbolTable::dumpUnreferencedSymbolsAtCurrentLevel()
{
	SymbolMap::iterator iter;
	SymbolStack::reverse_iterator current_level_riter = m_symbolTable.rbegin();
	SymbolEntry *pSymbol;

	// foreach symbol at the current top of stack
	iter = (*current_level_riter).begin();
	for (; iter != (*current_level_riter).end(); iter++)
	{
		pSymbol = &(iter->second);

		if (!pSymbol->isReferenced)
		{
			printf("%s(%d) : warning: %s '%s' not referenced.\n", 
				pSymbol->srcFile.c_str(),
				pSymbol->srcLine,
				_typeStrings[pSymbol->type],
				pSymbol->lexeme.c_str()
				);
		}
	}
}
