#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DISPLAY_RES_X 64
#define DISPLAY_RES_Y 32

uint8_t chip8_display[DISPLAY_RES_X * DISPLAY_RES_Y];
uint8_t chip8_display_updated;
uint8_t chip8_sound_off;
double chip8_next_timer_update;

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