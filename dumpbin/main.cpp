#include "../aout.h"
#include <stdio.h>

//
bool g_bDumpText = false;
bool g_bDumpData = false;
bool g_bDumpTextRelocs = false;
bool g_bDumpDataRelocs = false;
bool g_bGenerateTestFile = false;
bool g_bDumpSymbols = false;
bool g_bDumpStrings = false;

//
// show usage
//
void usage()
{
	puts("usage: dumpbin [options] filename\n");
	puts("-t\tdump text segment");
	puts("-d\tdump data segment");
	puts("-r\tdump relocations");
	puts("-s\tdump symbols");
	puts("-a\tdump all");
	exit(0);
}

//
// get options from the command line
//
int getopt(int n, char *args[])
{
	int i;
	for (i = 1; args[i] && args[i][0] == '-'; i++)
	{
		if (args[i][1] == 't')
			g_bDumpText = true;

		if (args[i][1] == 'd')
			g_bDumpData = true;

		if (args[i][1] == 'r')
		{
			g_bDumpTextRelocs = true;
			g_bDumpDataRelocs = true;
		}

		if (args[i][1] == 'g')
			g_bGenerateTestFile = true;

		if (args[i][1] == 's')
			g_bDumpSymbols = true;

		if (args[i][1] == 'a')
		{
			g_bDumpText = true;
			g_bDumpData = true;
			g_bDumpTextRelocs = true;
			g_bDumpDataRelocs = true;
			g_bDumpSymbols = true;
			g_bDumpStrings = true;
		}
	}

	return i;
}

//
void testGen()
{
	ObjectFile a;

	a.allocBSS(256);

	// generate text segment
	for (int i = 0; i < 256; i++)
	{
		a.addText(i);
	}

	// generate data segment
	for (int i = 0; i < 256; i++)
	{
		a.addData(i);
	}

	// generate text relocs
	for (int i = 0; i < 10; i++)
	{
		RelocationEntry re;
		
		re.address = 0xfefe;
		a.addTextRelocation(re);
	}

	// generate data relocs
	for (int i = 0; i < 10; i++)
	{
		RelocationEntry re;

		a.addDataRelocation(re);
	}

	FILE *f = nullptr;
	fopen_s(&f, "a.out", "wb");
		a.writeFile(f);
	fclose(f);
}

//
//
//
void main(int argc, char *argv[])
{
	if (argc == 1)
	{
		usage();
	}

	int iFirstArg = getopt(argc, argv);

	if (g_bGenerateTestFile)
	{
		testGen();
		exit(0);
	}

	ObjectFile a;

	a.readFile(argv[iFirstArg]);

	if (!a.isValid())
	{
		printf("Invalid file format!\n");
		exit(-1);
	}

	printf("Dump of file: %s\n\n", argv[iFirstArg]);

	// TODO - test for OBJ file vs. EXE
	if (a.getTextRelocSize() == 0 && a.getDataRelocSize() == 0)
		printf("File Type: %s\n\n", "EXECUTABLE FILE");
	else
		printf("File Type: %s\n\n", "OBJECT FILE");

	// dump the header data
	a.dumpHeader(stdout);

	// optionally dump the text segment
	if (g_bDumpText)
		a.dumpText(stdout);

	// optionally dump the data segment
	if (g_bDumpData)
		a.dumpData(stdout);

	if (g_bDumpTextRelocs)
		a.dumpTextRelocs(stdout);

	if (g_bDumpDataRelocs)
		a.dumpDataRelocs(stdout);

	if (g_bDumpSymbols)
		a.dumpSymbols(stdout);
}