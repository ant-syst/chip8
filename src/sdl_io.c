#include "tools.h"

#include <SDL2/SDL.h>

#define TRY(FN)                     \
do {                                \
    if((FN) != 0)                   \
        THROW("", sdl_error, 0);    \
} while(0)

struct display {
    SDL_Window *window;
    SDL_Renderer *renderer;
    int pixel_width;
    int pixel_height;
};

static int none_init(int (**init_fn)())
{
    init_fn = init_fn;

    return 1;
}

static int sdl_init(int (**init_fn)())
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init: %s", SDL_GetError());
        goto error;
    }

    *init_fn = &none_init;

    return 1;

    error:

    return 0;
}

void sdl_free(struct io ** io_ptr)
{
    if(*io_ptr)
    {
        struct display * dp = (*io_ptr)->opaque;

        if(dp)
        {
            if(dp->window)
                SDL_DestroyWindow(dp->window);

            if(dp->renderer)
                SDL_DestroyRenderer(dp->renderer);

            free(dp);
        }

        free(*io_ptr);
        *io_ptr = NULL;
    }
}

int sdl_render(struct io * io_ptr, char pixels[N_LINES][N_COLS])
{
    int y, x;
    SDL_Rect rect;
    struct display * dp = io_ptr->opaque;

    // Clear screen
    TRY(SDL_SetRenderDrawColor(dp->renderer, 0, 0, 0, 255));
    TRY(SDL_RenderClear(dp->renderer));

    for(x = 0; x < N_COLS; x++) {

        for(y = 0; y < N_LINES; y++) {

            if(pixels[y][x])
            {
                rect.x = x * dp->pixel_width;
                rect.y = y * dp->pixel_height;
                rect.w = dp->pixel_width;
                rect.h = dp->pixel_height;

                TRY(SDL_SetRenderDrawColor(dp->renderer, 0xFF, 0xFF, 0xFF, 0x00));
                TRY(SDL_RenderFillRect(dp->renderer, &rect));
            }
        }
    }

    SDL_RenderPresent(dp->renderer);

    return 1;

    sdl_error:

    fprintf(stderr, "sdl_error: %s", SDL_GetError());

    return 0;
}

static enum chip8_inputs sdl_input_poll(struct io * io_ptr, uint8_t keys[N_KEYS])
{
    SDL_Event event;
    int key;

    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT:
                return CHIP8_QUIT;
            case SDL_KEYDOWN:
            case SDL_KEYUP:

                /*printf("Physical %d:%s key acting as %d:%s key\n",
                        event.key.keysym.scancode,
                        SDL_GetScancodeName(event.key.keysym.scancode),
                        event.key.keysym.sym,
                        SDL_GetKeyName(event.key.keysym.sym));*/

                if(event.key.keysym.sym == SDLK_ESCAPE)
                    return CHIP8_QUIT;

                key = io_ptr->km(event.key.keysym.sym);
                if(key != IO_KEY_NOT_FOUND)
                    keys[key] = event.type == SDL_KEYDOWN;
            break;
        }
    }

    return CHIP8_CONT;
}

struct io * sdl_alloc(int width, int height, keys_mapping * km,
                      int pixel_width, int pixel_height)
{
    struct io * io_ptr = NULL;
    struct display * dp = NULL;

    // Init lib once
    static int (*init_fn)() = &sdl_init;
    if(!init_fn(&init_fn))
        THROW("init", error, 0);

    io_ptr = calloc(1, sizeof(struct io));
    if(!io_ptr)
        THROW("calloc", error, 1);
    io_ptr->update = &sdl_render;
    io_ptr->free = &sdl_free;
    io_ptr->input_poll = &sdl_input_poll;
    io_ptr->km = km;

    dp = calloc(1, sizeof(struct display));
    if(!dp)
        THROW("calloc", error, 1);
    dp->pixel_width = pixel_width;
    dp->pixel_height = pixel_height;

    io_ptr->opaque = dp;

    dp->window = SDL_CreateWindow("chip8", SDL_WINDOWPOS_CENTERED ,
                                  SDL_WINDOWPOS_CENTERED,
                                  width, height, SDL_WINDOW_RESIZABLE);
    if(!dp->window) {
        fprintf(stderr, "SDL_CreateWindow: %s", SDL_GetError());
        goto error;
    }

    dp->renderer = SDL_CreateRenderer(dp->window, -1, 0);
    if(!dp->renderer) {
        fprintf(stderr, "SDL_CreateRenderer: %s", SDL_GetError());
        goto error;
    }

    return io_ptr;

    error:

    sdl_free(&io_ptr);

    return NULL;
}

#ifdef TEST

// TOD double
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
    char pixels[N_LINES][N_COLS];

    io_ptr = sdl_alloc(800, 600, &key_mapping, 10, 10);
    if(!io_ptr)
        THROW("dp_alloc", error, 0);

    if(!sdl_render(io_ptr, pixels))
        THROW("dp_render", error, 0);

    sleep(1);

    pixels[N_LINES-1][N_COLS-1] = 1;

    if(!sdl_render(io_ptr, pixels))
        THROW("dp_render", error, 0);

    sdl_free(&io_ptr);

    return EXIT_SUCCESS;

    error:

    sdl_free(&io_ptr);

    return EXIT_FAILURE;
}

#endif
