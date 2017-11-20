#ifndef H_OPCODES
#define H_OPCODES

#define X(it)(it >> 8 & 0b1111)
#define Y(it)(it >> 4 & 0b1111)
#define KK(it)(it & 0b11111111)
#define NNN(it)(it & 0xFFF)
#define N(it)(it & 0xF)

#endif
