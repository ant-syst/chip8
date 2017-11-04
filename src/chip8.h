#ifndef H_CHIP_8
#define H_CHIP_8

#include <stdint.h>
#include <sys/time.h>

#define NUM_GPR 16
#define FONTS_START 0x0

#define MEM_SIZE 4096
#define MEM_START 0x200
#define STACK_SIZE 16

#define N_KEYS 16
#define N_LINES 32
#define N_COLS 64

enum chip8_key {
    KEY_0, KEY_1, KEY_2, KEY_3,
    KEY_4, KEY_5, KEY_6, KEY_7,
    KEY_8, KEY_9, KEY_A, KEY_B,
    KEY_C, KEY_D, KEY_E, KEY_F,
    IO_KEY_NOT_FOUND = -1
};

enum chip8_inputs {
    CHIP8_QUIT,
    CHIP8_CONT
};

typedef enum chip8_key (keys_mapping)(int key);

struct io {
    void * opaque;
    keys_mapping * km;
    int (*update)(struct io * io_ptr, char pixels[N_LINES][N_COLS]);
    enum chip8_inputs (*input_poll)(struct io * io_ptr, uint8_t keys[N_KEYS]);
    void (*free)(struct io ** io_ptr);
};

// TODO TAD
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
    struct timespec begin, end;
};

void chip8_free(struct chip8 ** chip);

struct chip8 * chip8_alloc(void);

int chip8_load_rom(struct chip8 * chip, char const * path);

uint16_t chip8_decode_it(struct chip8 * chip);

int chip8_execute(struct chip8 * chip, uint16_t it);

int chip8_update_timers(struct chip8 * chip);

int check_it_addr(uint16_t addr);

#endif
