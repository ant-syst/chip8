#include "tools.h"
#include "chip8.h"
#include "disassemble.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define log(IT, FMT, ...) snprintf(buffer, DISASS_BUFFER_LEN, FMT, ##__VA_ARGS__)

#define I_00E0(IT)          log(IT, "clear screen")
#define I_00EE(IT)          log(IT, "return from a subroutine")
#define I_1NNN(IT, NNN)     log(IT, "jump %#x", NNN)
#define I_2NNN(IT, NNN)     log(IT, "call %#x", NNN)
#define I_3XKK(IT, X, KK)   log(IT, "skip if v%X == %d", X, KK)
#define I_4XKK(IT, X, KK)   log(IT, "skip if v%X != %d", X, KK)
#define I_5XY0(IT, X, Y)    log(IT, "skip if v%X == v%X", X, Y)
#define I_6XKK(IT, X, KK)   log(IT, "v%X <- %d", X, KK)
#define I_7XKK(IT, X, KK)   log(IT, "v%X <- v%X + %d", X, X, KK)
#define I_8XY0(IT, X, Y)    log(IT, "v%X <- v%X", X, Y)
#define I_8XY1(IT, X, Y)    log(IT, "v%X <- v%X or v%X", X, X, Y)
#define I_8XY2(IT, X, Y)    log(IT, "v%X <- v%X and v%X", X, X, Y)
#define I_8XY3(IT, X, Y)    log(IT, "v%X <- v%X xor v%X", X, X, Y)
#define I_8XY4(IT, X, Y)    log(IT, "v%X <- v%X + v%X", X, X, Y)
#define I_8XY5(IT, X, Y)    log(IT, "v%X <- v%X - v%X", X, X, Y)
#define I_8XY6(IT, X, Y)    log(IT, "v%X <- v%X >> 1", X, X)
#define I_8XY7(IT, X, Y)    log(IT, "v%X <- v%X - v%X", X, Y, X)
#define I_8XYE(IT, X, Y)    log(IT, "v%X <- v%X << 1", X, X)
#define I_9XY0(IT, X, Y)    log(IT, "skip if v%X != v%X", X, Y)
#define I_ANNN(IT, NNN)     log(IT, "I <- %#x", NNN)
#define I_BNNN(IT, NNN)     log(IT, "jump %#x + v0", NNN)
#define I_CXKK(IT, X, KK)   log(IT, "v%X <- rand() and %d", X, KK)
#define I_DXYN(IT, X, Y, N) log(IT, "sprite at v%X:v%X %d", X, Y, N)
#define I_EX9E(IT, X)       log(IT, "skip if key v%X is pressed", X)
#define I_EXA1(IT, X)       log(IT, "skip if key v%X is not pressed", X)
#define I_FX07(IT, X)       log(IT, "v%X <- timer", X)
#define I_FX0A(IT, X)       log(IT, "v%X <- wait key press", X)
#define I_FX15(IT, X)       log(IT, "timer <- v%X", X)
#define I_FX18(IT, X)       log(IT, "stimer <- v%X", X)
#define I_FX1E(IT, X)       log(IT, "I <- I + v%X", X)
#define I_FX29(IT, X)       log(IT, "I <- v%X", X)
#define I_FX33(IT, X)       log(IT, "bcd v%X", X)
#define I_FX55(IT, X)       log(IT, "store v0:v%X", X)
#define I_FX65(IT, X)       log(IT, "load v0:v%X", X)
#define DEFAULT(IT)         log(IT, "dw %#x ; %d", IT, IT)

void disassemble(uint16_t it, char buffer[DISASS_BUFFER_LEN])
{
    #include "opcodes.c"
}
