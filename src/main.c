#include "tools.h"
#include "sdl_io.h"
#include "console_io.h"
#include "args.h"
#include "mapping.h"
#include "vm.h"

#include <stdlib.h>

int main(int argc, char ** argv)
{
    struct debugger * dbg = NULL;
    struct io * io = NULL;
    struct chip8 * chip = NULL;
    struct vm * vm = NULL;
    char const * rom = NULL, * io_name = NULL;
    int pix_size = 10;
    int enable_debug;

    if(parse_name(argc, argv, "--help") != 0)
        goto parse_error;

    enable_debug = parse_name(argc, argv, "--use-debug");

    rom = parse_str(argc, argv, "--rom", NULL);
    if(!rom)
        goto parse_error;

    if(!parse_choices(argc, argv, "--io", (char const*[]){"curses", "sdl", NULL}, "sdl", &io_name))
        goto parse_error;

    chip = chip8_alloc();
    if(!chip)
        THROW(error, 0, "chip8_alloc");

    if(strcmp(io_name, "sdl") == 0)
        io = sdl_alloc(N_COLS * pix_size, N_LINES * pix_size, &key_mapping, 10, 10);
    else
        io = nc_alloc(&key_mapping);

    if(!io)
        THROW(error, 0, "io alloc");

    dbg = dbg_alloc(chip, enable_debug);
    if(!dbg)
        THROW(error, 0, "dbg_alloc");

    if(!chip8_load_rom(chip, rom))
        THROW(error, 0, "chip8_load_rom");

    vm = vm_alloc();
    if(!vm)
        THROW(error, 0, "vm_alloc");

    if(!vm_run(vm, chip, io, dbg))
        THROW(error, 0, "run");

    io->free(&io);
    chip8_free(&chip);
    dbg_free(&dbg);
    vm_free(&vm);

    return EXIT_SUCCESS;

    parse_error:
        printf(
            "Usage: %s --help | --rom <rom file> --io <curses|sdl> "
            "[--use-debug]\n"
            "arguments:\n"
                "--rom             The rom path that must be loaded\n"
                "--io <curses|sdl> The user interface (SDL or ncurses in terminal)\n"
                "[--use-debug]     Run the emulator in debug mode waiting a \n"
                "                  debugger connection\n",
            argv[0]
        );

    error:

    if(io)
        io->free(&io);
    chip8_free(&chip);
    dbg_free(&dbg);
    vm_free(&vm);

    return EXIT_FAILURE;
}
