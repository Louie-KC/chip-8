#include "chip8.h"

#define TOTAL_MEMORY 0x1000    // 4096
#define FONT_START_ADDR 0x50   // 80
#define PROG_START_ADDR 0x200  // 512
#define NUM_GP_REGISTERS 16
#define STACK_SIZE 16

#define TIMER_HZ_DELAY 1.0 / 60

#define SUPER_CHIP_RPL_FILE "rpl-flags.bin"

// Memory
uint8_t memory[TOTAL_MEMORY];

// Registers
uint8_t  V[NUM_GP_REGISTERS];  // last = flag register
uint16_t pc;  // program counter
uint16_t I;   // index register

// Stack
uint16_t stack[STACK_SIZE];
uint16_t sp;  // stack pointer

// Timers
uint8_t delay_timer;
uint8_t sound_timer;

// (SUPER-CHIP 1.0) low/high resolution flag
uint8_t high_res_mode;

// courtesy of https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
uint8_t fonts[] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void update_timers(double time_sec) {
    if (time_sec > chip8_next_timer_update) {
        if (delay_timer > 0) {
            delay_timer -= 1;
        }
        if (sound_timer > 0) {
            sound_timer -= 1;
        }
        chip8_sound_off = sound_timer == 0;
        chip8_next_timer_update += TIMER_HZ_DELAY;
    }
}

// Retrieve the next instruction from memory and increment the program counter.
uint16_t fetch(void) {
    uint16_t instruction = memory[pc] << 8 | memory[pc + 1];
    pc += 2;
    return instruction;
}

void decode_and_exec(uint16_t instruction, uint8_t key_input) {
    uint8_t unrecognised = 0;

    uint8_t first_nibble = (instruction & 0xF000) >> 12;
    uint8_t last_nibble  = (instruction & 0x000F);

    uint8_t  X   = (instruction & 0x0F00) >> 8;
    uint8_t  Y   = (instruction & 0x00F0) >> 4;
    uint8_t  N   = (instruction & 0x000F);
    uint8_t  NN  = (instruction & 0x00FF);
    uint16_t NNN = (instruction & 0x0FFF);

    // display
    uint16_t dx;  // base col
    uint16_t dy;  // base row
    uint16_t dr;  // iter row
    uint16_t dc;  // iter col
    uint16_t di;  // buffer index

    // Intermediary result variable for overflow/underflow detection
    uint16_t arithmetic_result;

    FILE *rpl_f;

    switch (first_nibble) {
        case 0x0:
            switch (instruction) {
                // 00E0: clear display
                case 0x00E0:
                    memset(chip8_display, 0, DISPLAY_RES_X * DISPLAY_RES_Y);
                    chip8_display_updated = 1;
                    break;

                // 00EE: subroutine return
                case 0x00EE:
                    pc = stack[--sp];
                    break;

                // 00FB (SUPER-CHIP 1.1): Shift/move display pixels 4 right
                case 0x00FB:
                    for (dx = DISPLAY_RES_X - 4; dx >= 0 && dx != __UINT16_MAX__; dx--) {
                        for (dy = 0; dy < DISPLAY_RES_Y; dy++) {
                            di = (dy * DISPLAY_RES_X) + dx;
                            chip8_display[di] = chip8_display[di - 4];
                            if (dx < 4) {
                                chip8_display[di] = 0;
                            }
                        }
                    }
                    break;

                // 00FC (SUPER-CHIP 1.1): Shift/move display pixels 4 left
                case 0x00FC:
                    for (dx = 0; dx < DISPLAY_RES_X - 4; dx++) {
                        for (dy = 0; dy < DISPLAY_RES_Y; dy++) {
                            di = (dy * DISPLAY_RES_X) + dx;
                            chip8_display[di] = chip8_display[di + 4];
                            if (dx + 4 >= DISPLAY_RES_X - 4) {
                                chip8_display[di + 4] = 0;
                            }
                        }
                    }
                    break;

                // 00FD (SUPER-CHIP 1.0): Exit interpreter
                case 0x00FD:
                    chip8_exit_flag = 1;
                    break;
                
                // 00FE (SUPER-CHIP 1.0): Disable high resolution mode
                case 0x00FE:
                    high_res_mode = 0;
                    break;

                // 00FF (SUPER-CHIP 1.0): Enable high resolution mode
                case 0x00FF:
                    high_res_mode = 1;
                    break;

                default:
                    // 00CN (SUPER-CHIP 1.1): Move display pixels N down
                    if (Y == 0xC) {
                        for (dy = DISPLAY_RES_Y - 1; dy >= N && dy != __UINT16_MAX__; dy--) {
                            for (dx = 0; dx < DISPLAY_RES_X; dx++) {
                                di = (dy * DISPLAY_RES_X) + dx;
                                chip8_display[di * 1] = chip8_display[(dy - N) * DISPLAY_RES_X + dx];
                            }
                        }
                        memset(chip8_display, 0, DISPLAY_RES_Y * N);
                    } else {
                        unrecognised = 1;
                    }
            }
            break;
        
        // 1NNN: jump
        case 0x1:
            pc = NNN;
            break;

        // 2NNN: subroutine call
        case 0x2:
            stack[sp++] = pc;  // Push instruction address to return to onto stack
            pc = NNN;          // Jump to subroutine
            break;

        // 3XNN: skip 1 instruction if VX == NN
        case 0x3:
            if (V[X] == NN) {
                pc += 2;
            }
            break;

        // 4XNN: skip 1 instruction if VX != NN
        case 0x4:
            if (V[X] != NN) {
                pc += 2;
            }
            break;

        // 5XY0: skip 1 instruction if VX == VY
        case 0x5:
            if (V[X] == V[Y]) {
                pc += 2;
            }
            break;
        
        // 6XNN: set register V[X]
        case 0x6:
            V[X] = NN;
            break;
        
        // 7XNN: add to register V[X]
        case 0x7:
            V[X] += NN;
            break;

        case 0x8:
            switch (last_nibble) {
                // 8XY0: set register VX = VY
                case 0x0:
                    V[X] = V[Y];
                    break;
                
                // 8XY1: binary OR VX = VX | VY
                case 0x1:
                    V[X] = V[X] | V[Y];
                    break;
                
                // 8XY2: binary AND VX = VX & VY
                case 0x2:
                    V[X] = V[X] & V[Y];
                    break;
                
                // 8XY3: logical XOR VX = VX ^ VY
                case 0x3:
                    V[X] = V[X] ^ V[Y];
                    break;
                
                // 8XY4: add V[X] = V[X] + V[Y] w/ overflow detection
                case 0x4:
                    V[0xF] = 0;
                    arithmetic_result = (V[X] + V[Y]) % 256;
                    if (V[X] > arithmetic_result) {
                        V[0xF] ^= 1;  // overflow. flip bit
                    }
                    V[X] = arithmetic_result;  // no AND 0xFF due to mod 256
                    break;
                
                // 8XY5: subtract V[X] = V[X] - V[Y] w/ underflow detection
                case 0x5:
                    V[0xF] = 1;
                    arithmetic_result = (V[X] - V[Y]) % 256;
                    if (V[X] < arithmetic_result) {
                        V[0xF] ^= 1;  // underflow. flip bit
                    }
                    V[X] = arithmetic_result;  // no AND 0xFF due to mod 256
                    break;
                
                // 8XY6: Right shift. VX = VX >> 1 (ambiguous VY)
                case 0x6:
                    V[0xF] = V[X] & 0b00000001;
                    // V[Y] = V[X]; // Ambiguous
                    V[X] = V[X] >> 1;
                    break;
                
                // 8XY7: subtract V[X] = V[Y] - V[X] w/ underflow detection
                case 0x7:
                    V[0xF] = 1;
                    arithmetic_result = (V[Y] - V[X]) % 256;
                    if (V[Y] < arithmetic_result) {
                        V[0xF] ^= 1;  // underflow. flip bit
                    }
                    V[X] = arithmetic_result;  // no AND 0xFF due to mod 256
                    break;
                
                // 8XYE: Left shift. VX = VX << 1 (ambiguous VY)
                case 0xE:
                    V[0xF] = (V[X] & 0b10000000) >> 7;
                    // V[Y] = V[X]; // Ambiguous
                    V[X] = V[X] << 1;
                    break;
                
                default:
                    unrecognised = 1;
            }
            break;

        // 9XY0: skip 1 instruction if VX != VY
        case 0x9:
            if (V[X] != V[Y]) {
                pc += 2;
            }
            break;

        // ANNN: set index register
        case 0xA:
            I = NNN;
            break;

        // BNNN: jump PC to V0 + NNN (ambiguous, modern BXNN: PC = XNN + VX)
        case 0xB:
            pc = V[0x0] + NNN;
            break;

        // CXNN: store random number (ANDed with NN) in VX
        case 0xC:
            V[X] = (rand() & NN);
            break;
        
        // DXYN: display
        case 0xD:
            // The display positions should wrap. The sprite itself should not.
            dx = V[X] % DISPLAY_RES_X;
            dy = V[Y] % DISPLAY_RES_Y;
            V[0xF] = 0;
            chip8_display_updated = 1;
            for (dr = 0; dr < N && dy + dr < DISPLAY_RES_Y; dr++) {
                uint8_t sprite_data = memory[I + dr];
                for (dc = 0; dc < 8 && dx + dc < DISPLAY_RES_X; dc++) {
                    // Check each bit in a left to right order
                    if ((sprite_data & (0b10000000 >> dc)) != 0) {
                        di = (((dy + dr) * DISPLAY_RES_X) + dx + dc);
                        di = di * (2 - high_res_mode);
                        if (chip8_display[di] == 1) {
                            V[0xF] = 1;
                        }
                        // Flip the display pixels bit
                        chip8_display[di] ^= 1;
                        if (!high_res_mode) {
                            chip8_display[di + 1] ^= 1;
                            chip8_display[di + DISPLAY_RES_X] ^= 1;
                            chip8_display[di + DISPLAY_RES_X + 1] ^= 1;
                        }
                    }
                }
            }
            break;

        case 0xE:
            switch (NN) {
                // EX9E: skip 1 instruction if key VX is down
                case 0x9E:
                    if ((key_input & 0xF0) && (key_input & 0x0F) == V[X]) {
                        pc += 2;
                    }
                    break;

                // EXA1: skip 1 instruction if key VX is up
                case 0xA1:
                    if (!(key_input & 0xF0) || (key_input & 0x0F) != V[X]) {
                        pc += 2;
                    }
                    break;

                default:
                    unrecognised = 1;
            }
            break;

        // F...: Operations
        case 0xF:
            switch (NN) {
                // FX07: Set VX to the delay timers value
                case 0x07:
                    V[X] = delay_timer;
                    break;

                // FX15: Set delay timer to VX
                case 0x15:
                    delay_timer = V[X];
                    break;
                    
                // FX18: Set sound timer to VX
                case 0x18:
                    sound_timer = V[X];
                    break;

                // FX0A: Get key (blocking)
                case 0x0A:
                    if (key_input & 0x10) {
                        V[X] = key_input & 0x0F;
                    } else {
                        pc -= 2;  // Retry on next step
                    }
                    break;

                // FX1E: Add VX to index I
                case 0x1E:
                    arithmetic_result = (I + V[X]) % 0xFFF;
                    if (arithmetic_result < I) {
                        // Amiga interpreter behaviour
                        V[0xF] = 1;
                    }
                    I = arithmetic_result;
                    break;
                
                // FX29: Font character
                case 0x29:
                    if (!(V[X] & 0xF0)) {  // (SUPER-CHIP 1.0) low res mode
                        I = FONT_START_ADDR + (V[X] * 5);  // 5 bytes per char sprite
                        break;
                    } else {  // (SUPER-CHIP 1.0) high res mode
                        // fall through to FX30 (SUPER-CHIP 1.1 op)
                    }

                // FX30 (SUPER-CHIP 1.1): Large font character
                case 0x30:
                    printf("TODO: FX30\n");
                    break;

                // FX33: Binary-coded decimal conversion. Lay out digits starting at I
                case 0x33:
                    memory[I]     = V[X] / 100 % 10;
                    memory[I + 1] = V[X] / 10 % 10; 
                    memory[I + 2] = V[X] % 10;
                    break;
                
                // FX55: Store first n (determined by X) register values in memory
                case 0x55:
                    for (int i = 0; i <= X; i++) {
                        memory[I + i] = V[i];
                    }
                    // I += X + 1; // Ambiguous: old ROMS expect this
                    break;
                
                // FX65: Load first n (determined by X) register values from memory
                case 0x65:
                    for (int i = 0; i <= X; i++) {
                        V[i] = memory[I + i];
                    }
                    // I += X + 1;  // Ambiguous: old ROMS expect this
                    break;

                // FX75 (SUPER-CHIP 1.0): Store V0..VX in RPL user flags  TODO
                case 0x75:
                    rpl_f = fopen(SUPER_CHIP_RPL_FILE, "wb");
                    for (int i = 0; i <= X && i <= 7; i++) {
                        fwrite(&V[i], sizeof(uint8_t), 1, rpl_f);
                    }
                    fclose(rpl_f);
                    break;

                // FX85 (SUPER-CHIP 1.0): Read V0..VX from RPL user flags  TODO
                case 0x85:
                    rpl_f = fopen(SUPER_CHIP_RPL_FILE, "rb");
                    for (int i = 0; i <= X && i <= 7; i++) {
                        fread(&V[i], sizeof(uint8_t), 1, rpl_f);
                    }
                    fclose(rpl_f);
                    break;

                default:
                    unrecognised = 1;
            }
            break;

        default:
            unrecognised = 1;
    }
    if (unrecognised) {
        printf("[INFO] decode_and_exec: Unrecognised instruction '%04x'\n", instruction);
    }
}

#ifdef DEBUG
void chip8_print_state() {
    printf("* Registers\n");
    printf("V0: %02x, V1: %02x, V2: %02x, V3: %02x\n", V[0], V[1], V[2], V[3]);
    printf("V4: %02x, V5: %02x, V6: %02x, V7: %02x\n", V[4], V[5], V[6], V[7]);
    printf("V8: %02x, V9: %02x, VA: %02x, VB: %02x\n", V[8], V[9], V[0xA], V[0xB]);
    printf("VC: %02x, VD: %02x, VE: %02x, VF: %02x\n", V[0xC], V[0xD], V[0xE], V[0xF]);

    printf("\npc: %03x (%u)\n", pc, pc);
    printf("I : %03x (%u)\n", I, I);
    printf("sp: %02x (%u)\n", sp, sp);

    printf("\n* Stack");
    if (sp) {
        for (int i = sp - 1; i >= 0; i--) {
            printf("\n%u: %03x", i, stack[i]);
        }
    } else {
        printf("\nEmpty stack");
    }
    printf("\n");
}

void chip8_print_memory(uint16_t addr, uint16_t range) {
    u_int16_t i;
    printf("* Memory [0x%03x...0x%03x]", addr, addr + range);
    for (i = addr; i < addr + range; i++) {
        if (i % 4 == 0) {
            printf("\n");
        }
        printf("%x: %02x ", i, memory[i]);
    }
    printf("\n");
}

void chip8_print_next_op(void) {
    printf("op: %04x at addr %03x\n", memory[pc] << 8 | memory[pc + 1], pc);
}
#endif  // DEBUG

void chip8_init(void) {
    pc = PROG_START_ADDR;
    I  = 0;
    sp = 0;

    delay_timer = 0;
    sound_timer = 0;

    memset(memory,        0, TOTAL_MEMORY);
    memset(chip8_display, 0, DISPLAY_RES_X * DISPLAY_RES_Y);
    memset(stack,         0, STACK_SIZE);
    memset(V,             0, NUM_GP_REGISTERS);

    for (unsigned long i = 0; i < sizeof(fonts); i++) {
        memory[FONT_START_ADDR + i] = fonts[i];
    }

    chip8_display_updated = 0;

    // SUPER-CHIP 1.0
    high_res_mode = 0;
    chip8_exit_flag = 0;
}

uint8_t chip8_load_rom(const char *rom_path) {
    uint16_t file_bytes;
    uint8_t  buffer[TOTAL_MEMORY - PROG_START_ADDR] = {0};

    FILE *f = fopen(rom_path, "rb");
    if (!f) {
        printf("[FAIL] chip8_load_rom: Failed to open '%s'\n", rom_path);
        return -1;
    }
    
    // Determine file size
    fseek(f, 0, SEEK_END);
    file_bytes = ftell(f);
    fseek(f, 0, SEEK_SET);

    // Ensure fits within chip-8 memory
    if (file_bytes > TOTAL_MEMORY - PROG_START_ADDR) {
        printf("[ERROR] chip8_load_rom: File '%s' too large (%d)\n", rom_path, file_bytes);
        fclose(f);
        return -1;
    }

    if (fread(buffer, file_bytes, 1, f) == 0) {
        printf("[FAIL] chip8_load_rom: Failed to read '%s'\n", rom_path);
        fclose(f);
        return -1;
    }

    // Copy read bytes to chip-8 memory
    for (int i = 0; i < file_bytes; i++) {
        memory[PROG_START_ADDR + i] = buffer[i];
    }

    fclose(f);
    return 0;
}

void chip8_step(uint8_t key_input, double time_sec) {
    update_timers(time_sec);
    uint16_t instruction = fetch();
    decode_and_exec(instruction, key_input);
}