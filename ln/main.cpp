#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include <stdio.h>

//
// Command line switches
//
uint16_t g_bBaseAddr = 0;
char *g_szOutputFilename = "a.out";


//
// show usage
//
void usage()
{
	printf("usage: ln [options] filename\n");
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
		//if (args[i][1] == 'v')
		//	g_bDebug = true;

		if (args[i][1] == 'o')
		{
			g_szOutputFilename = args[i + 1];
			i++;
		}

		if (args[i][1] == 'b')
		{
			g_bBaseAddr = (uint16_t)atol(args[i + 1]);
			i++;
		}
	}

	return i;
}

//
//
//
int main(int argc, char* argv[])
{
	if (argc == 1)
		usage();

	int iFirstArg = getopt(argc, argv);
	
	std::vector<AoutFile*> files;

	fprintf(stdout, "Linking...\n");

	// loop over and load the input files
	for (int i = iFirstArg; i < argc; i++)
	{
		fprintf(stdout, "%s\n", argv[i]);

		AoutFile *pObj = new AoutFile();
		FILE *f = fopen(argv[i], "rb");
			pObj->readFile(f);
		fclose(f);

		files.push_back(pObj);
	}
	
	// possibly adjust base address...
	files[0]->setTextBase(g_bBaseAddr);

	// compute segment starts
	for (size_t i = 1; i < files.size(); i++)
	{
		uint32_t offset;

		offset = files[i - 1]->getTextBase() + files[i-1]->getTextSize();
		files[i]->setTextBase(offset);

		offset = files[i - 1]->getDataBase() + files[i - 1]->getDataSize();
		files[i]->setDataBase(offset);

		// TODO - the BSS segments should stack at the end of the data segments!!
		offset = files[i - 1]->getBssBase() + files[i - 1]->getBssSize();
		files[i]->setBssBase(offset);
	}

	// for each module
	// do relocs and inter-segment fixups
	for (size_t i = 0; i < files.size(); i++)
	{
		files[i]->relocate(files);
	}

	// merge the segments (AND the symbols!)
	for (size_t i = 1; i < files.size(); i++)
	{
		files[0]->concat(files[i]);
	}

	// TODO - set the entry point?

	// write the output file
	files[0]->writeFile(g_szOutputFilename);

	fprintf(stdout, "Linking complete -> %s\b", g_szOutputFilename);

	return 0;
}
