#include "vm.h"
#include "tools.h"
#include "timespec.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define CPU_HZ 240
#define TIMER_HZ 60

#define CPU_DELAY_NS FREQUENCY_TO_NS(CPU_HZ)
#define TIMER_DELAY_NS FREQUENCY_TO_NS(TIMER_HZ)
#define TIMER_FACTOR (CPU_HZ/TIMER_HZ)

struct clock {
    struct timespec sleep_date;
    unsigned int timer_num;
};

struct vm {
    struct io * io;
    struct chip8 * chip;
    struct clock clk;
};

static inline int clock_init(struct clock * clk)
{
    return clock_gettime(CLOCK_MONOTONIC, &clk->sleep_date) != -1;
}

static inline int clock_update(struct clock * clk, struct chip8 * chip)
{
    // timer
    if(clk->timer_num == TIMER_FACTOR-1)
    {
        if(!chip8_update_timers(chip))
            THROW(error, 0, "update_timers");

        clk->timer_num = 0;
    }
    else
        clk->timer_num++;

    // next sleep
    timespec_add_ns(&clk->sleep_date, CPU_DELAY_NS);

    // Sleep
    int res = 0;
    if((res=clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &clk->sleep_date, NULL)) != 0)
        THROW(error, 1, "clock_nanosleep");

    return 1;

    error:

    return 0;
}

void vm_free(struct vm ** vm)
{
    if(*vm)
        free(*vm);
    *vm = NULL;
}

struct vm * vm_alloc(void)
{
    struct vm * vm = calloc(1, sizeof(struct vm));
    if(!vm)
        THROW(error, 1, "calloc");

    return vm;

    error:

    vm_free(&vm);

    return NULL;
}

int vm_run(struct vm * vm, struct chip8 * chip, struct io * io, struct debugger * dbg)
{
    if(!clock_init(&vm->clk))
        THROW(error, 0, "timer_init");

    while(1)
    {
        uint16_t it = chip8_decode_it(chip);

        dbg_call(dbg);

        if(!chip8_execute(chip, it))
            THROW(error, 0, "decode");

        if(!io->update(io, chip->pixels))
            THROW(error, 0, "io_update");

        if(io->input_poll(io, chip->keyboard) == CHIP8_QUIT)
            break;

        if(!clock_update(&vm->clk, chip))
            THROW(error, 0, "timer_update");
    }

    return 1;

    error:

    return 0;
}
