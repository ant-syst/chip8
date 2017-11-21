#ifndef H_CHIP_8
#define H_CHIP_8

#include "hw.h"

#include <stdint.h>
#include <sys/time.h>

struct cpu {
    uint8_t v[NUM_GPR];
    uint16_t i;
    uint16_t pc;
    uint8_t sp;
    uint8_t dt;
    uint8_t st;
};

struct chip8 {
    struct cpu cpu;
    uint8_t mem[MEM_SIZE];
    uint16_t stack[STACK_SIZE];
    char pixels[N_LINES][N_COLS];
    uint8_t keyboard[N_KEYS];
};

enum mem_access_type {
    READ,
    WRITE
};

void chip8_free(struct chip8 ** chip);

struct chip8 * chip8_alloc(void);

int chip8_load_rom(struct chip8 * chip, char const * path);

uint16_t chip8_decode_it(struct chip8 * chip);

int chip8_execute(struct chip8 * chip, uint16_t it);

int chip8_update_timers(struct chip8 * chip);

static inline int chip8_check_mem_range(uint16_t begin, uint16_t offset,
                                  enum mem_access_type access_type)
{
    if(access_type == WRITE)
        return (begin >= MEM_START) && (begin + offset < MEM_SIZE);
    else
        return (begin + offset < MEM_SIZE);
}

static inline int chip8_check_it_addr(uint16_t addr)
{
    return addr >= MEM_START && addr < (MEM_SIZE -1);
}

#endif
