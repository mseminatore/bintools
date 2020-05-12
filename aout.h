#ifndef __AOUT_H
#define __AOUT_H

#pragma once

#define _CRT_SECURE_NO_WARNINGS

#define LOBYTE(val) ((val) & 0xFF)
#define HIBYTE(val) (((val) & 0xFF00) >> 8)

#include <vector>
#include <map>

struct AOUT_HEADER
{
	int a_magic;	// magic number
	int a_text;		// text segment size
	int a_data;		// initialized data size
	int a_bss;		// uninitialized data size
	int a_syms;		// symbol table size
	int a_entry;	// entry point
	int a_trsize;	// text relocation size
	int a_drsize;	// data relocation size
};

static_assert(sizeof(AOUT_HEADER) == 32, "Invalid a.out header size!");

// Segment types for relocation entries
enum
{
	SEG_TEXT,
	SEG_DATA,
	SEG_BSS
};

// Symbol Entity types
enum
{
	SET_EXTERN = 1,		// globally visible symbol
	SET_TEXT = 2,		// TEXT segment symbol
	SET_DATA = 4,		// DATA segment symbol
	SET_BSS = 8,		// BSS segment symbol
	SET_ABS = 16,		// absolute non-relocatable item. Usually debugging symbol
	SET_UNDEFINED = 32	// symbol not defined in this module
};

struct RelocationEntry
{
	uint32_t	address;		// offset within the segment (data or text) of the relocation item
	uint32_t	index : 24,		// if extern is true, index number into the symbol table of this item, otherwise it identifies which segment text/data/bss
				pcrel : 1,		// is the address relative to PC
				length : 2,		// byte size of the entry 0,1,2,3 = 1,2,4,8
				external : 1,	// is the symbol external to this segment
				spare : 4;		// unused
				//r_baserel : 1,
				//r_jmptable : 1,
				//r_relative : 1,
				//r_copy : 1;
	RelocationEntry()
	{
		address = index = pcrel = length = external = spare = 0;
	}
};

static_assert(sizeof(RelocationEntry) == 8, "Invalid relocation entry!");

//
//
//
struct SymbolEntity
{
	uint32_t nameOffset;	// offset in the string table of the null-terminated name of the symbol
	uint32_t type;
	uint32_t value;
};

static_assert(sizeof(SymbolEntity) == 12, "Invalid symbol entry!");

//
// TODO - is this even needed?
//
class ObjectFile
{
public:
	ObjectFile() {}
	virtual ~ObjectFile() {}
};

//
//
//
class AoutFile : public ObjectFile
{
protected:
	AOUT_HEADER file_header;

	using Segment = std::vector<uint8_t>;
		
	Segment text_segment;
	Segment data_segment;

	using Relocations = std::vector<RelocationEntry>;
	Relocations textRelocs;
	Relocations dataRelocs;

	// Make this a vector with a map of name/index pairs for lookup.
	using SymbolTable = std::vector<std::pair<std::string, SymbolEntity>>;
	using SymbolLookup = std::map<std::string, size_t>;

	SymbolTable symbolTable;
	SymbolLookup symbolLookup;

	using StringTable = std::vector<char>;
	StringTable stringTable;

	uint32_t textBase, dataBase, bssBase;

public:
	AoutFile();
	virtual ~AoutFile();

	void concat(const AoutFile *rhs);

	int writeFile(FILE *fptr);
	int writeFile(const std::string &name);
	int readFile(FILE *fptr);
	int readFile(const std::string &name);

	// code/data segments
	uint32_t addText(uint8_t item);
	uint32_t addData(uint8_t item);
	uint32_t allocBSS(size_t size);

	uint32_t getTextSize() { return text_segment.size(); }
	uint32_t getDataSize() { return data_segment.size(); }
	uint32_t getBssSize() { return file_header.a_bss; }

	void setTextBase(uint32_t base) { textBase = base; }
	void setDataBase(uint32_t base) { dataBase = base; }
	void setBssBase(uint32_t base) { bssBase = base; }

	uint32_t getTextBase() { return textBase; }
	uint32_t getDataBase() { return dataBase; }
	uint32_t getBssBase() { return bssBase; }

	uint8_t *textPtr() { return text_segment.data(); }
	uint8_t *dataPtr() { return data_segment.data(); }

	void setEntryPoint(uint16_t val) { file_header.a_entry = val; }

	// symbols
	void addSymbol(const std::string &name, SymbolEntity &sym);
	uint32_t addString(const std::string &name);
	size_t indexOfSymbol(const std::string &name);
	SymbolEntity symbolAt(size_t index);
	bool findSymbol(const std::string &name, SymbolEntity &sym);

	// relocations
	void addTextRelocation(RelocationEntry&);
	void addDataRelocation(RelocationEntry&);
	void relocate(const std::vector<AoutFile*>&);

	// debug output
	void dumpHeader(FILE*);
	void dumpText(FILE*);
	void dumpData(FILE*);
	void dumpTextRelocs(FILE*);
	void dumpDataRelocs(FILE*);
	void dumpSymbols(FILE*);

	void hexDumpGroup(FILE *f, uint8_t *buf);
	void hexDumpLine(FILE *f, uint32_t offset, uint8_t *buf);
	void hexDumpSegment(FILE *f, uint8_t *seg, size_t size);
};

#endif __AOUT_H

