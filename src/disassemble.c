#include "tools.h"
#include "chip8.h"
#include "disassemble.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// TODO doublons
#define X(it)(it >> 8 & 0b1111)
#define Y(it)(it >> 4 & 0b1111)
#define KK(it)(it & 0b11111111)
#define NNN(it)(it & 0xFFF)
#define N(it)(it & 0xF)

#define log(IT, FMT, ...) snprintf(buffer, DISASS_BUFFER_LEN, FMT, ##__VA_ARGS__)

#define I_00E0(IT)          log(IT, "clear screen")
#define I_00EE(IT)          log(IT, "return from a subroutine")
#define I_1NNN(IT, ADDR)    log(IT, "jump %#x", ADDR)
#define I_2NNN(IT, ADDR)    log(IT, "call %#x", ADDR)
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
    switch(it & 0XF000)
    {
        case 0x0000:
            switch(it & 0xFF)
            {
                case 0xE0:
                    I_00E0(it);
                break;
                case 0xEE:
                    I_00EE(it);
                break;
                default:
                    DEFAULT(it);
                break;
            }
        break;
        case 0x1000:
            I_1NNN(it, NNN(it));
        break;
        case 0x2000:
            I_2NNN(it, NNN(it));
        break;
        case 0x3000:
            I_3XKK(it, X(it), KK(it));
        break;
        case 0x4000:
            I_4XKK(it, X(it), KK(it));
        break;
        case 0x5000:
            I_5XY0(it, X(it), Y(it));
        break;
        case 0x6000:
            I_6XKK(it, X(it), KK(it));
        break;
        case 0x7000:
            I_7XKK(it, X(it), KK(it));
        break;
        case 0x8000:
        {
            switch(it & 0xF)
            {
                case 0x0:
                    I_8XY0(it, X(it), Y(it));
                break;
                case 0x1:
                    I_8XY1(it, X(it), Y(it));
                break;
                case 0x2:
                    I_8XY2(it, X(it), Y(it));
                break;
                case 0x3:
                    I_8XY3(it, X(it), Y(it));
                break;
                case 0x4:
                    I_8XY4(it, X(it), Y(it));
                break;
                case 0x5:
                    I_8XY5(it, X(it), Y(it));
                break;
                case 0x6:
                    I_8XY6(it, X(it), Y(it));
                break;
                case 0x7:
                    I_8XY7(it, X(it), Y(it));
                break;
                case 0xE:
                    I_8XYE(it, X(it), Y(it));
                break;
                default:
                    DEFAULT(it);
                break;
            }
        }
        break;
        case 0x9000:
            I_9XY0(it, X(it), Y(it));
        break;
        case 0xA000:
            I_ANNN(it, NNN(it));
        break;
        case 0xB000:
            I_BNNN(it, NNN(it));
        break;
        case 0xC000:
            I_CXKK(it, X(it), KK(it));
        break;
        case 0xD000:
            I_DXYN(it, X(it), Y(it), N(it));
        break;
        case 0xE000:
            switch(it & 0xFF)
            {
                case 0x9E:
                    I_EX9E(it, X(it));
                break;
                case 0xA1:
                    I_EXA1(it, X(it));
                break;
                default:
                    DEFAULT(it);
                break;
            }
        break;
        case 0xF000:
            switch(it & 0xFF)
            {
                case 0x07:
                    I_FX07(it, X(it));
                break;
                case 0x0A:
                    I_FX0A(it, X(it));
                break;
                case 0x15:
                    I_FX15(it, X(it));
                break;
                case 0x18:
                    I_FX18(it, X(it));
                break;
                case 0x1E:
                    I_FX1E(it, X(it));
                break;
                case 0x29:
                    I_FX29(it, X(it));
                break;
                case 0x33:
                    I_FX33(it, X(it));
                break;
                case 0x55:
                    I_FX55(it, X(it));
                break;
                case 0x65:
                    I_FX65(it, X(it));
                break;
                default:
                    DEFAULT(it);
                break;
            }
        break;
        default:
            DEFAULT(it);
        break;
    }
}
