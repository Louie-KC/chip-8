#include "peripheral.h"
#include "chip8.h"

#define FAILURE -1
#define SUCCESS 0

#define AUDIO_N_CHANNELS 1
#define AUDIO_SAMPLE_RATE 44100
#define AUDIO_BUFFER_SIZE 512
#define AUDIO_OSCILLATION_RATE 440.0f

void sdl_audio_callback(void *, Uint8 *, int);

struct {
    float current_step;
    float step_size;
} oscillator;

SDL_Window *window;
SDL_Renderer *renderer;
uint8_t draw_scale;

// Video single/double buffering
uint8_t (*buffer_draw_colour_cmp_fn)(uint8_t *, uint16_t);
void (*buffer_fn)(uint8_t *);
uint8_t video_last_frame[DISPLAY_RES_X * DISPLAY_RES_Y];

// Check only the current display buffer to determine if a pixel should be on.
uint8_t single_buffer_colour(uint8_t *display, uint16_t i) {
    return display[i];
}

// Check both the current and last display buffer to determine if a pixel should be on.
uint8_t double_buffer_colour(uint8_t *display, uint16_t i) {
    return display[i] | video_last_frame[i];
}

// Do nothing with the current buffer.
void single_buffer_post_draw(uint8_t *display) {  // do nothing
    (void) display;
}

// Copy the current buffer into the last frame buffer for next draw double buffering.
void double_buffer_post_draw(uint8_t *display) {
    memcpy(video_last_frame, display, DISPLAY_RES_X * DISPLAY_RES_Y);
}

uint8_t sdl_init(uint8_t scale, uint8_t double_buffer) {
    draw_scale = scale;

    if (double_buffer) {
        buffer_draw_colour_cmp_fn = &double_buffer_colour;
        buffer_fn = &double_buffer_post_draw;
    } else {
        buffer_draw_colour_cmp_fn = &single_buffer_colour;
        buffer_fn = &single_buffer_post_draw;  // do nothing
    }

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
        SDL_WINDOWPOS_CENTERED, DISPLAY_RES_X * scale, DISPLAY_RES_Y * scale,
        SDL_WINDOW_SHOWN);
    if (!window) {
        fprintf(stderr, "SDL_CreateWindow Error: %s\n", SDL_GetError());
        sdl_close();
        return FAILURE;
    }
    renderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!renderer) {
        fprintf(stderr, "SDL_CreateRenderer Error: %s\n", SDL_GetError());
        sdl_close();
        return FAILURE;
    }
    // Start with a clear screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    memset(video_last_frame, 0, DISPLAY_RES_X * DISPLAY_RES_Y);

    peripheral_quit_flag = 0;
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

uint8_t sdl_input_step(void) {
    SDL_Event event;
    const uint8_t* keyboard;
    uint8_t input = 0;

    if (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            peripheral_quit_flag = 1;
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
    // State save
    else if (keyboard[SDL_SCANCODE_F5]) {
        input = 0x21;
    }
    // State load
    else if (keyboard[SDL_SCANCODE_F9]) {
        input = 0x22;
    }
    // Force re-draw
    else if (keyboard[SDL_SCANCODE_F10]) {
        input = 0x23;
    }

    return input;
}

void sdl_draw_step(uint8_t *display) {
    SDL_Rect rect;
    uint8_t  x;
    uint8_t  y;
    uint16_t i;
    
    // Draw rectangles for each pixel
    for (x = 0; x < DISPLAY_RES_X; x++) {
        for (y = 0; y < DISPLAY_RES_Y; y++) {
            i = (y * DISPLAY_RES_X) + x;
            rect.x = x * draw_scale;
            rect.y = y * draw_scale;
            rect.w = draw_scale;
            rect.h = draw_scale;

            // Call single or double buffer cmp based on how sdl_init was called
            if ((*buffer_draw_colour_cmp_fn)(display, i)) {
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            SDL_RenderFillRect(renderer, &rect);
        }
    }

    // Call buffering function for single (do nothing) or double buffering.
    (*buffer_fn)(display);

    // Render all drawings
    SDL_RenderPresent(renderer);
}

// Fill audio buffer with a constant frequency set by `oscillator`.
void sdl_audio_callback(void *user_data, uint8_t *stream, int len) {
    (void) user_data;   // unused
    (void) len;         // unused
    float *fstream = (float *) stream;
    
    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
        oscillator.current_step += oscillator.step_size;
        fstream[i] = sinf(oscillator.current_step);
    }
}