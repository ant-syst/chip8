#include "tools.h"
#include "console_io.h"
#include <locale.h>

#include <stdlib.h>
#include <stdio.h>
#define _XOPEN_SOURCE_EXTENDED
#include <ncurses.h>

struct console {
    WINDOW * win;
    keys_mapping * km;
};

static void nc_free(struct io ** io_ptr)
{
    if(*io_ptr)
    {
        if((*io_ptr)->opaque)
        {
            struct console * csl = (*io_ptr)->opaque;
            delwin(csl->win);
            free(csl);
        }

        endwin();

        free((*io_ptr));
        *io_ptr = NULL;
    }
}

static int nc_update(struct io * io_ptr, char pixels[N_LINES][N_COLS])
{
    struct console * csl = io_ptr->opaque;
    int row, col;

    for(row=0; row<N_LINES; row++)
    {
        for(col=0; col<N_COLS; col++)
        {
            if(pixels[row][col])
                mvwprintw(csl->win, row, col, " ");
            else
                mvwaddch(csl->win, row, col, ' ' | A_REVERSE);
        }
    }

    wrefresh(csl->win);

    return 1;
}

static enum chip8_inputs nc_input_poll(struct io * io_ptr, uint8_t keys[N_KEYS])
{
    int key;
    struct console * csl = io_ptr->opaque;
    wint_t wc;

    key = wget_wch(csl->win, &wc);

    switch(key)
    {
        case ERR:
            // Non blocking io
        break;
        default:
        {
            enum chip8_key c_key = csl->km(wc);

            if(c_key != IO_KEY_NOT_FOUND)
                keys[c_key] = 1;
        }
    }

    return CHIP8_CONT;
}

struct io * nc_alloc(keys_mapping * km)
{
    struct io * io_ptr = NULL;
    struct console * csl = NULL;

    if(!initscr())
        THROW(error, 1, "initscr");

    // Don't echo any keypresses
    if(noecho() == ERR)
        THROW(error, 1, "noecho");

    // Don't display a cursor
    if(curs_set(FALSE) == ERR)
        THROW(error, 1, "curs_set");

    io_ptr = calloc(1, sizeof(struct io));
    if(!io_ptr)
        THROW(error, 1, "calloc");

    csl = calloc(1, sizeof(struct console));
    if(!csl)
        THROW(error, 1, "calloc");
    csl->km = km;

    io_ptr->opaque = csl;

    csl->win = newwin(N_LINES, N_COLS, 0, 0);
    if(!csl->win)
        THROW(error, 1, "newwin");

    io_ptr->update = &nc_update;
    io_ptr->free = &nc_free;
    io_ptr->input_poll = &nc_input_poll;

    // non blocking inputs
    if(nodelay(csl->win, TRUE) == ERR)
        THROW(error, 1, "nodelay");

    if(box(csl->win, ACS_VLINE, ACS_HLINE) != OK)
        THROW(error, 1, "box");

    if(keypad(csl->win, TRUE) != OK)
        THROW(error, 1, "keypad");

    return io_ptr;

    error:

    nc_free(&io_ptr);

    return NULL;
}
