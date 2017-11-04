#include "chip8.h"
#include "tools.h"
#include "sdl_io.h"
#include "debugger.h"

#include <stdlib.h>
#include <time.h>

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
static enum chip8_key key_mapping(int key)
{    
    switch(key)
    {
        case '1':
            return KEY_1;
        case '2':
            return KEY_2;
        case '3':
            return KEY_3;
        case '4':
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

static int parse_name(int argc, char ** argv, char const *name)
{
    int i;
    
    for(i=1; i<argc; i++)
        if(strcmp(name, argv[i]) == 0)
            return 0;
    
    return 1;
}

static char const * parse_str(int argc, char ** argv, char const *name, char const *def)
{
    int i;
    
    for(i=1; i<argc-1; i++)
        if(strcmp(name, argv[i]) == 0)
            return argv[i + 1];
    
    return def;
}

static int run(struct chip8 * chip, struct io * io_ptr, struct debugger * dbg)
{
    while(1)
    {
        uint16_t it = chip8_decode_it(chip);
        
        if(!chip8_execute(chip, it))
            THROW("decode", error, 0);

        if(!chip8_update_timers(chip))
            THROW("update_timers", error, 0);

        if(!io_ptr->update(io_ptr, chip->pixels))
            THROW("update", error, 0);
        
        if(io_ptr->input_poll(io_ptr, chip->keyboard) == CHIP8_QUIT)
            break;
            
        //dbg_call(dbg);
    }
    
    return 1;
    
    error:
    
    return 0;
}

int main(int argc, char ** argv)
{
    struct debugger * dbg = NULL;
    struct io * io_ptr = NULL;
    struct chip8 * chip = NULL;
    char const * rom = NULL;
    int pix_size = 10;

    rom = parse_str(argc, argv, "--rom", NULL);
    if(!rom)
    {
        printf("Usage: %s --rom <rom file>\n", argv[0]);
        goto error;
    }

    chip = chip8_alloc();
    if(!chip)
        THROW("chip8_alloc", error, 0);

    io_ptr = sdl_alloc(N_COLS * pix_size, N_LINES * pix_size, &key_mapping, 10, 10);
    if(!io_ptr)
        THROW("sdl_alloc", error, 0);

    dbg = dbg_alloc(chip);
    if(!dbg)
        THROW("dbg_alloc", error, 0);

    if(!chip8_load_rom(chip, rom))
        THROW("chip8_load_rom", error, 0);

    if(!run(chip, io_ptr, dbg))
        THROW("run", error, 0);

    io_ptr->free(&io_ptr);
    chip8_free(&chip);
    dbg_free(&dbg);

    return EXIT_SUCCESS;

    error:

    if(io_ptr)
        io_ptr->free(&io_ptr);
    chip8_free(&chip);
    dbg_free(&dbg);

    return EXIT_FAILURE;
}
