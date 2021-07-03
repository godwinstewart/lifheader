/* LIF Header manipulation
 *
 * G. Stewart - June 2021
 *
 * Based on Jean-François Garnier's "alifhdr"
 */
 
#ifndef LIFHEADER_H
#define LIFHEADER_H

typedef unsigned char byte;

#define HEADERLENGTH	32
#define BYTESPERSECTOR	256
#define FILENAMELENGTH	10

#ifdef __WIN32
#include <winsock.h>
#include <stdint.h>
#else
#include <arpa/inet.h>
#endif
#include <stdio.h>

/* Define the structure of the LIF header here */
typedef struct {
	char fileName[FILENAMELENGTH];
	uint16_t fileType;
	uint32_t startSector;
	uint32_t fileSize;
	byte timestamp[6];
	uint16_t volumeID;
	uint32_t generalPurpose;
} LIFHDR, *PLIFHDR;


/* Convert BCD hex to decimal... */
int BCD2int(int);

/* ... and back again */
int int2BCD(int);

/* Parse the command line */
void parseCommandLine(int, char**);

/* Create a new LIF header and initialise its fields */
PLIFHDR NewLIFHeader();

/* Show the contents of a file's LIF header */
void ShowLIFHeader(PLIFHDR);

/* Show the command line usage */
void ShowUsage();

/* Get the actual useful length of a file, not just the sectors used, from the header */
int GetRealFileLength(PLIFHDR);

/* Sets the real size of the data */
void SetLIFSize(PLIFHDR, uint32_t);

/* Many HP-71B files encode the data length in the "General Purpose" uint32 */
int HP71Length(PLIFHDR);

/* Allocate space for and read in a LIF header from a file */
PLIFHDR LoadLIF(FILE*);

/* Parse the LIF filename */
void ParseLIFName();

#endif
