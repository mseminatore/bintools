#include "../aout.h"
#include <stdio.h>

//
bool g_bDumpText = false;
bool g_bDumpData = false;
bool g_bGenerateTestFile = false;
bool g_bDumpSymbols = false;
bool g_bDumpStrings = false;

//
// show usage
//
void usage()
{
	printf("usage: dumpbin [options] filename\n\n");
	printf("-t\tdump text segment\n");
	printf("-d\tdump data segment\n");
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

		if (args[i][1] == 'g')
			g_bGenerateTestFile = true;
	}

	return i;
}

//
//
//
void testGen()
{
	AoutFile a;

	// generate text
	for (int i = 0; i < 256; i++)
	{
		a.addText(i);
	}

	// generate data
	for (int i = 0; i < 256; i++)
	{
		a.addData(i);
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

	AoutFile a;

	FILE *fptr = nullptr;
	
	auto result = fopen_s(&fptr, argv[iFirstArg], "rb");

	a.readFile(fptr);

	// dump the header data
	a.dumpHeader(stdout);

	// optionally dump the text segment
	if (g_bDumpText)
		a.dumpText(stdout);

	// optionally dump the data segment
	if (g_bDumpData)
		a.dumpData(stdout);
}