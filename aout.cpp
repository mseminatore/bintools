#include "aout.h"
#include <assert.h>
#include <ctype.h>

AoutFile::AoutFile()
{
	// init file header properties
	memset(&file_header, 0, sizeof(file_header));

	// default to NMAGIC format?
	file_header.a_magic = 263;
}

AoutFile::~AoutFile()
{

}

int AoutFile::writeFile(FILE *fptr)
{
	assert(fptr != nullptr);
	if (fptr == nullptr)
		return -1;

	// update the header
	file_header.a_text = text_segment.size();
	file_header.a_data = data_segment.size();
	file_header.a_trsize = textRelocs.size();
	file_header.a_drsize = dataRelocs.size();
	file_header.a_syms = symbolTable.size();

	// write the header
	auto result = fwrite(&file_header, sizeof(file_header), 1, fptr);

	// write the text segment
	fwrite(text_segment.data(), text_segment.size(), 1, fptr);

	// write the data segment
	fwrite(data_segment.data(), data_segment.size(), 1, fptr);

	// write the text relocations
	for (auto iter = textRelocs.begin(); iter != textRelocs.end(); iter++)
	{
		fwrite(&(*iter), sizeof(RelocationEntry), 1, fptr);
	}

	// write the data relocations
	for (auto iter = dataRelocs.begin(); iter != dataRelocs.end(); iter++)
	{
		fwrite(&(*iter), sizeof(RelocationEntry), 1, fptr);
	}

	// write the symbol table
	for (auto iter = symbolTable.begin(); iter != symbolTable.end(); iter++)
	{
		auto se = (*iter).second;
		fwrite(&se, sizeof(se), 1, fptr);
	}

	// write the string table
	for (size_t i = 0; i < stringTable.size(); i++)
	{
		fputc(stringTable[i], fptr);
	}

	return 0;
}

int AoutFile::readFile(FILE *fptr)
{
	assert(fptr != nullptr);
	if (fptr == nullptr)
		return -1;

	// read the header
	auto result = fread(&file_header, sizeof(file_header), 1, fptr);

	// read the text segment
	for (int i = 0; i < file_header.a_text; i++)
	{
		auto c = fgetc(fptr);
		text_segment.push_back(c);
	}

	// read the data segment
	for (int i = 0; i < file_header.a_data; i++)
	{
		auto c = fgetc(fptr);
		data_segment.push_back(c);
	}

	// read the text relocations
	for (int i = 0; i < file_header.a_trsize; i++)
	{
		RelocationEntry re;

		fread(&re, sizeof(re), 1, fptr);
		textRelocs.push_back(re);
	}

	// read the data relocations
	for (int i = 0; i < file_header.a_drsize; i++)
	{
		RelocationEntry re;

		fread(&re, sizeof(re), 1, fptr);
		dataRelocs.push_back(re);
	}

	// read the symbol table
	std::vector<SymbolEntity> st;
	for (int i = 0; i < file_header.a_syms; i++)
	{
		SymbolEntity se;
		fread(&se, sizeof(se), 1, fptr);
		st.push_back(se);
	}

	// read the string table
	char c = fgetc(fptr);
	while (c != EOF)
	{
		stringTable.push_back(c);
		c = fgetc(fptr);
	}

	for (int i = 0; i < file_header.a_syms; i++)
	{
		auto sym = st[i];
		char *pStr = &(stringTable[sym.nameOffset]);
		symbolTable.push_back(SymbolTable::value_type(pStr, sym));
		symbolLookup.insert(SymbolLookup::value_type(pStr, i));
	}

	return 0;
}

uint32_t AoutFile::allocBSS(size_t size)
{
	uint32_t loc = file_header.a_bss;
	file_header.a_bss += size;

	return loc;
}

uint32_t AoutFile::addText(uint8_t item)
{
	uint32_t addr = text_segment.size();
		text_segment.push_back(item);
	return addr;
}

uint32_t AoutFile::addData(uint8_t item)
{
	uint32_t addr = data_segment.size();
		data_segment.push_back(item);
	return addr;
}

void AoutFile::addSymbol(const std::string &name, SymbolEntity &sym)
{
	auto it = symbolLookup.find(name);

	// TODO - return if symbol already exists??
	if (it != symbolLookup.end())
		return;

	uint32_t offset = addString(name);
	sym.nameOffset = offset;

	auto index = symbolTable.size();

	symbolTable.push_back(SymbolTable::value_type(name, sym));
	symbolLookup.insert(SymbolLookup::value_type(name, index));
}

uint32_t AoutFile::addString(const std::string &name)
{
	uint32_t offset = stringTable.size();

	for (size_t i = 0; i < name.size(); i++)
		stringTable.push_back(name[i]);

	// make it asciiz
	stringTable.push_back(0);

	return offset;
}

size_t AoutFile::indexOfSymbol(const std::string &name)
{
	auto iter = symbolLookup.find(name);

	if (iter == symbolLookup.end())
		return UINT_MAX;

	return iter->second;
}

void AoutFile::addTextRelocation(RelocationEntry &r)
{
	textRelocs.push_back(r);
}

void AoutFile::addDataRelocation(RelocationEntry &r)
{
	dataRelocs.push_back(r);
}

void AoutFile::dumpHeader(FILE *f)
{
	assert(f != nullptr);
	if (f == nullptr)
		return;

	fprintf(f, "     Magic Number: 0x%X\n", file_header.a_magic);
	fprintf(f, "Text Segment size: 0x%04X (%d) bytes\n", file_header.a_text, file_header.a_text);
	fprintf(f, "Data Segment size: 0x%04X (%d) bytes\n", file_header.a_data, file_header.a_data);
	fprintf(f, " BSS Segment size: 0x%04X (%d) bytes\n", file_header.a_bss, file_header.a_bss);
	fprintf(f, "Symbol Table size: 0x%04X (%d) bytes\n", file_header.a_syms, file_header.a_syms);
	fprintf(f, " Main Entry Point: 0x%04X\n", file_header.a_entry);
	fprintf(f, " Text reloc count: 0x%04X (%d) entries\n", file_header.a_trsize, file_header.a_trsize);
	fprintf(f, " Data reloc count: 0x%04X (%d) entries\n\n", file_header.a_drsize, file_header.a_drsize);
}

//
//
//
void AoutFile::hexDumpGroup(FILE *f, uint8_t *buf)
{
	for (int item = 0; item < 4; item++)
	{
		fprintf(f, "%02X ", buf[item]);
	}
}

//
//
//
void AoutFile::hexDumpLine(FILE *f, uint32_t offset, uint8_t *buf)
{
	fprintf(f, "%04X: ", offset);

	hexDumpGroup(f, buf);
	fputc(' ', f);
	hexDumpGroup(f, buf+4);

	fputs("- ", f);

	hexDumpGroup(f, buf+8);
	fputc(' ', f);
	hexDumpGroup(f, buf+12);

	fputc('[', f);

	// write out character row
	for (int i = 0; i < 16; i++)
	{
		if (isprint(buf[i]))
			fputc(buf[i], f);
		else
			fputc('.', f);
	}
	
	fprintf(f, "]\n");
}

//
//
//
void AoutFile::hexDumpSegment(FILE *f, uint8_t *seg, size_t size)
{
	uint32_t offset = 0;

	for (size_t i = 0; i < size / 16; i++)
	{
		hexDumpLine(f, offset, &seg[offset]);
		offset += 16;
	}

	// TODO - we fail to print the last (partial) line!
	//for (size_t i = 0; i < size % 16; i++)
	//{

	//}
}

void AoutFile::dumpText(FILE *f)
{
	assert(f != nullptr);
	if (f == nullptr)
		return;

	fprintf(f, "Text segment (hex)\n");
	fprintf(f, "------------------\n\n");

	Segment::value_type *pData = text_segment.data();
	hexDumpSegment(f, pData, text_segment.size());
	fputc('\n', f);
}

void AoutFile::dumpData(FILE *f)
{
	assert(f != nullptr);
	if (f == nullptr)
		return;

	fprintf(f, "Data segment (hex)\n");
	fprintf(f, "------------------\n\n");

	Segment::value_type *pData = data_segment.data();
	hexDumpSegment(f, data_segment.data(), data_segment.size());
	fputc('\n', f);
}

void AoutFile::dumpTextRelocs(FILE *f)
{
	assert(f != nullptr);
	if (f == nullptr)
		return;

	fprintf(f, "Text segment relocations\n");
	fprintf(f, "------------------------\n\n");

	auto iter = textRelocs.begin();
	for (;iter != textRelocs.end(); iter++)
	{
		auto re = *iter;
		fprintf(f, "address: 0x%04X, external: %d, size: %d\n", re.address, re.external, 1 << re.length);
	}

	fputc('\n', f);
}

void AoutFile::dumpDataRelocs(FILE *f)
{
	assert(f != nullptr);
	if (f == nullptr)
		return;

	fprintf(f, "Data segment relocations\n");
	fprintf(f, "------------------------\n\n");

	auto iter = dataRelocs.begin();
	for (; iter != dataRelocs.end(); iter++)
	{
		auto re = *iter;
		fprintf(f, "address: 0x%04X, external: %d, size: %d\n", re.address, re.external, 1 << re.length);
	}

	fputc('\n', f);
}
