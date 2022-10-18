#define _CRT_SECURE_NO_WARNINGS

#include "../aout.h"
#include <stdio.h>
#include <stdarg.h>

enum {
	LOG_ALWAYS,
	LOG_VERBOSE,
	LOG_DEBUG,
};

//
// Command line switches
//
uint16_t g_bBaseAddr = 0;
char *g_szOutputFilename = "a.out";
bool g_bDebug = false;

//
// show usage
//
void usage()
{
	puts("usage: ln [options] filename\n");
	puts("-b 0000\tset base address");
	puts("-o file\tset output filename");
	puts("-v\tverbose output");

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
		if (args[i][1] == 'v')
			g_bDebug = true;

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
void log(int level, const char *fmt, ...)
{
	char buf[256];
	va_list argptr;

	va_start(argptr, fmt);
	vsprintf(buf, fmt, argptr);
	va_end(argptr);

	if (!level || (level && g_bDebug))
		fputs(buf, stdout);
}

//
int main(int argc, char* argv[])
{
	if (argc == 1)
		usage();

	int iFirstArg = getopt(argc, argv);
	
	std::vector<ObjectFile*> files;

	log(LOG_ALWAYS, "Linking...\n");

	// loop over and load the input files
	for (int i = iFirstArg; i < argc; i++)
	{
		log(LOG_ALWAYS, "%s\n", argv[i]);

		ObjectFile *pObj = new ObjectFile();
		pObj->readFile(argv[i]);

		files.push_back(pObj);
	}

	// create the linker meta object file
	ObjectFile *pObj = new ObjectFile();
	files.push_back(pObj);

	// create variable for top of stack
	SymbolEntity se;
	se.type = SET_DATA;
	se.value = pObj->addData(0);
	pObj->addData(0xE0);
	pObj->addSymbol("__brk", se);

	// create variable for end of bss segment / start of RAM
	se.value = pObj->allocBSS(1);
	se.type = SET_BSS;
	pObj->addSymbol("___ram_start", se);

	// possibly adjust base code address...
	files[0]->setTextBase(g_bBaseAddr);

	// compute code and data segment starts
	log(LOG_VERBOSE, "Compute data segment starts\n");
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

	// compute bss segment starts, start of bss is at the end of the last data segment
	size_t lastFile = files.size() - 1;
	uint32_t bssOffset = files[lastFile]->getDataBase() + files[lastFile]->getDataSize();
	files[0]->setBssBase(bssOffset);

	log(LOG_VERBOSE, "Compute bss segment starts\n");
	for (size_t i = 1; i < files.size(); i++)
	{
		uint32_t offset;

		// the BSS segments should stack at the end of the data segments
		offset = files[i - 1]->getBssBase() + files[i - 1]->getBssSize();
		files[i]->setBssBase(offset);
	}

	// for each module, do relocs and inter-segment fixups
	log(LOG_VERBOSE, "Relocating symbols and external reference fixups\n");
	for (size_t i = 0; i < files.size(); i++)
	{
		if (!files[i]->relocate(files))
		{
			log(LOG_ALWAYS, "Linking failed.\n");
			exit(-1);
		}
	}

	// merge the segments (AND the symbols!)
	log(LOG_VERBOSE, "Merging segments and symbols\n");
	for (size_t i = 1; i < files.size(); i++)
	{
		files[0]->concat(files[i]);
	}

	// TODO - set the entry point?
	log(LOG_VERBOSE, "Setting entry point to %04X\n", g_bBaseAddr);
	files[0]->setEntryPoint(g_bBaseAddr);

	// write the output file
	files[0]->writeFile(g_szOutputFilename);

	log(LOG_ALWAYS, "Linking complete -> %s\n", g_szOutputFilename);

	return 0;
}
