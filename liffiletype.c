#include "liffiletype.h"
#include <strings.h>

uint16_t fileTypes[] = {
	0xe204,		/* HP-71B BIN file */
	0xe205,		/* HP-71B BIN file, secure */
	0xe206,		/* HP-71B BIN file, private */
	0xe207,		/* HP-71B BIN file, private, secure */
	0xe208,		/* HP-71B LEX file */
	0xe209,		/* LEX file, secure */
	0xe20a,		/* LEX file, private */
	0xe20b,		/* LEX file, private, secure */
	0xe214,		/* HP-71B BASIC file */
	0xe215,		/* HP-71B BASIC file, secure */
	0xe216,		/* HP-71B BASIC file, private */
	0xe217,		/* HP-71B BASIC file, private, secure */
	0xe21c,		/* HP-71B ROM file */
	0xe20c,		/* HP-71B key assignments */
	0xe20d,		/* HP-71B key assignments, secure */
	0x0001,		/* HP-71B text file */
	0xe0d1,		/* HP-71B text file, secure */
	0xe080,		/* HP-41C program */
	0xe0d0,		/* Serial Data or HP-41C data file */
	0xe050,		/* HP-41C key assignments */
	0xe060,		/* HP-41C status file */
	0xe040,		/* HP-41C "WALL" file */
	0xe0f0,		/* HP-71B DATA file */
	0xe0f1,		/* HP-71B DATA file, secure */
	0xe218,		/* HP-71B FRAM file */
	0xe219,		/* HP-71B FRAM file, secure */
	0xe21a,		/* HP-71B FRAM file, private */
	0xe21b,		/* HP-71B FRAM file, private, secure */
	0xe222,		/* HP-71B graphics file */
	0xe070,		/* HP-41C ROM/MLDL dump */
	
	0x0000		/* end-of-list marker */
};

const char* lifOptions[] = {
	"bin71",
	"bin71",
	"bin71",
	"bin71",
	"lex71",
	"lex71",
	"lex71",
	"lex71",
	"bas71",
	"bas71",
	"bas71",
	"bas71",
	"rom71",
	"key71",
	"key71",
	"txt71",
	"txt71",
	"prg41",
	"sdata",
	"key41",
	"sta41",
	"all41",
	"",
	"",
	"frm71",
	"frm71",
	"frm71",
	"frm71",
	"gra71",
	"rom41"
};

const char* lifDescriptions[] = {
	"HP-71B BIN file",
	"HP-71B BIN file, secure",
	"HP-71B BIN file, private",
	"HP-71B BIN file, secure, private",
	"HP-71B LEX file",
	"HP-71B LEX file, secure",
	"HP-71B LEX file, private",
	"HP-71B LEX file, secure, private",
	"HP-71B BASIC file",
	"HP-71B BASIC file, secure",
	"HP-71B BASIC file, private",
	"HP-71B BASIC file, secure, private",
	"HP-71B ROM file",
	"HP-71B key assignments",
	"HP-71B key assignments, secure",
	"HP-71B text file",
	"HP-71B text file, secure",
	"HP-41C program",
	"HP-71B SDATA/HP-41C data file",
	"HP-41C key assignments",
	"HP-41C status file",
	"HP-41C \"WALL\" file",
	"HP-71B DATA file",
	"HP-71B DATA file, secure"
	"HP-71B FRAM file",
	"HP-71B FRAM file, secure",
	"HP-71B FRAM file, private",
	"HP-71B FRAM file, secure, private",
	"HP-71B graphics file",
	"HP-41C ROM/MLDL dump"
};


const char* lifDescriptionFromID(uint16_t id) {

	int index = 0;

	while (fileTypes[index]) {
		if (id == fileTypes[index]) {
			return lifDescriptions[index];
		}
		++index;
	}

	return NULL;

}


uint16_t lifIDFromType(char* fileType) {
	
	int index = 0;
	
	while (fileTypes[index]) {
		if (!strcasecmp(fileType,lifOptions[index])) break;
		++index;
	}
	
	return fileTypes[index];
	
}
