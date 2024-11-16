#ifndef PERIPHERAL_H
#define PERIPHERAL_H

#include <stdio.h>
#include <stdint.h>
#include <SDL2/SDL.h>

char peripheral_quit_flag;

/*
 * Initialise the front end of the emulator with SDL.
 * Audio, Video and Events (user input) are enabled.
 * 
 * @param1: render scale
 * @param2: use double buffering
 */
uint8_t sdl_init(uint8_t, uint8_t);

/*
 * Tidy-up of SDL. Close, destroy and quit all SDL contexts/objects.
 */
void sdl_close(void);

/*
 * Get user input in the CHIP-8 keypad format.
 * 
 * Maps a 4x4 grid of keys ('1' as top left, 'v' as bottom right) on the
 * keyboard to the CHIP-8 keypad keys (0-F).
 * 
 * At most 1 key is detected, with the top left/'1' key having the
 * highest precedence, followed by the '2' key.
 * 
 * Returns an 8 bit value where the first 4 bits indicate whether any
 * key is down/pushed, and the second 4 bits desribing the CHIP-8 key
 * that is down/pushed.
 * e.g.
 * 
 * 00000000
 * ^^^^
 * No key is pushed.
 * 
 * 00010000
 * ^^^^
 * The CHIP-8 '0' key is pushed.
 */
uint8_t sdl_input_step(void);

/*
 * Update the SDL window/renderer with a new image.
 * 
 * Takes a display buffer (from chip8) and draws it on the SDL
 * window/renderer combined with the previous frame (double buffering).
 * 
 * Updates the previous frame buffer with the passed in buffer values.
 * 
 * @param: Pointer to 64x32 CHIP-8 display buffer
 */
void sdl_draw_step(uint8_t *);

#endif  // PERIPHERAL_H