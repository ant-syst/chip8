#include "sdl_io.h"
#include "tools.h"
#include "mapping.h"

#include <unistd.h>
#include <stdlib.h>

int main(void)
{
    struct io * io_ptr = NULL;
    static char pixels[N_LINES][N_COLS];

    io_ptr = sdl_alloc(800, 600, &key_mapping, 10, 10);
    if(!io_ptr)
        THROW(error, 0, "dp_alloc");

    if(!io_ptr->update(io_ptr, pixels))
        THROW(error, 0, "dp_render");

    sleep(4);

    pixels[N_LINES-1][N_COLS-1] = 1;

    if(!io_ptr->update(io_ptr, pixels))
        THROW(error, 0, "dp_render");

    sleep(4);

    io_ptr->free(&io_ptr);

    return EXIT_SUCCESS;

    error:

    if(io_ptr)
        io_ptr->free(&io_ptr);

    return EXIT_FAILURE;
}
