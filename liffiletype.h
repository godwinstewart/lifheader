#ifndef LIFFILETYPE_H
#define LIFFILETYPE_H

#define LIF_UNKNOWN 0

#include <inttypes.h>
#include <stddef.h>

const char* lifDescriptionFromID(uint16_t);
uint16_t lifIDFromType(char*);

#endif
