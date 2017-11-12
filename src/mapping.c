#include "mapping.h"

/*
    Keypad                   Keyboard
    +-+-+-+-+                +-+-+-+-+
    |1|2|3|C|                |1|2|3|4|
    +-+-+-+-+                +-+-+-+-+
    |4|5|6|D|                |A|Z|E|R|
    +-+-+-+-+       =>       +-+-+-+-+
    |7|8|9|E|                |Q|S|D|F|
    +-+-+-+-+                +-+-+-+-+
    |A|0|B|F|                |W|X|C|V|
    +-+-+-+-+                +-+-+-+-+
*/
enum chip8_key key_mapping(int key)
{
    switch(key)
    {
        case '1':
        case '&':
            return KEY_1;
        case '2':
        case L'é':
            return KEY_2;
        case '3':
        case '"':
            return KEY_3;
        case '4':
        case '\'':
            return KEY_C;
        case 'a':
            return KEY_4;
        case 'z':
            return KEY_5;
        case 'e':
            return KEY_6;
        case 'r':
            return KEY_D;
        case 'q':
            return KEY_7;
        case 's':
            return KEY_8;
        case 'd':
            return KEY_9;
        case 'f':
            return KEY_E;
        case 'w':
            return KEY_A;
        case 'x':
            return KEY_0;
        case 'c':
            return KEY_B;
        case 'v':
            return KEY_F;
        default:
            return IO_KEY_NOT_FOUND;
    }
}
