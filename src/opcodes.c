#include "opcodes.h"

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
