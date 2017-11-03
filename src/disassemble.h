#ifndef H_DISASSEMBLE
#define H_DISASSEMBLE

#include <stdint.h>

#define DISASS_BUFFER_LEN 255

void disassemble(uint16_t it, char buffer[DISASS_BUFFER_LEN]);

#endif
