#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "chip8.c"
#include "peripheral.c"

// #define ENABLE_DEBUG_LOG

#ifdef ENABLE_DEBUG_LOG
#define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif  // ENABLE_DEBUG_LOG

int main(int argc, char *argv[]) {
    
    if (sdl_init() != 0) {
        return -1;
    }

    chip8_init();
    // chip8_load_program(*)

    // Test: Set half of the screens pixels to on/white
    memset(chip8_display, 1, (DISPLAY_RES_X * DISPLAY_RES_Y) / 2);

    while (!quit_flag) {
        sdl_input_step();
        // chip8_step();  // TEMP: Commented out to allow the draw step to occur.
        sdl_draw_step(chip8_display);
    }

    sdl_close();
    return 0;
}
