#include "tools.h"
#include "chip8.h"
#include "term_colors.h"

#include <stdlib.h>
#include <stdio.h>
#include <execinfo.h>

#define TEST_NAME(NAME) test ## _ ## NAME
#define INIT_TEST_NAME(NAME) __init_test ## _ ## NAME

#define REGISTER_TEST(NAME)                                             \
                                                                        \
static int TEST_NAME(NAME)(struct chip8 * chip);                        \
                                                                        \
__attribute__((constructor)) static void INIT_TEST_NAME(NAME)(void) {   \
    static struct test_fn f;                                            \
                                                                        \
    f.call = &TEST_NAME(NAME);                                          \
    f.name = XSTR(TEST_NAME(NAME));                                     \
                                                                        \
    *tail = &f;                                                         \
    tail = &f.next;                                                     \
}                                                                       \
                                                                        \
static int TEST_NAME(NAME)(struct chip8 * chip)                         \

#define NUM_COLS(STATIC_MATRIX) (sizeof((STATIC_MATRIX)[0]) / sizeof((STATIC_MATRIX)[0][0]))
#define NUM_ROWS(STATIC_MATRIX) (sizeof((STATIC_MATRIX)) / sizeof((STATIC_MATRIX)[0][0]) / NUM_COLS(STATIC_MATRIX))

struct test_fn {
    char const * name;
    int (*call)(struct chip8 * chip);
    struct test_fn * next;
};

static struct test_fn * head, ** tail = &head;

#include "chip8.c"

// TODO test de overflow

static int cmp_matrix(char const * m1, char const * m2, int num_rows, int num_cols)
{
    int row, col;

    for(row=0; row<num_rows; row++)
        for(col=0; col<num_cols; col++)
            if(m1[row * num_cols + col] != m2[row * num_cols + col])
                return 0;

    return 1;
}

static int copy_matrix(char * pixels, int pix_num_rows, int pix_num_cols,
                        char * matrix, int m_x, int m_y,
                        int m_num_rows, int m_num_cols)
{
    int row;
    int col;

    for(row=0; row<m_num_rows; row++)
    {
        int curr_y = row + m_y;
        if(curr_y > pix_num_rows)
            THROW2(error, 0, "Wrong matrix size");

        for(col=0; col<m_num_cols; col++)
        {
            int curr_x = col + m_x;
            if(curr_x > pix_num_cols)
                THROW2(error, 0, "Wrong matrix size");

            pixels[curr_y * pix_num_cols + curr_x] = matrix[row * m_num_cols + col];
        }
    }

    return 1;

    error:

    return 0;
}

REGISTER_TEST(1nnn)
{
    if(!chip8_execute(chip, 0x2FF0))
        return 0;

    return chip->cpu.pc == 0xFF0;
}

REGISTER_TEST(2nnn)
{
    chip->cpu.pc = 0x10;

    if(!chip8_execute(chip, 0x2FF0))
        return 0;

    return chip->cpu.sp == 1 && chip->stack[0] == 0x10 && chip->cpu.pc == 0xFF0;
}

REGISTER_TEST(3xkk_eq)
{
    chip->cpu.v[0xa] = 0xFF;

    if(!chip8_execute(chip, 0x3aFF))
        return 0;

    return chip->cpu.pc == MEM_START + 4;
}

REGISTER_TEST(3xkk_not_eq)
{
    chip->cpu.v[0xa] = 0xFF;

    if(!chip8_execute(chip, 0x3aF0))
        return 0;

    return chip->cpu.pc == MEM_START + 2;
}

REGISTER_TEST(4xkk_eq)
{
    chip->cpu.v[0xa] = 0xFF;

    if(!chip8_execute(chip, 0x4aFF))
        return 0;

    return chip->cpu.pc == MEM_START + 2;
}

REGISTER_TEST(4xkk_not_eq)
{
    chip->cpu.v[0xa] = 0xFF;

    if(!chip8_execute(chip, 0x4aF0))
        return 0;

    return chip->cpu.pc == MEM_START + 4;
}

REGISTER_TEST(5xy0_eq)
{
    chip->cpu.v[0xa] = 128;
    chip->cpu.v[0xb] = 128;

    if(!chip8_execute(chip, 0x5ab0))
        return 0;

    return chip->cpu.pc == MEM_START + 4;
}

REGISTER_TEST(5xy0_not_eq)
{
    chip->cpu.v[0xa] = 128;
    chip->cpu.v[0xb] = 127;

    if(!chip8_execute(chip, 0x5ab0))
        return 0;

    return chip->cpu.pc == MEM_START + 2;
}

REGISTER_TEST(6xkk)
{
    if(!chip8_execute(chip, 0x6a20))
        return 0;

    return chip->cpu.v[0xa] == 0x20;
}

REGISTER_TEST(7xkk)
{
    chip->cpu.v[0xa] = 128;

    if(!chip8_execute(chip, 0x7a20))
        return 0;

    return chip->cpu.v[0xa] == 0x20 + 128;
}

REGISTER_TEST(8xy0)
{
    chip->cpu.v[0xb] = 128;

    if(!chip8_execute(chip, 0x8ab0))
        return 0;

    return chip->cpu.v[0xa] == 128;
}

REGISTER_TEST(8xy1)
{
    chip->cpu.v[0xa] = 0xFC;
    chip->cpu.v[0xb] = 0x3F;

    if(!chip8_execute(chip, 0x8ab1))
        return 0;

    return chip->cpu.v[0xa] == 0xFF;
}

REGISTER_TEST(8xy2)
{
    chip->cpu.v[0xa] = 0xFC;
    chip->cpu.v[0xb] = 0x3F;

    if(!chip8_execute(chip, 0x8ab2))
        return 0;

    return chip->cpu.v[0xa] == 0x3C;
}

REGISTER_TEST(8xy3)
{
    chip->cpu.v[0xa] = 0xFC;
    chip->cpu.v[0xb] = 0x3F;

    if(!chip8_execute(chip, 0x8ab3))
        return 0;

    return chip->cpu.v[0xa] == 0xC3;
}

REGISTER_TEST(8xy4_wo_carry)
{
    chip->cpu.v[0xa] = 0x10;
    chip->cpu.v[0xb] = 0x20;
    chip->cpu.v[0xF] = 1;

    if(!chip8_execute(chip, 0x8ab4))
        return 0;

    return chip->cpu.v[0xa] == 0x30 && chip->cpu.v[0xF] == 0;
}

REGISTER_TEST(8xy4_w_carry)
{
    chip->cpu.v[0xa] = 0xFF;
    chip->cpu.v[0xb] = 0x1;
    chip->cpu.v[0xF] = 0;

    if(!chip8_execute(chip, 0x8ab4))
        return 0;

    return chip->cpu.v[0xa] == 0x0 && chip->cpu.v[0xF] == 1;
}

REGISTER_TEST(8xy5_wo_borrow)
{
    chip->cpu.v[0xa] = 0x20;
    chip->cpu.v[0xb] = 0x10;
    chip->cpu.v[0xF] = 0;

    if(!chip8_execute(chip, 0x8ab5))
        return 0;

    return chip->cpu.v[0xa] == 0x10 && chip->cpu.v[0xF] == 1;
}

REGISTER_TEST(8xy5_w_borrow)
{
    chip->cpu.v[0xa] = 0x10;
    chip->cpu.v[0xb] = 0x20;
    chip->cpu.v[0xF] = 1;

    if(!chip8_execute(chip, 0x8ab5))
        return 0;

    return chip->cpu.v[0xa] == 0xF0 && chip->cpu.v[0xF] == 0;
}

REGISTER_TEST(8xy6_wo)
{
    chip->cpu.v[0xa] = 0x4;
    chip->cpu.v[0xF] = 1;

    if(!chip8_execute(chip, 0x8ab6))
        return 0;

    return chip->cpu.v[0xa] == 0x2 && chip->cpu.v[0xF] == 0;
}

REGISTER_TEST(8xy6_w)
{
    chip->cpu.v[0xa] = 0x1;
    chip->cpu.v[0xF] = 0;

    if(!chip8_execute(chip, 0x8ab6))
        return 0;

    return chip->cpu.v[0xa] == 0x0 && chip->cpu.v[0xF] == 1;
}

REGISTER_TEST(8xy7_wo_borrow)
{
    chip->cpu.v[0xa] = 0x10;
    chip->cpu.v[0xb] = 0x20;
    chip->cpu.v[0xF] = 0;

    if(!chip8_execute(chip, 0x8ab7))
        return 0;

    return chip->cpu.v[0xa] == 0x10 && chip->cpu.v[0xF] == 1;
}

REGISTER_TEST(8xy7_w_borrow)
{
    chip->cpu.v[0xa] = 0x20;
    chip->cpu.v[0xb] = 0x10;
    chip->cpu.v[0xF] = 1;

    if(!chip8_execute(chip, 0x8ab7))
        return 0;

    return chip->cpu.v[0xa] == 0xF0 && chip->cpu.v[0xF] == 0;
}

REGISTER_TEST(8xyE_wo)
{
    chip->cpu.v[0xa] = 0x2;
    chip->cpu.v[0xF] = 1;

    if(!chip8_execute(chip, 0x8abE))
        return 0;

    return chip->cpu.v[0xa] == 0x4 && chip->cpu.v[0xF] == 0;
}

REGISTER_TEST(8xyE_w)
{
    chip->cpu.v[0xa] = 0xF0;
    chip->cpu.v[0xF] = 0;

    if(!chip8_execute(chip, 0x8abE))
        return 0;

    return chip->cpu.v[0xa] == 0xE0 && chip->cpu.v[0xF] == 1;
}

REGISTER_TEST(9xy0_1)
{
    chip->cpu.v[0xa] = 0x3;
    chip->cpu.v[0xF] = 0;

    if(!chip8_execute(chip, 0x9ab0))
        return 0;

    return chip->cpu.pc == MEM_START + 4;
}

REGISTER_TEST(Annn)
{
    if(!chip8_execute(chip, 0xa004))
        return 0;

    return chip->cpu.i == 0x4;
}

REGISTER_TEST(Bnnn)
{
    chip->cpu.v[0] = 0x4;

    if(!chip8_execute(chip, 0xB202))
        return 0;

    return chip->cpu.pc == 0x202 + 0x4;
}

REGISTER_TEST(Cxkk)
{
    UNUSED(chip);

    /*
    unsigned int i;
    uint8_t values[255];

    for(i=0; i<255*100; i++)
    {
        if(!chip8_execute(chip, 0xCaFF))
            return 0;
        values[chip->cpu.v[0xa]]++;
    }

    for(i=0; i<sizeof(values); i++)
        printf("%d ", values[i]);

    puts("");
    */

    return -1;
}

/* Test draw with overlap without unset pixels */
REGISTER_TEST(Dxyn_1)
{
    int x, y;
    char pixels[N_LINES][N_COLS] = {0};

    char sprite_init[][8] = {
        {1, 0, 0, 1, 1, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0},
    };

    char sprite_res[][8] = {
        {1, 1, 0, 1, 1, 0, 1, 1},
        {1, 1, 1, 1, 1, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0},
    };

    x = 5;
    y = 7;

    if(!copy_matrix(&chip->pixels[0][0], N_LINES, N_COLS,
                     &sprite_init[0][0], x, y,
                     NUM_ROWS(sprite_init), NUM_COLS(sprite_init)))
        THROW2(error, 0, "copy_matrix");

    if(!copy_matrix(&pixels[0][0], N_LINES, N_COLS,
                     &sprite_res[0][0], x, y-1,
                     NUM_ROWS(sprite_res), NUM_COLS(sprite_res)))
        THROW2(error, 0, "copy_matrix");

    y = 6;

    chip->cpu.i = MEM_START;
    chip->mem[chip->cpu.i] = 0b11011011;
    chip->mem[chip->cpu.i + 1] = 0b01100000;
    chip->cpu.v[0xa] = x;
    chip->cpu.v[0xb] = y;

    if(!chip8_execute(chip, 0xDab2))
        return 0;

    if(chip->cpu.v[0xf] != 0)
        THROW2(error, 0, "chip->cpu.v[0xf] != 0");

    if(!cmp_matrix(&chip->pixels[0][0], &pixels[0][0], N_LINES, N_COLS))
        THROW2(error, 0, "cmp_matrix");

    return 1;

    error:

    return 0;
}

/* Test draw with overlap with unset pixels */
REGISTER_TEST(Dxyn_2)
{
    int x, y1, y2;
    char pixels[N_LINES][N_COLS] = {0};
    char sprite_init[][8] = {
        {1, 1, 0, 1, 1, 0, 1, 1},
        {1, 1, 1, 1, 1, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0},
    };

    char sprite_res_1[][8] = {
        {0, 1, 0, 1, 1, 0, 1, 1},
        {1, 1, 1, 1, 1, 0, 0, 1},
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0},
    };

    char sprite_res_2[][8] = {
        {1, 0, 0, 0, 0, 0, 0, 0},
    };

    x = 5;
    y1 = 6;
    y2 = 12;

    // 1er part
    if(!copy_matrix(&chip->pixels[0][0], N_LINES, N_COLS,
                     &sprite_init[0][0], x, y1,
                     NUM_ROWS(sprite_init), NUM_COLS(sprite_init)))
        THROW2(error, 0, "copy_matrix");

    if(!copy_matrix(&pixels[0][0], N_LINES, N_COLS,
                     &sprite_res_1[0][0], x, y1,
                     NUM_ROWS(sprite_res_1), NUM_COLS(sprite_res_1)))
        THROW2(error, 0, "copy_matrix");

    chip->cpu.i = MEM_START;
    chip->mem[chip->cpu.i] = 0b10000000;
    chip->cpu.v[0xa] = x;
    chip->cpu.v[0xb] = y1;

    if(!chip8_execute(chip, 0xDab1))
        return 0;

    if(chip->cpu.v[0xf] != 1)
        THROW2(error, 0, "chip->cpu.v[0xf] != 1");

    if(!cmp_matrix(&chip->pixels[0][0], &pixels[0][0], N_LINES, N_COLS))
        THROW2(error, 0, "cmp_matrix");

    // 2eme part
    if(!copy_matrix(&pixels[0][0], N_LINES, N_COLS,
                     &sprite_res_2[0][0], x, y2,
                     NUM_ROWS(sprite_res_2), NUM_COLS(sprite_res_2)))
        THROW2(error, 0, "copy_matrix");

    chip->cpu.v[0xa] = x;
    chip->cpu.v[0xb] = y2;
    chip->mem[chip->cpu.i + 10] = 0b10000000;

    if(!chip8_execute(chip, 0xDab1))
        return 0;

    // Test if Vf has been reset to 0
    if(chip->cpu.v[0xf] != 0)
        THROW2(error, 0, "chip->cpu.v[0xf] != 0");

    return 1;

    error:

    return 0;
}

/* Test draw outside */
REGISTER_TEST(Dxyn_3)
{
    int x, y;
    char pixels[N_LINES][N_COLS] = {0};

    char sprite_1[][8] = {
        {1, 1, 1, 1, 1, 1, 1, 1},
        {1, 0, 0, 0, 0, 0, 0, 0}
    };

    char sprite_2[][8] = {
        {1, 0, 0, 1, 1, 0, 0, 1}
    };

    x = 5;
    y = 31;

    if(!copy_matrix(&pixels[0][0], N_LINES, N_COLS,
                     &sprite_1[0][0], x, 0,
                     NUM_ROWS(sprite_1), NUM_COLS(sprite_1)))
        THROW2(error, 0, "copy_matrix");

    if(!copy_matrix(&pixels[0][0], N_LINES, N_COLS,
                     &sprite_2[0][0], x, 31,
                     NUM_ROWS(sprite_2), NUM_COLS(sprite_2)))
        THROW2(error, 0, "copy_matrix");

    chip->cpu.i = MEM_START;
    chip->mem[chip->cpu.i] = 0b10011001;
    chip->mem[chip->cpu.i + 1] = 0b11111111;
    chip->mem[chip->cpu.i + 2] = 0b10000000;
    chip->cpu.v[0xa] = x;
    chip->cpu.v[0xb] = y;

    if(!chip8_execute(chip, 0xDab3))
        return 0;

    if(chip->cpu.v[0xf] != 0)
        THROW2(error, 0, "chip->cpu.v[0xf] != 0");

    if(!cmp_matrix(&chip->pixels[0][0], &pixels[0][0], N_LINES, N_COLS))
        THROW2(error, 0, "cmp_matrix");

    return 1;

    error:

    return 0;
}

/* Test draw outside */
REGISTER_TEST(Dxyn_4)
{
    int x, y;
    char pixels[N_LINES][N_COLS] = {0};

    char sprite_1[][8] = {
        {1, 1, 0, 0, 1, 0, 0, 0},
        {1, 1, 1, 1, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
    };

    char sprite_2[][8] = {
        {0, 0, 0, 0, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 1, 1, 1},
        {0, 0, 0, 0, 0, 1, 0, 0},
    };

    x = 61;
    y = 5;

    if(!copy_matrix(&pixels[0][0], N_LINES, N_COLS,
                     &sprite_1[0][0], 0, y,
                     NUM_ROWS(sprite_1), NUM_COLS(sprite_1)))
        THROW2(error, 0, "copy_matrix");

    if(!copy_matrix(&pixels[0][0], N_LINES, N_COLS,
                     &sprite_2[0][0], 56, y,
                     NUM_ROWS(sprite_2), NUM_COLS(sprite_2)))
        THROW2(error, 0, "copy_matrix");

    chip->cpu.i = MEM_START;
    chip->mem[chip->cpu.i] = 0b10011001;
    chip->mem[chip->cpu.i + 1] = 0b11111111;
    chip->mem[chip->cpu.i + 2] = 0b10000000;
    chip->cpu.v[0xa] = x;
    chip->cpu.v[0xb] = y;

    if(!chip8_execute(chip, 0xDab3))
        return 0;

    if(chip->cpu.v[0xf] != 0)
        THROW2("chip->cpu.v[0xf] != 0");

    if(!cmp_matrix(&chip->pixels[0][0], &pixels[0][0], N_LINES, N_COLS))
        THROW2(error, 0, "cmp_matrix");

    return 1;

    error:

    return 0;
}

REGISTER_TEST(Ex9E_1)
{
    chip->keyboard[0x4] = 1;
    chip->cpu.v[0xA] = 0x4;

    if(!chip8_execute(chip, 0xea9e))
        return 0;

    return chip->cpu.pc == MEM_START + 0x4;
}

REGISTER_TEST(Ex9E_2)
{
    chip->keyboard[0x3] = 1;
    chip->cpu.v[0xA] = 0x4;

    if(!chip8_execute(chip, 0xea9e))
        return 0;

    return chip->cpu.pc == MEM_START + 0x2;
}

REGISTER_TEST(ExA1_1)
{
    chip->keyboard[0x4] = 1;
    chip->cpu.v[0xA] = 0x4;

    if(!chip8_execute(chip, 0xeaa1))
        return 0;

    return chip->cpu.pc == MEM_START + 0x2;
}

REGISTER_TEST(ExA1_2)
{
    chip->keyboard[0x3] = 1;
    chip->cpu.v[0xA] = 0x4;

    if(!chip8_execute(chip, 0xeaa1))
        return 0;

    return chip->cpu.pc == MEM_START + 0x4;
}

REGISTER_TEST(Fx07)
{
    chip->cpu.dt = 0x3;

    if(!chip8_execute(chip, 0xfa07))
        return 0;

    return chip->cpu.v[0xa] == 0x3;
}

REGISTER_TEST(Fx0A_1)
{
    struct chip8 * cpy = malloc(sizeof(struct chip8));
    if(!cpy)
        goto error;

    memcpy(cpy, chip, sizeof(struct chip8));

    if(!chip8_execute(chip, 0xfa0a))
        goto error;

    if(memcmp(chip, cpy, sizeof(struct chip8)) != 0)
        goto error;

    free(cpy);

    return 1;

    error:

    free(cpy);

    return 0;
}

REGISTER_TEST(Fx0A_2)
{
    chip->keyboard[0x4] = 1;

    if(!chip8_execute(chip, 0xfa0a))
        return 0;

    return chip->cpu.v[0xa] == 0x4;
}

REGISTER_TEST(Fx15)
{
    chip->cpu.v[0xa] = 0x3;

    if(!chip8_execute(chip, 0xfa15))
        return 0;

    return chip->cpu.dt == 0x3;
}

REGISTER_TEST(Fx18)
{
    chip->cpu.v[0xa] = 0x3;

    if(!chip8_execute(chip, 0xfa18))
        return 0;

    return chip->cpu.st == 0x3;
}

REGISTER_TEST(Fx1E)
{
    chip->cpu.v[0xa] = 0x3;
    chip->cpu.i = 0x1;

    if(!chip8_execute(chip, 0xfa1e))
        return 0;

    return chip->cpu.i == 0x4;
}

REGISTER_TEST(Fx29)
{
    int num = 0x3;

    chip->cpu.v[0xa] = num;

    if(!chip8_execute(chip, 0xfa29))
        return 0;

    return chip->cpu.i == 0x3 * 5 &&
        memcmp(&chip->mem[chip->cpu.i], fonts[num], sizeof(fonts[num])) == 0;
}

REGISTER_TEST(Fx33)
{
    chip->cpu.i = MEM_START;
    chip->cpu.v[0xa] = 124;

    if(!chip8_execute(chip, 0xfa33))
        return 0;

    return chip->mem[chip->cpu.i] == 1 &&
        chip->mem[chip->cpu.i + 1] == 2 &&
        chip->mem[chip->cpu.i + 2] == 4;
}

REGISTER_TEST(Fx55)
{
    chip->cpu.i = MEM_START;
    chip->cpu.v[0x0] = 1;
    chip->cpu.v[0x1] = 2;

    if(!chip8_execute(chip, 0xf155))
        return 0;

    return chip->mem[chip->cpu.i] == chip->cpu.v[0x0] &&
        chip->mem[chip->cpu.i + 1] == chip->cpu.v[0x1] &&
        chip->mem[chip->cpu.i + 2] == 0;
}

REGISTER_TEST(Fx65)
{
    chip->cpu.i = MEM_START;
    chip->mem[chip->cpu.i] = 1;
    chip->mem[chip->cpu.i + 1] = 2;

    if(!chip8_execute(chip, 0xf165))
        return 0;

    return chip->mem[chip->cpu.i] == chip->cpu.v[0x0] &&
        chip->mem[chip->cpu.i + 1] == chip->cpu.v[0x1] &&
        chip->cpu.v[0x2] == 0;
}

static int run_tests(void)
{
    int i;
    struct test_fn * iter;
    struct chip8 * chip = NULL;

    for(i=0, iter = head; iter != NULL; i++, iter=iter->next)
    {
        int res;

        chip = chip8_alloc();
        if(!chip)
            THROW2(error, 0, "chip8_alloc");

        res = iter->call(chip);
        if(res == -1)
            printf(YELLOW "%d %s) Not yet implemented\n" RESET, i, iter->name);
        else if(!res)
            printf(RED "%d %s) has failed\n" RESET, i, iter->name);
        else
            printf("%d %s) has succeed\n", i, iter->name);

        chip8_free(&chip);
    }

    return 1;

    error:

    chip8_free(&chip);

    return 0;
}

int main(void)
{
    if(run_tests())
        return EXIT_SUCCESS;
    return EXIT_FAILURE;
}
