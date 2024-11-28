#ifndef CHIP8_H
#define CHIP8_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define DISPLAY_RES_X 128
#define DISPLAY_RES_Y 64

// Quirks: For specific modern quirks, XOR ^ with legacy mode
#define CHIP8_QUIRK_LEGACY_SHIFT 0x1  // Shift VY in VX instead of shift VX in-place
#define CHIP8_QUIRK_LEGACY_JUMP_V0_OFFSET 0x2  // V0 instead of VX
#define CHIP8_QUIRK_LEGACY_REG_DUMP_I 0x4  // I added to
#define CHIP8_QUIRK_SUPER_LEGACY_SCROLL 0x8

#define CHIP8_QUIRK_LEGACY_MODE 0xF
#define CHIP8_QUIRK_MODERN_MODE 0x0

#define CHIP8_STATE_FILE_NAME "ch8-state.bin"

uint8_t chip8_display[DISPLAY_RES_X * DISPLAY_RES_Y];
uint8_t chip8_display_updated;
uint8_t chip8_sound_off;
uint8_t chip8_quirk_flag;
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

/*
 * Write all of the emulators state to a `bin` file specified
 * by `CHIP8_STATE_FILE_NAME`.
 */
void chip8_write_state(void);

/*
 * Load the state recorded in a `bin` file specified by
 * `CHIP8_STATE_FILE_NAME` into the emulator.
 */
void chip8_load_state(void);

#endif  // CHIP8_H