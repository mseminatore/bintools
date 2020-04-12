#pragma once

#include <vector>
#include <map>

#ifndef __AOUT_H
#define __AOUT_H

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

struct RelocationEntry
{
	int address;	// offset within the segment (data or text) of the relocation item
	unsigned int	symbolnum : 24,
					pcrel : 1,
					length : 2,
					external : 1,
					spare : 4;
					//r_baserel : 1,
					//r_jmptable : 1,
					//r_relative : 1,
					//r_copy : 1;
};

static_assert(sizeof(RelocationEntry) == 8, "Invalid relocation entry!");

class ObjectFile
{
public:
	ObjectFile() {}
	virtual ~ObjectFile() {}
};

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

	//std::map<> symbol_table;

public:
	AoutFile();
	virtual ~AoutFile();

	int writeFile(FILE *fptr);
	int readFile(FILE *fptr);

	void addBSS(size_t size);

	void addText(uint8_t item);
	void addData(uint8_t item);
	void addSymbol();
	void addString();

	// relocations
	void addTextRelocation(RelocationEntry&);
	void addDataRelocation(RelocationEntry&);

	// debug output
	void dumpHeader(FILE*);
	void dumpText(FILE*);
	void dumpData(FILE*);

	void hexDumpGroup(FILE *f, uint8_t *buf);
	void hexDumpLine(FILE *f, uint32_t offset, uint8_t *buf);
	void hexDumpSegment(FILE *f, uint8_t *seg, size_t size);
};

#endif __AOUT_H
