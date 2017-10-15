#ifndef H_SDL_IO
#define H_SDL_IO

#include "chip8.h"

struct io * sdl_alloc(int width, int height, keys_mapping * km,
                      int pixel_width, int pixel_height);

#endif
