#include <stdio.h>

#include <SDL2/SDL.h>

#define FAILURE -1
#define SUCCESS 0

#define AUDIO_N_CHANNELS 1
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_BUFFER_SIZE 1024 / 2
#define AUDIO_OSCILLATION_RATE 440.0f

struct {
    float current_step;
    float step_size;
} oscillator;

SDL_Window *window;
SDL_Renderer *renderer;

char draw_scale;
char quit_flag;

int sdl_init(char);
void sdl_close(void);
unsigned char sdl_input_step(void);
void sdl_draw_step(unsigned char *);
void sdl_audio_callback(void *, Uint8 *, int);

int sdl_init(char scale) {
    draw_scale = scale;

    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        fprintf(stderr, "SDL_Init Error: %s\n", SDL_GetError());
        return FAILURE;
    }

    // Init audio
    SDL_AudioSpec audio_spec = {
        .format = AUDIO_F32,
        .channels = AUDIO_N_CHANNELS,
        .freq = AUDIO_SAMPLE_RATE,
        .samples = AUDIO_BUFFER_SIZE,
        .callback = sdl_audio_callback
    };

    if (SDL_OpenAudio(&audio_spec, NULL) < 0) {
        fprintf(stderr, "SDL_OpenAudio Error: %s\n", SDL_GetError());
        return FAILURE;
    }
    oscillator.current_step = 0;
    oscillator.step_size = (2 * M_PI) / AUDIO_OSCILLATION_RATE;

    // Init window/video
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
    SDL_CloseAudio();
    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }
    if (window) {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();
}

// Get user input.
// 
// Maps a 4x4 grid of keys (1 as the top left) on the keyboard to the
// CHIP-8 keypad keys (0-F).
//
// At most 1 key is detected, with 1 (top left) having the highest
// precedence and the F (V the keyboard) being the lowest.
//
// Returns a 8 bit value, where the first half byte indicates if a key
// is being input, and the second half byte tells you which of the
// 0-F CHIP-8 keys is down.
unsigned char sdl_input_step(void) {
    SDL_Event event;
    const Uint8* keyboard;
    unsigned char input = 0;

    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            quit_flag = 1;
            return 0;
        }
    }

    keyboard = SDL_GetKeyboardState(NULL);

    // Input layout:
    // keypad  | mapped keys
    // --------|------------
    // 1 2 3 C | 1 2 3 4
    // 4 5 6 D | Q W E R
    // 7 8 9 E | A S D F
    // A 0 B F | Z X C V
    if (keyboard[SDL_SCANCODE_1]) {
        input = 0x11;
    }
    else if (keyboard[SDL_SCANCODE_2]) {
        input = 0x12;
    }
    else if (keyboard[SDL_SCANCODE_3]) {
        input = 0x13;
    }
    else if (keyboard[SDL_SCANCODE_4]) {
        input = 0x1C;
    }
    else if (keyboard[SDL_SCANCODE_Q]) {
        input = 0x14;
    }
    else if (keyboard[SDL_SCANCODE_W]) {
        input = 0x15;
    }
    else if (keyboard[SDL_SCANCODE_E]) {
        input = 0x16;
    }
    else if (keyboard[SDL_SCANCODE_R]) {
        input = 0x1D;
    }
    else if (keyboard[SDL_SCANCODE_A]) {
        input = 0x17;
    }
    else if (keyboard[SDL_SCANCODE_S]) {
        input = 0x18;
    }
    else if (keyboard[SDL_SCANCODE_D]) {
        input = 0x19;
    }
    else if (keyboard[SDL_SCANCODE_F]) {
        input = 0x1E;
    }
    else if (keyboard[SDL_SCANCODE_Z]) {
        input = 0x1A;
    }
    else if (keyboard[SDL_SCANCODE_X]) {
        input = 0x10;
    }
    else if (keyboard[SDL_SCANCODE_C]) {
        input = 0x1B;
    }
    else if (keyboard[SDL_SCANCODE_V]) {
        input = 0x1F;
    }

    return input;
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

// Fill audio buffer with a constant frequency set by `oscillator`.
void sdl_audio_callback(void *user_data, Uint8 *stream, int len) {
    float *fstream = (float *) stream;
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        oscillator.current_step += oscillator.step_size;
        fstream[i] = sinf(oscillator.current_step);
    }
}