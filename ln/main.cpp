#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include <stdio.h>

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

		//if (args[i][1] == 'o')
		//	g_bDebug = true;
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

	// loop over and load the input files
	for (int i = iFirstArg; i < argc; i++)
	{
		AoutFile *pObj = new AoutFile();
		FILE *f = fopen(argv[i], "rb");
			pObj->readFile(f);
		fclose(f);

		files.push_back(pObj);
	}

	// compute segment starts
	for (int i = 1; i < files.size(); i++)
	{
		uint32_t offset;

		offset = files[i - 1]->getTextBase() + files[i-1]->getTextSize();
		files[i]->setTextBase(offset);

		offset = files[i - 1]->getDataBase() + files[i - 1]->getDataSize();
		files[i]->setDataBase(offset);

		offset = files[i - 1]->getBssBase() + files[i - 1]->getBssSize();
		files[i]->setBssBase(offset);
	}

	// collect all the symbols

	// do relocs

	// merge the segments

	// write the output file

	return 0;
}
