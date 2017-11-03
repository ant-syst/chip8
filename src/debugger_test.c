#include "chip8.h"
#include "tools.h"
#include "debugger.h"

#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    struct debugger * dbg;
    struct chip8 * chip;

    chip = chip8_alloc();
    if(!chip)
        THROW2(error, 0, "chip8_alloc");

    dbg = dbg_alloc(chip);
    if(!dbg)
        THROW2(error, 0, "debugger_alloc");

    while(1)
    {
        for(unsigned int cpt = MEM_START; cpt<MEM_SIZE; cpt++)
        {
            dbg_call(dbg);

            chip->cpu.pc = cpt;

            usleep(10000);
        }
    }

    return EXIT_SUCCESS;

    error:

    return EXIT_FAILURE;
}
