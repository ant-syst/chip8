#include "chip8.h"
#include "tools.h"
#include "sdl_io.h"
#include "debugger.h"
#include "args.h"
#include "mapping.h"

#include <stdlib.h>
#include <time.h>

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
    int enable_debug;

    enable_debug = parse_name(argc, argv, "--use-debug");

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

    dbg = dbg_alloc(chip, enable_debug);
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
