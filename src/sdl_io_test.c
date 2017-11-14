#include "sdl_io.h"
#include "tools.h"

#include <unistd.h>
#include <stdlib.h>

// TODO double
static enum chip8_key key_mapping(int key)
{
	switch(key)
	{
		case '1':
			return KEY_1;
		case '2':
			return KEY_2;
		case '4':
			return KEY_3;
		case '3':
			return KEY_C;
		case 'q':
			return KEY_4;
		case 'w':
			return KEY_5;
		case 'e':
			return KEY_6;
		case 'r':
			return KEY_D;
		case 'a':
			return KEY_7;
		case 's':
			return KEY_8;
		case 'd':
			return KEY_9;
		case 'f':
			return KEY_E;
		case 'z':
			return KEY_A;
		case 'x':
			return KEY_0;
		case 'c':
			return KEY_B;
		case 'v':
			return KEY_F;
		default:
			return IO_KEY_NOT_FOUND;
	}
}

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
