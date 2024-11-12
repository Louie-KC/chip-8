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

#define REQ_ARGC 2
#define USAGE "<rom file path>"

int main(int argc, char *argv[]) {
    if (argc != REQ_ARGC) {
        printf("Incorrect number of arguments.\n");
        printf("Usage: %s %s\n", argv[0], USAGE);
        return -1;
    }

    if (sdl_init() != 0) {
        return -1;
    }

    chip8_init();
    chip8_load_rom(argv[1]);

    while (!quit_flag) {
        sdl_input_step();
        chip8_step();
        sdl_draw_step(chip8_display);
    }

    sdl_close();
    return 0;
}
