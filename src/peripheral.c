#include <stdio.h>

#include <SDL2/SDL.h>

#define FAILURE -1
#define SUCCESS 0

SDL_Window *window;
SDL_Renderer *renderer;

char last_input;
char draw_scale;
char quit_flag;

int sdl_init(char);
void sdl_close(void);
void sdl_input_step(void);
void sdl_draw_step(unsigned char *);

int sdl_init(char scale) {
    draw_scale = scale;

    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return FAILURE;
    }
    window = SDL_CreateWindow("CHIP-8 Emulator", SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, 64 * scale, 32 * scale, SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        return FAILURE;
    }
    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        sdl_close();
        return FAILURE;
    }
    quit_flag = 0;
    return SUCCESS;
}

void sdl_close(void) {
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

void sdl_input_step(void) {
    SDL_Event event;
    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit_flag = 1;
        }
    }
}

void sdl_draw_step(unsigned char * display) {
    SDL_Rect rect;
    int x;
    int y;

    // Clear screen with black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw rectangles for each pixel
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (x = 0; x < 64; x++) {
        for (y = 0; y < 32; y++) {
            if (display[(y * 64) + x] == 1) {
                rect.x = x * draw_scale;
                rect.y = y * draw_scale;
                rect.w = draw_scale;
                rect.h = draw_scale;
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    // Render all drawings
    SDL_RenderPresent(renderer);
}