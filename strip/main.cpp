#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include <stdio.h>

//
// Command line switches
//
bool g_bStripAll = false;
bool g_bStripRelocations = false;
const char *g_szOutputFilename = "a.out";

//
// show usage
//
void usage()
{
	puts("\nusage: strip [options] filename\n");
	puts("-a\tstrip all");
	puts("-r\tstrip relocations");
	puts("-o file\tset output filename\n");

	exit(0);
}

//
// get options from the command line
//
int getopt(int n, char *args[])
{
	int i;
	for (i = 1; args[i][0] == '-'; i++)
	{
		if (args[i][1] == 'a')
			g_bStripAll = true;

		if (args[i][1] == 'r')
			g_bStripRelocations = true;

		if (args[i][1] == 'o')
		{
			g_szOutputFilename = args[i + 1];
			i++;
		}
	}

	return i;
}

//
int main(int argc, char *argv[])
{
	if (argc == 1)
		usage();

	int iFirstArg = getopt(argc, argv);

	// read in the object file
	ObjectFile obj;
	obj.readFile(argv[iFirstArg]);

	obj.stripSymbols();

	if (g_bStripRelocations || g_bStripAll)
		obj.stripRelocations();

	// write out the object file
	obj.writeFile(g_szOutputFilename);

	return 0;
}