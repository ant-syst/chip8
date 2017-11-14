#include "sdl_io.h"
#include "tools.h"

#include <SDL2/SDL.h>

// TODO doublons
#define TRY(FN)                     \
do {                                \
    if((FN) != 0)                   \
        THROW(sdl_error, 0, "");   \
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
    int x, y, ww, wh;
    SDL_Rect disp, pix;
    struct display * dp = io_ptr->opaque;

    SDL_GetWindowSize(dp->window, &ww, &wh);

    // Clear screen
    TRY(SDL_SetRenderDrawColor(dp->renderer, 0, 0, 0, 255));
    TRY(SDL_RenderClear(dp->renderer));

    // Draw border screen
    TRY(SDL_SetRenderDrawColor(dp->renderer, 0xFF, 0xFF, 0xFF, 0x00));
    disp.w = dp->pixel_width * N_COLS;
    disp.h = dp->pixel_height * N_LINES;
    disp.x = (ww - disp.w) / 2;
    disp.x = disp.x < 0 ? 0 : disp.x;
    disp.y = (wh - disp.h) / 2;
    disp.y = disp.y < 0 ? 0 : disp.y;
    TRY(SDL_RenderDrawRect(dp->renderer, &disp));

    // Draw pixels
    pix.w = dp->pixel_width;
    pix.h = dp->pixel_height;

    for(x=0; x<N_COLS; x++)
    {
        for(y=0; y<N_LINES; y++)
        {
            if(pixels[y][x])
            {
                pix.x = disp.x + x * dp->pixel_width;
                pix.y = disp.y + y * dp->pixel_height;
                TRY(SDL_SetRenderDrawColor(dp->renderer, 0xFF, 0xFF, 0xFF, 0x00));
                TRY(SDL_RenderFillRect(dp->renderer, &pix));
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
                if(event.key.keysym.sym == SDLK_ESCAPE)
                    return CHIP8_QUIT;

                key = io_ptr->km(event.key.keysym.sym);
                if(key != IO_KEY_NOT_FOUND)
                    keys[key] = event.type == SDL_KEYDOWN;  // TODO fail
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
        THROW(error, 0, "init");

    io_ptr = calloc(1, sizeof(struct io));
    if(!io_ptr)
        THROW(error, 1, "calloc");
    io_ptr->update = &sdl_render;
    io_ptr->free = &sdl_free;
    io_ptr->input_poll = &sdl_input_poll;
    io_ptr->km = km;

    dp = calloc(1, sizeof(struct display));
    if(!dp)
        THROW(error, 1, "calloc");
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
