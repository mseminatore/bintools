#ifndef __AOUT_H
#define __AOUT_H

#pragma once

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

enum
{
	SEG_TEXT,
	SEG_DATA,
	SEG_BSS
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

//
//
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

	using SymbolTable = std::map<std::string, SymbolEntity>;
	SymbolTable symbolTable;

	using StringTable = std::vector<char>;
	StringTable stringTable;

public:
	AoutFile();
	virtual ~AoutFile();

	int writeFile(FILE *fptr);
	int readFile(FILE *fptr);


	// code/data segments
	uint32_t addText(uint8_t item);
	uint32_t addData(uint8_t item);
	uint32_t allocBSS(size_t size);
	uint32_t getTextAddress() { return text_segment.size(); }
	uint32_t getDataAddress() { return data_segment.size(); }

	// symbols
	void addSymbol(const std::string &name, const SymbolEntity &sym);
	uint32_t addString(const std::string &name);

	// relocations
	void addTextRelocation(RelocationEntry&);
	void addDataRelocation(RelocationEntry&);

	// debug output
	void dumpHeader(FILE*);
	void dumpText(FILE*);
	void dumpData(FILE*);
	void dumpTextRelocs(FILE*);
	void dumpDataRelocs(FILE*);

	void hexDumpGroup(FILE *f, uint8_t *buf);
	void hexDumpLine(FILE *f, uint32_t offset, uint8_t *buf);
	void hexDumpSegment(FILE *f, uint8_t *seg, size_t size);
};

#endif __AOUT_H

