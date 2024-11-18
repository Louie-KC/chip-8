#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DISPLAY_RES_X 128
#define DISPLAY_RES_Y 64

uint8_t chip8_display[DISPLAY_RES_X * DISPLAY_RES_Y];
uint8_t chip8_display_updated;
uint8_t chip8_sound_off;
uint8_t chip8_exit_flag;
double chip8_next_timer_update;

#ifdef DEBUG
void chip8_print_state(void);
void chip8_print_memory(uint16_t, uint16_t);
void chip8_print_next_op(void);
#endif  // DEBUG

/*
 * Initialise the chip8 emulator.
 */
void chip8_init(void);

/*
 * Load the ROM at the provided path into chip8 memory.
 */
uint8_t chip8_load_rom(const char *);

/*
 * Perform one step of chip8 functions.
 * 1. Update delay and sound timers
 * 2. Fetch next opcode
 * 3. Decode and execute fetched opcode
 */
void chip8_step(uint8_t, double);

#endif  // CHIP8_H