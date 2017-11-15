#include "mapping.h"
#include "tools.h"
#include "console_io.h"

#include <unistd.h>
#include <stdio.h>

int main(void)
{
    int i;
    static char pixels[N_LINES][N_COLS];
    static uint8_t keys[N_KEYS];
    struct io * io_ptr = NULL;

    FILE * logfs = fopen("log.txt", "w");
    if(!logfs)
        THROW(error, 1, "fopen");

    io_ptr = nc_alloc(&key_mapping);
    if(!io_ptr)
        THROW(error, 0, "nc_alloc");

    for(i=0; i<100; i++)
    {
        pixels[10][i] = 1;

        io_ptr->update(io_ptr, pixels);

        if(io_ptr->input_poll(io_ptr, keys) == CHIP8_QUIT)
            break;

        fprintf(logfs, "\n");
        for(int j=0; j<N_KEYS; j++)
        {
            if(keys[j])
                fprintf(logfs, "%2d => %d\n", j, (int)keys[j]);
        }

        fflush(logfs);

        sleep(2);
    }

    io_ptr->free(&io_ptr);

    fclose(logfs);

    return 0;

    error:

    return -1;
}
