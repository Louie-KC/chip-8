#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <SDL2/SDL.h>

#include "chip8.h"
#include "peripheral.h"

#define REQ_ARGC 2
#define USAGE "[ROM file path]"

#define CPU_HZ_DELAY 1.0 / 700
#define DISPLAY_HZ_DELAY 1.0 / 60

#ifdef DEBUG
unsigned int steps_can_run = 0;

void debug_print_keys(void) {
    printf("Debug mode keys:\n");
    printf("s         - step 1 instruction\n");
    printf("s [n]     - step n (decimal) instructions\n");
    printf("i         - print chip8 state\n");
    printf("m [a]     - print memory at address a (hex)\n");
    printf("m [a] [l] - print memory at address a (hex), length l (hex)\n");
    printf("n         - print next opcode/instruction\n");
    printf("h         - print this list again\n");
    printf("q         - quit\n");
}
#endif  // DEBUG

void debug(void) {
#ifdef DEBUG
    char buffer[64];
    short mem_addr;
    short mem_len;

    if (steps_can_run > 0) {
        steps_can_run--;
    } else {
        // while (!steps_can_run) {
        for (;;) {
            printf("> ");
            if (!fgets(buffer, 64, stdin) || !strnlen(buffer, 64)) {
                continue;
            }
            if (buffer[0] == 's') {
                if (sscanf(buffer, "%*s %u", &steps_can_run) == -1) {
                    steps_can_run = 1;
                }
                if (steps_can_run == 1) {
                    printf("Running 1 step\n");
                } else {
                    printf("Running %u steps\n", steps_can_run);
                }
                steps_can_run--;
                break;
            }
            else if (buffer[0] == 'i') {
                chip8_print_state();
            }
            else if (buffer[0] == 'm') {
                mem_len = 1;
                if (sscanf(buffer, "%*s %hx %hx", &mem_addr, &mem_len) != -1) {
                    chip8_print_memory(mem_addr, mem_len);
                }
            }
            else if (buffer[0] == 'n') {
                printf("Next ");
                chip8_print_next_op();
            }
            else if (buffer[0] == 'h') {
                debug_print_keys();
            }
            else if (buffer[0] == 'q') {
                sdl_close();
                exit(0);
            } else {
                printf("Invalid command.\n");
            }

            usleep(8);
        }

    }
    printf("Running ");
    chip8_print_next_op();
#endif  // DEBUG
}

int main(int argc, char *argv[]) {
    struct timeval time;
    double time_sec;
#ifndef DEBUG
    double next_cycle;
    double next_display;
#endif  // n DEBUG
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
#ifndef DEBUG
    next_cycle = time_sec;
    next_display = time_sec;
#endif  // n DEBUG
    chip8_next_timer_update = time_sec;  // manually set next timer update time
    chip8_sound_off = 1;
    
#ifdef DEBUG
    debug_print_keys();
#endif  // DEBUG
    while (!peripheral_quit_flag) {
        debug();
        gettimeofday(&time, NULL);
        time_sec = time.tv_sec + (time.tv_usec / 1000000.0);
#ifndef DEBUG
        if (time_sec > next_cycle) {
#endif  // n DEBUG
            input = sdl_input_step();
            chip8_step(input, time_sec);
#ifndef DEBUG
            next_cycle += CPU_HZ_DELAY;
        }

        if (time_sec > next_display) {
#endif  // n DEBUG
            if (chip8_display_updated) {
                sdl_draw_step(chip8_display);
                chip8_display_updated = 0;
            }
#ifndef DEBUG
            next_display += DISPLAY_HZ_DELAY;
        }
#endif  // n DEBUG

        // Pause/unpause audio based on sound timer
        SDL_PauseAudio(chip8_sound_off);

        // Very brief sleep to reduce CPU load of this busy loop
        usleep(8);
    }

    sdl_close();
    return 0;
}
