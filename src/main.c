#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#include "chip8.c"

// #define ENABLE_DEBUG_LOG

#ifdef ENABLE_DEBUG_LOG
#define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif  // ENABLE_DEBUG_LOG

int main(int argc, char *argv[]) {
    
    chip8_init();
    // chip8_load_program(*)

    for (;;) {
        chip8_step();
    }

    return 0;
}
