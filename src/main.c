#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <SDL2/SDL.h>

#include "chip8.h"
#include "peripheral.h"

// #define ENABLE_DEBUG_LOG

#ifdef ENABLE_DEBUG_LOG
#define DEBUG_LOG(...) printf(__VA_ARGS__)
#else
#define DEBUG_LOG(...)
#endif  // ENABLE_DEBUG_LOG

#define REQ_ARGC 2
#define USAGE "<rom file path>"

#define CPU_HZ_DELAY 1.0 / 700
#define DISPLAY_HZ_DELAY 1.0 / 60

int main(int argc, char *argv[]) {
    struct timeval time;
    double time_sec;
    double next_cycle;
    double next_display;
    uint8_t input;

    if (argc != REQ_ARGC) {
        printf("Incorrect number of arguments.\n");
        printf("Usage: %s %s\n", argv[0], USAGE);
        return -1;
    }

    if (sdl_init(8) != 0) {
        return -1;
    }

    chip8_init();
    chip8_load_rom(argv[1]);

    gettimeofday(&time, NULL);
    time_sec = time.tv_sec + (time.tv_usec / 1000000.0);
    next_cycle = time_sec;
    next_display = time_sec;
    chip8_next_timer_update = time_sec;  // manually set next timer update time
    chip8_sound_off = 1;

    while (!peripheral_quit_flag) {
        gettimeofday(&time, NULL);
        time_sec = time.tv_sec + (time.tv_usec / 1000000.0);

        if (time_sec > next_cycle) {
            input = sdl_input_step();
            chip8_step(input, time_sec);
            next_cycle += CPU_HZ_DELAY;
        }

        if (time_sec > next_display) {
            if (chip8_display_updated) {
                sdl_draw_step(chip8_display);
                chip8_display_updated = 0;
            }
            next_display += DISPLAY_HZ_DELAY;
        }

        // Pause/unpause audio based on sound timer
        SDL_PauseAudio(chip8_sound_off);

        // Very brief sleep to reduce CPU load of this busy loop
        usleep(8);
    }

    sdl_close();
    return 0;
}
