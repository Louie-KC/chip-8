#include <stdio.h>
#include <string.h>

#define TOTAL_MEMORY 0x1000    // 4096
#define FONT_START_ADDR 0x50   // 80
#define PROG_START_ADDR 0x200  // 512
#define NUM_GP_REGISTERS 16
#define DISPLAY_RES_X 64
#define DISPLAY_RES_Y 32
#define STACK_SIZE 16

// Flags
unsigned char chip8_display_updated;

// Memory
unsigned char memory[TOTAL_MEMORY];

// Registers
unsigned char V[NUM_GP_REGISTERS];  // last = flag register
unsigned short pc;  // program counter
unsigned short I;   // index register

// Stack
unsigned short stack[STACK_SIZE];
unsigned short sp;  // stack pointer

// Display
unsigned char chip8_display[DISPLAY_RES_X * DISPLAY_RES_Y];

// Timers
unsigned char delay_timer;
unsigned char sound_timer;

// courtesy of https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
unsigned char fonts[] = {
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

// Retrieve the next instruction from memory and increment the program counter.
unsigned short fetch() {
    unsigned short instruction = memory[pc] << 8 | memory[pc + 1];
    pc += 2;
    return instruction;
}

void decode_and_exec(unsigned short instruction) {
    unsigned char unrecognised = 0;

    unsigned char first_nibble = (instruction & 0xF000) >> 12;
    unsigned char last_nibble  = (instruction & 0x000F);

    unsigned char  X   = (instruction & 0x0F00) >> 8;
    unsigned char  Y   = (instruction & 0x00F0) >> 4;
    unsigned char  N   = (instruction & 0x000F);
    unsigned char  NN  = (instruction & 0x00FF);
    unsigned short NNN = (instruction & 0x0FFF);

    // display
    unsigned short dx;
    unsigned short dy;
    unsigned short di;

    unsigned char arithmetic_result;  // for overflow/underflow detection

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
                default:
                    unrecognised = 1;
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
                    V[X] = arithmetic_result;
                    break;
                
                // 8XY5: subtract V[X] = V[X] - V[Y] w/ underflow detection
                case 0x5:
                    V[0xF] = 1;
                    arithmetic_result = (V[X] - V[Y]) % 256;
                    if (V[X] < arithmetic_result) {
                        V[0xF] ^= 1;  // underflow. flip bit
                    }
                    V[X] = arithmetic_result;
                    break;
                
                // 8XY6: Left shift. VX = VX << 1 (ambiguous VY)
                case 0x6:
                    V[0xF] = (V[X] & 0b10000000) >> 7;
                    // V[Y] = V[X]; // Ambiguous
                    V[X] = V[X] << 1;
                    break;
                
                // 8XY7: subtract V[X] = V[Y] - V[X] w/ underflow detection
                case 0x7:
                    V[0xF] = 1;
                    arithmetic_result = (V[Y] - V[X]) % 256;
                    if (V[Y] < arithmetic_result) {
                        V[0xF] ^= 1;  // underflow. flip bit
                    }
                    V[X] = arithmetic_result;
                    break;
                
                // 8XYE: Right shift. VX = VX >> 1 (ambiguous VY)
                case 0xE:
                    V[0xF] = V[X] & 0b00000001;
                    // V[Y] = V[X]; // Ambiguous
                    V[X] = V[X] >> 1;
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
        
        // DXYN: display
        case 0xD:
            // The display positions should wrap. The sprite itself should not.
            dx = V[X] % DISPLAY_RES_X;
            dy = V[Y] % DISPLAY_RES_Y;
            V[0xF] = 0;
            chip8_display_updated = 1;
            for (int row = 0; row < N && dy + row < DISPLAY_RES_Y; row++) {
                unsigned char sprite_data = memory[I + row];
                for (int col = 0; col < 8 && dx + col < DISPLAY_RES_X; col++) {
                    // Check each bit in a left to right order
                    if ((sprite_data & (0b10000000 >> col)) != 0) {
                        di = (((dy + row) * DISPLAY_RES_X) + dx + col);
                        if (chip8_display[di] == 1) {
                            V[0xF] = 1;
                        }
                        // Flip the display pixels bit
                        chip8_display[di] ^= 1;
                    }
                }
            }
            break;

        default:
            unrecognised = 1;
    }
    if (unrecognised) {
        printf("[INFO] decode_and_exec: Unrecognised instruction '%x'\n", instruction);
    }
}

void chip8_init() {
    pc = PROG_START_ADDR;
    I  = 0;
    sp = 0;

    delay_timer = 0;
    sound_timer = 0;

    memset(memory,        0, TOTAL_MEMORY);
    memset(chip8_display, 0, DISPLAY_RES_X * DISPLAY_RES_Y);
    memset(stack,         0, STACK_SIZE);
    memset(V,             0, NUM_GP_REGISTERS);

    for (int i = 0; i < sizeof(fonts); i++) {
        memory[FONT_START_ADDR + i] = fonts[i];
    }

    chip8_display_updated = 0;
}

int chip8_load_rom(char *rom_path) {
    unsigned short file_bytes;
    unsigned char buffer[TOTAL_MEMORY - PROG_START_ADDR] = {0};

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

void chip8_step() {
    unsigned short instruction = fetch();
    decode_and_exec(instruction);
}