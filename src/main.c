#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <SDL2/SDL.h>

#include "chip8.h"
#include "peripheral.h"

#define MIN_ARGC 2
#define MAX_ARGC 4
#define USAGE "rom_path [1..256] (draw scale) [single|double] (buffering)"

#define CPU_HZ_DELAY 1.0 / 700
#define DISPLAY_HZ_DELAY 1.0 / 60

#define DEFAULT_RENDER_SCALE 8
#define DEFAULT_USE_DOUBLE_BUFFER 1

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

void handle_state_controls(uint8_t last_input) {
    if (0x01 & last_input) {
        chip8_write_state();
    }
    else if (0x02 & last_input) {
        chip8_load_state();
    }
    else if (0x03 & last_input) {
        chip8_display_updated = 1;
    }
}

int main(int argc, char *argv[]) {
    struct timeval time;
    double time_sec;
#ifndef DEBUG
    double next_cycle;
    double next_display;
#endif  // n DEBUG
    uint8_t input;
    uint8_t last_input = 0;
    uint8_t render_scale = DEFAULT_RENDER_SCALE;
    uint8_t use_double_buffering = DEFAULT_USE_DOUBLE_BUFFER;
    
    // Args check and parse
    if (argc < MIN_ARGC || argc > MAX_ARGC) {
        printf("Incorrect number of arguments.\n");
        printf("Usage: %s %s\n", argv[0], USAGE);
        return -1;
    }
    for (int i = 2; i < argc; i++) {
        int failure = 1;
        if (argv[i][0] == '-') {  // buffering
            if (strncmp(argv[i], "-single", 9) == 0) {
                use_double_buffering = 0;
                failure = 0;
            }
            else if (strncmp(argv[i], "-double", 9) == 0) {
                use_double_buffering = 1;
                failure = 0;
            }
        } else {  // render scale
            render_scale = atoi(argv[i]);
            failure = render_scale == 0;
        } if (failure) {
            printf("Bad argument '%s'\n", argv[i]);
            printf("Usage: %s %s\n", argv[0], USAGE);
            return -1;
        }
    }

    // Initialisation
    if (sdl_init(render_scale, use_double_buffering) != 0) {
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

    // Emulation loop
    while (!peripheral_quit_flag) {
        debug();
        gettimeofday(&time, NULL);
        time_sec = time.tv_sec + (time.tv_usec / 1000000.0);
        
#ifndef DEBUG
        if (time_sec > next_cycle) {
#endif  // n DEBUG
            input = sdl_input_step();
            chip8_step(input, time_sec);
            if ((last_input & 0x20) && !(input & 0x20)) {
                // on release of state control key
                handle_state_controls(last_input);
            }
            last_input = input;
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
