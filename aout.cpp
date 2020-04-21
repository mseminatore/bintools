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
	// write the string table

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
	// read the string table
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

void AoutFile::addSymbol(const std::string &name, const SymbolEntity &sym)
{
	symbolTable.insert(SymbolTable::value_type(name, sym));
}

uint32_t AoutFile::addString(const std::string &name)
{
	uint32_t offset = stringTable.size();
	return offset;
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
	fprintf(f, " Text reloc count: 0x%X (%d)\n", file_header.a_trsize, file_header.a_trsize);
	fprintf(f, " Data reloc count: 0x%X (%d)\n\n", file_header.a_drsize, file_header.a_drsize);
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
		fprintf(f, "address: 0x%04X, external: %d, size: %d\n", re.address, re.external, re.length);
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
		fprintf(f, "address: 0x%04X, external: %d, size: %d\n", re.address, re.external, re.length);
	}

	fputc('\n', f);
}
