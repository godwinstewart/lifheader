/* LIF Header manipulation
 *
 * G. Stewart - June 2021
 *
 * Based on Jean-François Garnier's "alifhdr"
 */

#include "lifheader.h"
#include "liffiletype.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <time.h>
#include <sys/stat.h>

/* Variables that define the behaviour of lifheader */
int showHow = 0;
int errorCode = 0;
int useToday = 0;
char* inputFile = NULL;
char* outputFile = NULL;
char* fileType = NULL;
char* action = NULL;
char* lifFileSpec = NULL;
byte sector[BYTESPERSECTOR];
char lifName[FILENAMELENGTH];

FILE *inStream;
FILE *outStream;

int main(int argc, char** argv) {
	
	PLIFHDR hdr;
	size_t r;
	
	parseCommandLine(argc, argv);
	if (errorCode) goto alldone;
	
	/* Was help asked for? */
	if (showHow) {
		ShowUsage();
		goto alldone;
	}
	
	/* whatever we're doing, we'll need an input file */
	
	/* If there is an input file and if it is "-"... */
	if (inputFile && !strcmp(inputFile, "-")) inputFile = NULL;
	
	if (inputFile) {
		if (!(inStream = fopen(inputFile, "rb"))) {
			fprintf(stderr, "ERROR: Could not open input file\n");
			errorCode = 3;
			goto alldone;
		}
	}
	else {
		inStream = stdin;
		useToday = 1;
	}
	
	/* Are we supposed to be displaying the header? */
	if (!strcasecmp(action, "show")) {
		if ((hdr = LoadLIF(inStream))) {
			ShowLIFHeader(hdr);
			free(hdr);
		}
		goto alldone;
	}
	
	/* We're either stripping or adding a LIF header. Either way we want an output file. */
	if (outputFile && !strcmp(outputFile, "-")) outputFile = NULL;
	if (outputFile) {
		if (!(outStream = fopen(outputFile, "wb"))) {
			fprintf(stderr, "ERROR: Could not open output file\n");
			errorCode = 5;
			goto alldone;
		}
	}
	else {
#ifdef __WIN32
		outStream = freopen(NULL, "wb", stdout);
		fprintf(stderr, "ERROR: No output file given\n");
		errorCode = 13;
		goto alldone;
#else
		outStream = stdout;
#endif
	}
	
	/* Stripping a header? Don't ask questions, just chop off the first 32 bytes. */
	if (!strcasecmp(action, "strip")) {
		/* Is the file at least 32 bytes long? Reading a header will tell us this. */
		if (!(hdr = LoadLIF(inStream))) goto alldone;
		
		/* Keep on sucking and spitting until there is no more to suck */
		r = 1;
		while (r > 0) {
			memset(sector, 0, BYTESPERSECTOR);
			r = fread(sector, sizeof(byte), BYTESPERSECTOR, inStream);
			if (r > 0) {
				fwrite(sector, sizeof(byte), r, outStream);
			}
		}
		goto alldone;
	}
	
	/* Must be an "add" command */
	if (!strcasecmp(action, "add")) {
		
		/* Initialise the LIF filename with spaces */
		memset(lifName, 0x20, FILENAMELENGTH);
		ParseLIFName();
		
		/* Bail out if there was an error parsing the LIF filename */
		if (errorCode) goto alldone;
		
		/* Do we have the file type to use? */
		if (!fileType) {
			fprintf(stderr, "ERROR: file type not given (-t option)\n");
			errorCode = 10;
			goto alldone;
		}
		
		/* Is it a recognised file type? */
		uint16_t lifID = lifIDFromType(fileType);
		if (!lifID) {
			fprintf(stderr, "ERROR: unknown LIF file type: %s\n", fileType);
			errorCode = 12;
			goto alldone;
		}
		
		/* Construct a LIF header with the information that we know so far */
		PLIFHDR hdr = NewLIFHeader();
		hdr->fileType = htons(lifID);
		memcpy(hdr->fileName, lifName, FILENAMELENGTH);
		
		/* Start reading the source file into memory and keep track of its length */
		uint32_t dataSize = 0;
		byte* inData;
		byte* newPtr;
		size_t bytesRead = 1;
		while (bytesRead > 0) {
			bytesRead = fread(sector, sizeof(byte), BYTESPERSECTOR, inStream);
			if (bytesRead >= 1) {
				
				if (dataSize) {
					newPtr = (byte*)realloc(inData, dataSize + BYTESPERSECTOR);
					if (newPtr)
						inData = newPtr;
					else {
						free(inData);
						inData = NULL;
					}
				}
				else
					inData = (byte*)malloc(BYTESPERSECTOR);
				
				if (!inData) {
					fprintf(stderr, "ERROR: Out of memory reading in source data.\n");
					errorCode = 13;
					goto alldone;
				}
				
				memcpy(inData + dataSize, sector, bytesRead);
				dataSize += bytesRead;
			}
		}
		
		/* Now that we have the length of the data we can construct the rest of the LIF header */
		int nbSectors = dataSize ? (((dataSize - 1) >> 8) + 1) : 0;
		hdr->fileSize = htonl(nbSectors);
		SetLIFSize(hdr, dataSize);
		
		/* Now for the timestamp. Are we using the current timestamp for this? */
		time_t useThisTime;
		struct stat statbuf;
		if (useToday) {
			useThisTime = time(NULL);
		}
		else {
			if (fstat(fileno(inStream), &statbuf)) {
				useThisTime = time(NULL);
			}
			else {
				useThisTime = statbuf.st_mtime;
			}
		}
		
		struct tm timestruct;
		localtime_r(&useThisTime, &timestruct);
		
		/* Fill in the blanks */
		hdr->timestamp[0] = int2BCD(timestruct.tm_year % 100);
		hdr->timestamp[1] = int2BCD(timestruct.tm_mon + 1);
		hdr->timestamp[2] = int2BCD(timestruct.tm_mday);
		hdr->timestamp[3] = int2BCD(timestruct.tm_hour);
		hdr->timestamp[4] = int2BCD(timestruct.tm_min);
		hdr->timestamp[5] = int2BCD(timestruct.tm_sec);
		
		/* We're done! Write the data */
		if (fwrite(hdr, sizeof(LIFHDR), 1, outStream) != 1)
		{
			fprintf(stderr, "ERROR: Unable to write to output.\n");
			errorCode = 11;
			free(hdr);
			free(inData);
			goto alldone;
		}
		
		if (fwrite(inData, sizeof(byte), dataSize, outStream) != dataSize) {
			fprintf(stderr, "ERROR: Unable to write to output.\n");
			errorCode = 11;
		}
		
		free(hdr);
		free(inData);
		
		goto alldone;
	}
	
	fprintf(stderr, "ERROR: Unknown action: %s\n", action);
	errorCode = 6;

alldone:
	return errorCode;
	
}


/* Read in a LIF header from a file */
PLIFHDR LoadLIF(FILE* inStream) {
	
	PLIFHDR hdr = NewLIFHeader();
	
	if (hdr) {
		if (fread(hdr, sizeof(LIFHDR), 1, inStream) != 1) {
			fprintf(stderr, "ERROR: Could not read from input\n");
			free(hdr);
			hdr = NULL;
			errorCode = 4;
		}
	}
	
	return hdr;
	
}

/* Parse the command line to find out what we have to do */
void parseCommandLine(int argc, char** argv) {
	
	int c; /* will be -1 when we run out of options */
	int l; /* lower case version of c */
	
	while ((c = getopt(argc, argv, "i:o:t:a:l:h")) != -1) {
		
		l = tolower(c);
		switch (l) {
			
			case 'i':
				inputFile = optarg;
				break;
			
			case 'o':
				outputFile = optarg;
				break;
			
			case 't':
				fileType = optarg;
				break;
			
			case 'a':
				action = optarg;
				break;
			
			case 'h':
				showHow = 1;
				return;
			
			case 'l':
				lifFileSpec = optarg;
				break;
			
			case '?':
				errorCode = 1;
				return;
			
		}
		
	}
	
	/* Check that the action was given */
	if (!action) {
		fprintf(stderr, "ERROR: No action given. Cannot continue.\n");
		errorCode = 2;
		return;
	}
	
}

/* Convert BCD hex to decimal */
int BCD2int(int bcd) {
	return ((bcd & 0xf0) >> 4) * 10 + (bcd & 0x0f);
}

/* Convert back from decimal to BCD */
int int2BCD(int dec) {
	return ((dec / 10) << 4) + (dec % 10);
}

/* Create a new LIF header and initialise its fields */
PLIFHDR NewLIFHeader() {
	
	PLIFHDR newHeader = (PLIFHDR)malloc(HEADERLENGTH);
	
	if (newHeader) {
		/* Zero out the structure */
		memset(newHeader, 0, HEADERLENGTH);
		
		/* Put spaces in the file name field */
		memset(newHeader->fileName, 0x20, FILENAMELENGTH);
		
		/* Set Volume to 0x8001 */
		newHeader->volumeID = htons(0x8001);
	}
	
	return newHeader;
	
}

/* Show the contents of a file's LIF header */
void ShowLIFHeader(PLIFHDR hdr) {
	
	const char* fileType;
	
	if (!hdr) return; /* Don't want to read NULL... */
	
	char name[FILENAMELENGTH+1];
	strncpy(name, hdr->fileName, FILENAMELENGTH);
	name[FILENAMELENGTH] = 0x00;
	uint32_t nSectors = ntohl(hdr->fileSize);
	uint32_t nBytes = nSectors * BYTESPERSECTOR;
	int usedBytes = GetRealFileLength(hdr);
	int yr = 1900 + BCD2int((int)hdr->timestamp[0]);
	if (yr < 1970) yr += 100;
	uint16_t lifType = ntohs(hdr->fileType);
	fileType = lifDescriptionFromID(lifType);
	
	printf("File name:    %s\n", name);
	printf("File type:    0x%04x (%s)\n", lifType, fileType ? fileType : "unknown");
	printf("Start sector: %u\n", (unsigned)ntohl(hdr->startSector));
	printf("File length:  %u sectors (%u bytes), %d bytes used\n", nSectors, nBytes, usedBytes);
	printf("Timestamp:    %d-%02d-%02d %02d:%02d:%02d\n",
		yr,
		BCD2int((int)hdr->timestamp[1]),
		BCD2int((int)hdr->timestamp[2]),
		BCD2int((int)hdr->timestamp[3]),
		BCD2int((int)hdr->timestamp[4]),
		BCD2int((int)hdr->timestamp[5])
	);
	printf("Volume ID:    0x%04x\n", (int)ntohs(hdr->volumeID));
	printf("Gen. Purpose: 0x%08x\n\n", (unsigned)ntohl(hdr->generalPurpose));
	
}

/* show command usage */
void ShowUsage() {
	printf("Usage:\n");
	printf("\tlifheader { -a action | -h } [ -i input_file ] [ -o output_file ] [ -t file_type ]\n");
	printf("\t          [ -l lif_file_name ]\n\n");
	printf("\t-h                Shows this help message.\n\n");
	printf("\t-a action         Specifies the action to undertake on the input file. Possible options are:\n");
	printf("\t\t-a strip        Strips the LIF header from the input file.\n");
	printf("\t\t-a add          Generates a LIF header, prepends it to the input file\n");
	printf("\t\t                and saves the result to the output file\n");
	printf("\t\t-a show         Shows the data in the LIF header.\n\n");
	printf("\t-i input_file     Designates the input file to read from. If not given\n");
	printf("\t                  or if the string `-' is given, then STDIN is used.\n\n");
#ifdef __WIN32
	printf("\t-o output_file    Designates the output file to write to (required for -a add and -a strip).\n\n");
#else
	printf("\t-o output_file    Designates the output file to write to. If not given\n");
	printf("\t                  or if the string `-' is given, then STDOUT is used.\n\n");
#endif
	printf("\t-t file_type      When adding a LIF header to a file, specifies the file type\n");
	printf("\t                  to indicate in the header. Possible options are:\n");
	printf("\t\t-t lex71        HP-71B LEX file (0xe208)\n");
	printf("\t\t-t bas71        HP-71B BASIC file (0xe214)\n");
	printf("\t\t-t rom71        HP-71B ROM file (0xe21c)\n");
	printf("\t\t-t key71        HP-71B key assignments (0xe20c)\n");
	printf("\t\t-t txt71        HP-71B text file (0x0001)\n");
	printf("\t\t-t bin71        HP-71B BIN file (0xe204)\n");
	printf("\t\t-t prg41        HP-41C program (0xe080)\n");
	printf("\t\t-t sdata        HP-71B SDATA/HP-41C data file (0xe0d0)\n");
	printf("\t\t-t key41        HP-41C key assignments (0xe050)\n");
	printf("\t\t-t sta41        HP-41C status file (0xe060)\n");
	printf("\t\t-t all41        HP-41C \"WALL\" file (0xe040)\n");
	printf("\t\t-t rom41        HP-41C ROM/MLDL dump (0xe070)\n\n");
	printf("\t-l lif_file_name  Provides the name for the file in the LIF image when adding a\n");
	printf("\t                  LIF header to a file. The name is deduced from the original\n");
	printf("\t                  filename if not given on the command line.\n\n");
}

/* Get the actual file length */
int GetRealFileLength(PLIFHDR hdr) {
	
	int nbRecords, recordLength;
	byte* ptr;
	
	/* This is going to depend on the file type */
	uint16_t fileType = ntohs(hdr->fileType);
	
	switch(fileType) {
		
		case 1:			/* text file */
		case 0xe0d1:	/* secure text file */
			return BYTESPERSECTOR * ntohl(hdr->fileSize);
		
		case 0x00ff:	/* disabled LEX file */
			return HP71Length(hdr);
		
		case 0xe0d0:	/* SDATA file or HP-41C data */
			return 8 * (ntohl(hdr->generalPurpose) >> 16);
		
		case 0xe0f0:	/* DATA file */
		case 0xe0f1:	/* secure DATA file */
			ptr = (byte*)&(hdr->generalPurpose);
			nbRecords = *ptr + *(ptr+1) * 256;
			recordLength = *(ptr+2) + *(ptr+3) * 256;
			return nbRecords * recordLength;
		
		case 0xe204:	/* BIN file */
		case 0xe205:	/* secure BIN file */
		case 0xe206:	/* private BIN file */
		case 0xe207:	/* private, secure BIN file */
		case 0xe208:	/* LEX file */
		case 0xe209:	/* secure LEX file */
		case 0xe20a:	/* private LEX file */
		case 0xe20b:	/* private, secure LEX file */
		case 0xe20c:	/* KEY file HP-71B */
		case 0xe20d:	/* secure KEY file */
		case 0xe214:	/* BASIC file HP-71B */
		case 0xe215:	/* secure BASIC file */
		case 0xe216:	/* private BASIC file */
		case 0xe217:	/* private, secure BASIC file */
			return HP71Length(hdr);
		
		case 0xe218:	/* FRAM file */
		case 0xe219:	/* secure FRAM file */
		case 0xe21a:	/* private FRAM file */
		case 0xe21b:	/* private, secure FRAM file */
			return BYTESPERSECTOR * ntohl(hdr->fileSize);
		
		case 0xe21c:	/* ROM file */
		case 0xe222:	/* Graphics file */
		case 0xe224:	/* Address file ?? */
		case 0xe22e:	/* Symbol file ?? */
			return HP71Length(hdr);
		
		case 0xe020:	/* WALL with X-Mem */
		case 0xe030:	/* WALL with X-Mem */
		case 0xe040:	/* WALL */
		case 0xe050:	/* KEYS file */
		case 0xe060:	/* STATUS file */
		case 0xe070:	/* HP-41C ROM/MLDL dump */
			ptr = (byte*)&(hdr->generalPurpose);
			return (*ptr * 256 + *(ptr+1)) * 8 + 1;
		
		case 0xe080:	/* HP-41C program */
			ptr = (byte*)&(hdr->generalPurpose);
			return *ptr * 256 + *(ptr+1) + 1;
		
		default:
			return -1;
		
	}
	
	return 0;
}

/* Sets the real size of the file */
void SetLIFSize(PLIFHDR hdr, uint32_t nbBytes) {
	
	uint16_t fileType = ntohs(hdr->fileType);
	uint32_t nybbles = nbBytes << 1;
	byte* ptr = (byte*)&(hdr->generalPurpose);
	
	hdr->generalPurpose = 0x0000;
	
	switch (fileType) {
		
		case 0xe204:	/* BIN71 file */
		case 0xe208:	/* LEX71 file */
		case 0xe20c:	/* KEY71 file */
		case 0xe214:	/* BAS71 file */
		case 0xe21c:	/* ROM71 file */
			*ptr = nybbles & 0xff;
			*(ptr+1) = (nybbles >> 8) & 0xff;
			*(ptr+2) = (nybbles >> 16) & 0xff;
			*(ptr+3) = (nybbles >> 24) & 0xff;
			break;
		
		case 0xe0d0:	/* HP-71B SDATA or HP-41C DATA */
			hdr->generalPurpose = htonl((nbBytes >> 3) << 13); /* divided by 8 and shifted 13 bits leftwards */
			break;
		
		case 0xe040:	/* WALL */
		case 0xe050:	/* KEYS file */
		case 0xe060:	/* STATUS file */
		case 0xe070:	/* HP-41C ROM/MLDL dump */
			hdr->generalPurpose = htonl(((nbBytes - 1) >> 3) << 16);
			break;
			
		case 0xe080:	/* HP-41C program */
			hdr->generalPurpose = htonl((nbBytes - 1) << 16);
			break;
		
	}
	
}

/* Many HP-71B files encode the data length in the "General Purpose" uint32 */
int HP71Length(PLIFHDR hdr) {
	byte* ptr = (byte*)&(hdr->generalPurpose);
	int nybbles = *ptr + *(ptr+1) * 256 + *(ptr+2) * 65536;
	return ++nybbles >> 1;
}

/* Parse the LIF filename given or use the filename of the input file */
void ParseLIFName() {
	
	int n, c;
	
	if (!lifFileSpec && !inputFile) {
		fprintf(stderr, "ERROR: Cannot deduce LIF file name from STDIN\n");
		errorCode = 7;
		return;
	}
	
	char* ptr;
	char* lastSlash;
	if (lifFileSpec)
		ptr = lifFileSpec;
	else
		ptr = inputFile;
	
	/* Where is the last slash in the filename? */
	lastSlash = strrchr(ptr, '/');
	if (lastSlash) {
		ptr = lastSlash + 1;
	}
	
	if (!*ptr) {
		fprintf(stderr, "ERROR: the LIF file name cannot be a null string\n");
		errorCode = 8;
		return;
	}
	
	for (n = 0; (n < 10) && *ptr; ++n) {
		c = toupper(*(ptr++));
		/* First character must be a letter */
		if (!n && ((c < 'A') || (c > 'Z'))) {
			fprintf(stderr, "ERROR: The first character of a LIF file name must be a letter 'A'-'Z'\n");
			errorCode = 9;
			return;
		}
		/* Letter, underscore or digit */
		if (((c >= 'A') && (c <= 'Z')) || ((c >= '0') && (c <= '9')) || (c =='_')) {
			lifName[n] = c;
		}
		/* Bail out at the first illegal char */
		else break;
	}
	
}
