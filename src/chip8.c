#include "tools.h"
#include "chip8.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define X(it)(it >> 8 & 0b1111)
#define Y(it)(it >> 4 & 0b1111)
#define KK(it)(it & 0b11111111)
#define NNN(it)(it & 0xFFF)
#define N(it)(it & 0xF)

static uint8_t const fonts[][5] = {
    {
        0b11110000,
        0b10010000,
        0b10010000,
        0b10010000,
        0b11110000,
    },
    {
        0b00100000,
        0b01100000,
        0b00100000,
        0b00100000,
        0b01110000,
    },
    {
        0b11110000,
        0b00010000,
        0b11110000,
        0b10000000,
        0b11110000,
    },
    {
        0b11110000,
        0b00010000,
        0b11110000,
        0b00010000,
        0b11110000,
    },
    {
        0b10010000,
        0b10010000,
        0b11110000,
        0b00010000,
        0b00010000,
    },
    {
        0b11110000,
        0b10000000,
        0b11110000,
        0b00010000,
        0b11110000,
    },
    {
        0b11110000,
        0b10000000,
        0b11110000,
        0b10010000,
        0b11110000,
    },
    {
        0b11110000,
        0b00010000,
        0b00100000,
        0b01000000,
        0b01000000,
    },
    {
        0b11110000,
        0b10010000,
        0b11110000,
        0b10010000,
        0b11110000,
    },
    {
        0b11110000,
        0b10010000,
        0b11110000,
        0b00010000,
        0b11110000,
    },
    {
        0b11110000,
        0b10010000,
        0b11110000,
        0b10010000,
        0b10010000,
    },
    {
        0b11100000,
        0b10010000,
        0b11100000,
        0b10010000,
        0b11100000,
    },
    {
        0b11110000,
        0b10000000,
        0b10000000,
        0b10000000,
        0b11110000,
    },
    {
        0b11100000,
        0b10010000,
        0b10010000,
        0b10010000,
        0b11100000,
    },
    {
        0b11110000,
        0b10000000,
        0b11110000,
        0b10000000,
        0b11110000,
    },
    {
        0b11110000,
        0b10000000,
        0b11110000,
        0b10000000,
        0b10000000
    }
};

static int get_keypress(struct chip8 * chip)
{
    int i;

    for(i=0; i<N_KEYS; i++)
        if(chip->keyboard[i])
            return i;

    return -1;
}

static int key_has_been_pressed(struct chip8 * chip, int key)
{
    return chip->keyboard[key];
}

static int stack_pop(struct chip8 * chip, uint16_t * address)
{
    if(chip->cpu.sp <= 0)
        return 0;

    chip->cpu.sp--;
    *address = chip->stack[chip->cpu.sp];

    return 1;
}

static int stack_push(struct chip8 * chip, uint16_t value)
{
    if(chip->cpu.sp >= STACK_SIZE)
        return 0;

    chip->stack[chip->cpu.sp] = value;
    chip->cpu.sp++;

    return 1;
}

void print_pixels(char pixels[N_LINES][N_COLS])
{
    for(int l=0; l<N_LINES; l++)
    {
        for(int c=0; c<N_COLS; c++)
        {
            printf("%c", pixels[l][c] ? ' ' : '0');
        }
        puts("");
    }
}

void chip8_free(struct chip8 ** chip)
{
    if(*chip)
    {
        free(*chip);
        *chip = NULL;
    }
}

struct chip8 * chip8_alloc(void)
{
    struct chip8 * chip = calloc(1, sizeof(struct chip8));
    if(!chip)
        THROW(error, 1, "malloc");

    memcpy(&chip->mem[FONTS_START], fonts, sizeof(fonts));
    chip->cpu.pc = MEM_START;

    return chip;

    error:

    chip8_free(&chip);

    return NULL;
}

int chip8_update_timers(struct chip8 * chip)
{
    if(chip->cpu.dt > 0 || chip->cpu.st > 0)
    {
        if(chip->cpu.dt > 0)
            chip->cpu.dt--;

        if(chip->cpu.st > 0)
            chip->cpu.st--;
    }

    return 1;

    error:

    return 0;
}

uint16_t chip8_decode_it(struct chip8 * chip)
{
    return __bswap_16(*((uint16_t*)&chip->mem[chip->cpu.pc]));
}

int chip8_load_rom(struct chip8 * chip, char const * path)
{
    FILE * program = fopen(path, "rb");
    if(program == NULL)
        THROW(error, 1, "fopen");

    fread(&chip->mem[MEM_START], 1, MEM_SIZE - MEM_START, program);
    if(ferror(program))
        THROW(error, 1, "fread");

    if(!feof(program))
        THROW(error, 0, "Program size too large");

    fclose(program);

    return 1;

    error:

    if(program != NULL)
        fclose(program);

    return 0;
}

int chip8_execute(struct chip8 * chip, uint16_t it)
{
    int inc_pc = 1;
    const unsigned int nnn = NNN(it);
    const unsigned int x = X(it);
    const unsigned int y = Y(it);
    const unsigned int kk = KK(it);

    switch(it & 0XF000)
    {
        case 0x0000:
            switch(it & 0xFF)
            {
                case 0xE0:
                    memset(chip->pixels, 0, N_COLS * N_LINES);
                break;

                case 0xEE:
                    if(!stack_pop(chip, &chip->cpu.pc))
                        THROW(error, 0, "stack_pop");
                break;

                default:
                    printf("Not managed %#x\n", it);
                break;
            }
        break;

        case 0x1000:
            chip->cpu.pc = nnn;
            inc_pc = 0;
        break;

        case 0x2000:
            if(!stack_push(chip, chip->cpu.pc))
                THROW(error, 0, "stack_push");

            chip->cpu.pc = nnn;
            inc_pc = 0;
        break;

        case 0x3000:
            if(chip->cpu.v[x] == kk)
                chip->cpu.pc += 2;
        break;

        case 0x4000:
            if(chip->cpu.v[x] != kk)
                chip->cpu.pc += 2;
        break;

        case 0x5000:
            if(chip->cpu.v[x] == chip->cpu.v[y])
                chip->cpu.pc += 2;
        break;

        case 0x6000:
            chip->cpu.v[x] = kk;
        break;

        case 0x7000:
            chip->cpu.v[x] += kk;
        break;

        case 0x8000:
        {
            uint8_t vx = chip->cpu.v[x];
            uint8_t vy = chip->cpu.v[y];

            switch(it & 0xF)
            {
                case 0x0:
                    chip->cpu.v[x] = vy;
                break;

                case 0x1:
                    chip->cpu.v[x] = vx | vy;
                break;

                case 0x2:
                    chip->cpu.v[x] = vx & vy;
                break;

                case 0x3:
                    chip->cpu.v[x] = vx ^ vy;
                break;

                case 0x4:
                {
                    uint16_t res = vx + vy;
                    chip->cpu.v[x] = (uint8_t)res;
                    chip->cpu.v[0XF] = res > 0xFF;
                }
                break;

                case 0x5:
                    chip->cpu.v[x] = vx - vy;
                    chip->cpu.v[0XF] = vx > vy;
                break;

                case 0x6:
                    chip->cpu.v[x] = vx >> 1;
                    chip->cpu.v[0XF] = vx & 0b1;
                break;

                case 0x7:
                    chip->cpu.v[x] = vy - vx;
                    chip->cpu.v[0XF] = vy > vx;
                break;

                case 0xE:
                    chip->cpu.v[x] = vx << 1;
                    chip->cpu.v[0XF] = vx >> 7 & 0b1;
                break;

                default:
                    printf("Not managed %#x\n", it);
                break;
            }
        }
        break;

        case 0x9000:
            if(chip->cpu.v[x] != chip->cpu.v[y])
                chip->cpu.pc += 2;
        break;

        case 0xA000:
            chip->cpu.i = nnn;
        break;

        case 0xB000:
            chip->cpu.pc = nnn + chip->cpu.v[0];
            inc_pc = 0;
        break;

        case 0xC000:
            chip->cpu.v[x] = rand() % 0xFF & kk;
        break;

        case 0xD000:
        {
            int row, col;
            int vx = chip->cpu.v[x];
            int vy = chip->cpu.v[y];
            int n = N(it);
            uint8_t * sprite = &chip->mem[chip->cpu.i];

            if(!chip8_check_mem_range(chip->cpu.i, n, READ))
                THROW(error, 0, "chip8_check_mem_range %d:%d",
                        chip->cpu.i, chip->cpu.i + n);

            chip->cpu.v[0xF] = 0;

            for(row=0; row<n; row++)
            {
                int y = (vy + row) % N_LINES;

                for(col=0; col<8; col++)
                {
                    int x = (vx + col) % N_COLS;

                    // TODO spec
                    uint8_t old = chip->pixels[y][x];
                    uint8_t new = old ^ ((sprite[row] >> (7-col)) & 0b1);

                    if(old && !new)
                        chip->cpu.v[0xF] = 0x1;

                    chip->pixels[y][x] = new;
                }
            }
        }
        break;

        case 0xE000:
            switch(it & 0xFF)
            {
                case 0x9E:
                    if(key_has_been_pressed(chip, chip->cpu.v[x]))
                        chip->cpu.pc += 2;
                break;

                case 0xA1:
                    if(!key_has_been_pressed(chip, chip->cpu.v[x]))
                        chip->cpu.pc += 2;
                break;

                default:
                    printf("Not managed %#x\n", it);
                break;
            }
        break;

        case 0xF000:
            switch(it & 0xFF)
            {
                case 0x07:
                    chip->cpu.v[x] = chip->cpu.dt;
                break;

                case 0x0A:
                {
                    int keypress = get_keypress(chip);
                    if(keypress >= 0)
                        chip->cpu.v[x] = keypress;
                    else
                        inc_pc = 0;
                }
                break;

                case 0x15:
                    chip->cpu.dt = chip->cpu.v[x];
                break;

                case 0x18:
                    chip->cpu.st = chip->cpu.v[x];
                break;

                case 0x1E:
                    chip->cpu.i += chip->cpu.v[x];
                break;

                case 0x29:
                    chip->cpu.i = FONTS_START + chip->cpu.v[x] * 5;
                break;

                case 0x33:
                {
                    uint8_t vx = chip->cpu.v[x];

                    if(!chip8_check_mem_range(chip->cpu.i, 2, WRITE))
                        goto error;

                    chip->mem[chip->cpu.i] = vx / 100 % 10;
                    chip->mem[chip->cpu.i + 1] = vx / 10 % 10;
                    chip->mem[chip->cpu.i + 2] = vx % 10;
                }
                break;

                case 0x55:
                {
                    uint8_t size = x + 1;

                    if(!chip8_check_mem_range(chip->cpu.i, size, WRITE))
                        THROW(error, 0, "chip8_check_mem_range");

                    memcpy(&chip->mem[chip->cpu.i], chip->cpu.v, size);
                }
                break;

                case 0x65:
                {
                    uint8_t size = x + 1;

                    if(!chip8_check_mem_range(chip->cpu.i, size, READ))
                        THROW(error, 0, "chip8_check_mem_range");

                    memcpy(chip->cpu.v, &chip->mem[chip->cpu.i], size);
                }
                break;
                default:
                    printf("Not managed %#x\n", it);
                break;
            }
        break;
        default:
            printf("Not managed %#x\n", it);
        break;
    }

    if(inc_pc)
        chip->cpu.pc += 2;

    // -1 because an instruction takes two bytes long
    if(!chip8_check_it_addr(chip->cpu.pc))
        THROW(error, 0, "wrong pc");

    return 1;

    error:

    return 0;
}
