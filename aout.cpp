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

	// write the header
	auto result = fwrite(&file_header, sizeof(file_header), 1, fptr);

	// write the text segment
	fwrite(text_segment.data(), text_segment.size(), 1, fptr);

	// write the data segment
	fwrite(data_segment.data(), data_segment.size(), 1, fptr);

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

	// read the symbol table
	// read the string table
	return 0;
}

void AoutFile::addText(uint8_t item)
{
	text_segment.push_back(item);
}

void AoutFile::addData(uint8_t item)
{
	data_segment.push_back(item);
}

void AoutFile::addSymbol()
{

}

void AoutFile::addString()
{

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
