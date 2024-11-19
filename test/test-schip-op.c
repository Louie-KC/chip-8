#include <stdio.h>
#include <assert.h>

#include "../src/chip8.c"

void test_super_chip_init(void) {
    chip8_init();

    // A partial test of the standard chip8 init

    assert(pc == PROG_START_ADDR);
    assert(sp == 0);
    assert(memory[PROG_START_ADDR] == 0);

    // If the above fail, run standard chip8 tests

    // SUPER-CHIP specific init assertions
    assert(high_res_mode == 0);
    assert(chip8_exit_flag == 0);

    printf("[PASS] test_super_chip_init\n");
}

// Test: Scroll display N pixels down (high res mode). Move pixels down
void test_00CN_high_res(void) {
    // 1. N = 0. Single pixel on top left corner and bottom left corner
    chip8_init();
    high_res_mode = 1;
    chip8_display[0] = 1;
    chip8_display[DISPLAY_RES_X * (DISPLAY_RES_Y - 1)] = 1;
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xC0;
    chip8_step(0, 0.0);
    assert(chip8_display[0] == 1);
    assert(chip8_display[DISPLAY_RES_X * (DISPLAY_RES_Y - 1)] == 1);

    // 2. N = 1. Single pixel on top left corner and bottom left corner
    chip8_init();
    high_res_mode = 1;
    chip8_display[0] = 1;  // top left
    chip8_display[DISPLAY_RES_X * (DISPLAY_RES_Y - 1)] = 1;  // bottom left
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xC1;
    chip8_step(0, 0.0);
    assert(chip8_display[0] == 0);
    assert(chip8_display[DISPLAY_RES_X] == 1);
    assert(chip8_display[DISPLAY_RES_X * (DISPLAY_RES_Y - 1)] == 0);

    // 3. N = 2
    chip8_init();
    high_res_mode = 1;
    chip8_display[0] = 1;
    chip8_display[DISPLAY_RES_X * (DISPLAY_RES_Y - 4)] = 1;
    chip8_display[DISPLAY_RES_X * (DISPLAY_RES_Y - 3)] = 1;
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xC2;
    chip8_step(0, 0.0);
    assert(chip8_display[0] == 0);  // lose the top left pixel
    assert(chip8_display[DISPLAY_RES_X] == 0);
    assert(chip8_display[DISPLAY_RES_X * 2] == 1);
    assert(chip8_display[DISPLAY_RES_X * (DISPLAY_RES_Y - 4)] == 0);
    assert(chip8_display[DISPLAY_RES_X * (DISPLAY_RES_Y - 3)] == 0);
    assert(chip8_display[DISPLAY_RES_X * (DISPLAY_RES_Y - 2)] == 1);
    assert(chip8_display[DISPLAY_RES_X * (DISPLAY_RES_Y - 1)] == 1);

    printf("[PASS] test_00CN_high_res\n");
}

// Test: Scroll right 4 pixels (high res mode). Move pixels right
void test_00FB_high_res(void) {
    // 1.
    chip8_init();
    high_res_mode = 1;
    chip8_display[0] = 1;
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFB;
    chip8_step(0, 0.0);
    assert(chip8_display[0] == 0);
    assert(chip8_display[1] == 0);
    assert(chip8_display[2] == 0);
    assert(chip8_display[3] == 0);
    assert(chip8_display[4] == 1);

    // 2.
    chip8_init();
    high_res_mode = 1;
    chip8_display[0] = 1;
    chip8_display[1] = 1;
    chip8_display[2] = 0;
    chip8_display[3] = 1;
    chip8_display[DISPLAY_RES_X] = 1;
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFB;
    chip8_step(0, 0.0);
    printf("post step\n");
    assert(chip8_display[0] == 0);
    assert(chip8_display[1] == 0);
    assert(chip8_display[2] == 0);
    assert(chip8_display[3] == 0);
    assert(chip8_display[4] == 1);
    assert(chip8_display[5] == 1);
    assert(chip8_display[6] == 0);
    assert(chip8_display[7] == 1);
    assert(chip8_display[DISPLAY_RES_X] == 0);
    assert(chip8_display[DISPLAY_RES_X + 1] == 0);
    assert(chip8_display[DISPLAY_RES_X + 2] == 0);
    assert(chip8_display[DISPLAY_RES_X + 3] == 0);
    assert(chip8_display[DISPLAY_RES_X + 4] == 1);
    assert(chip8_display[DISPLAY_RES_X + 5] == 0);

    // 3. Ensure new edge is empty
    chip8_init();
    high_res_mode = 1;
    memset(chip8_display, 1, DISPLAY_RES_X * DISPLAY_RES_Y);
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFB;
    chip8_step(0, 0.0);
    assert(chip8_display[0] == 0);
    assert(chip8_display[1] == 0);
    assert(chip8_display[2] == 0);
    assert(chip8_display[3] == 0);
    assert(chip8_display[4] == 1);
    assert(chip8_display[DISPLAY_RES_X] == 0);
    assert(chip8_display[DISPLAY_RES_X + 1] == 0);
    assert(chip8_display[DISPLAY_RES_X + 2] == 0);
    assert(chip8_display[DISPLAY_RES_X + 3] == 0);
    assert(chip8_display[DISPLAY_RES_X + 4] == 1);

    printf("[PASS] test_00FB_high_res\n");
}

// Test: Scroll left 4 pixels (high res mode). Move pixels left
void test_00FC_high_res(void) {
    // 1.
    chip8_init();
    high_res_mode = 1;
    chip8_display[DISPLAY_RES_X - 1] = 1;
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFC;
    chip8_step(0, 0.0);
    assert(chip8_display[DISPLAY_RES_X] == 0);
    assert(chip8_display[DISPLAY_RES_X - 1] == 0);
    assert(chip8_display[DISPLAY_RES_X - 2] == 0);
    assert(chip8_display[DISPLAY_RES_X - 3] == 0);
    assert(chip8_display[DISPLAY_RES_X - 4] == 0);
    assert(chip8_display[DISPLAY_RES_X - 5] == 1);

    // 2
    chip8_init();
    high_res_mode = 1;
    chip8_display[0] = 1;
    chip8_display[DISPLAY_RES_X - 1] = 1;
    chip8_display[DISPLAY_RES_X - 2] = 1;
    chip8_display[DISPLAY_RES_X * 2 - 1] = 1;
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFC;
    chip8_step(0, 0.0);
    assert(chip8_display[0] == 0);
    assert(chip8_display[DISPLAY_RES_X - 1] == 0);
    assert(chip8_display[DISPLAY_RES_X - 2] == 0);
    assert(chip8_display[DISPLAY_RES_X - 3] == 0);
    assert(chip8_display[DISPLAY_RES_X - 4] == 0);
    assert(chip8_display[DISPLAY_RES_X - 5] == 1);
    assert(chip8_display[DISPLAY_RES_X - 6] == 1);
    assert(chip8_display[DISPLAY_RES_X * 2 - 1] == 0);
    assert(chip8_display[DISPLAY_RES_X * 2 - 2] == 0);
    assert(chip8_display[DISPLAY_RES_X * 2 - 3] == 0);
    assert(chip8_display[DISPLAY_RES_X * 2 - 4] == 0);
    assert(chip8_display[DISPLAY_RES_X * 2 - 5] == 1);

    // 3. Ensure new edge is empty
    chip8_init();
    high_res_mode = 1;
    memset(chip8_display, 1, DISPLAY_RES_X * DISPLAY_RES_Y);
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFC;
    chip8_step(0, 0.0);
    assert(chip8_display[DISPLAY_RES_X - 1] == 0);
    assert(chip8_display[DISPLAY_RES_X - 2] == 0);
    assert(chip8_display[DISPLAY_RES_X - 3] == 0);
    assert(chip8_display[DISPLAY_RES_X - 4] == 0);
    assert(chip8_display[DISPLAY_RES_X * 2 - 1] == 0);
    assert(chip8_display[DISPLAY_RES_X * 2 - 2] == 0);
    assert(chip8_display[DISPLAY_RES_X * 2 - 3] == 0);
    assert(chip8_display[DISPLAY_RES_X * 2 - 4] == 0);

    printf("[PASS] test_00FC_high_res\n");
}

// Test: Exit interpreter
void test_00FD(void) {
    chip8_init();
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFD;
    chip8_step(0, 0.0);
    assert(chip8_exit_flag == 1);

    printf("[PASS] test_00FD\n");
}

// Test: Switch to low res mode
void test_00FE(void) {
    // 1. low res to low res
    chip8_init();
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFE;
    chip8_step(0, 0.0);
    assert(high_res_mode == 0);

    // 2. high res to low res
    chip8_init();
    high_res_mode = 1;
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFE;
    chip8_step(0, 0.0);
    assert(high_res_mode == 0);

    // 3. high res to low res. Ensure display buffer is not cleared
    chip8_init();
    high_res_mode = 1;
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFE;
    chip8_display[30] = 0x11;
    chip8_step(0, 0.0);
    assert(high_res_mode == 0);
    assert(chip8_display[30] == 0x11);

    printf("[PASS] test_00FE\n");
}

// Test: Switch to high res mode
void test_00FF(void) {
    // 1. high res to high res
    chip8_init();
    high_res_mode = 1;
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFF;
    chip8_step(0, 0.0);
    assert(high_res_mode == 1);

    // 2. low res to high res
    chip8_init();
    high_res_mode = 1;
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFF;
    chip8_step(0, 0.0);
    assert(high_res_mode == 1);

    // 3. low res to high res. Ensure display buffer is not cleared
    chip8_init();
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xFF;
    chip8_display[20] = 0x12;
    chip8_step(0, 0.0);
    assert(high_res_mode == 1);
    assert(chip8_display[20] == 0x12);

    printf("[PASS] test_00FF\n");
}

// Test: Dump VX register values (up to and including V7)
void test_FX75(void) {
    FILE *f;
    uint8_t file_bytes;
    uint8_t read_byte;

    // 1. Write one register (V0)
    chip8_init();
    V[0x0] = 5;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x75;
    chip8_step(0, 0.0);
    f = fopen(SUPER_CHIP_RPL_FILE, "rb");
    assert(f);
    
    // Determine file size
    fseek(f, 0, SEEK_END);
    file_bytes = ftell(f);
    fseek(f, 0, SEEK_SET);
    assert(file_bytes == 1);

    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 5);
    fclose(f);
    
    // 2. Write four registers (and implicitly clear the file)
    chip8_init();
    V[0x0] = 6;
    V[0x1] = 7;
    V[0x2] = 8;
    V[0x3] = 9;
    V[0x4] = 0xA;
    V[0x5] = 0xB;  // ignored
    memory[PROG_START_ADDR]     = 0xF4;
    memory[PROG_START_ADDR + 1] = 0x75;
    chip8_step(0, 0.0);
    f = fopen(SUPER_CHIP_RPL_FILE, "rb");
    assert(f);
    
    // Determine file size
    fseek(f, 0, SEEK_END);
    file_bytes = ftell(f);
    fseek(f, 0, SEEK_SET);
    assert(file_bytes == 5);

    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 6);
    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 7);
    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 8);
    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 9);
    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 0xA);
    fclose(f);
    
    // 3. Limit writing to V7 (and implicitly clear the file)
    chip8_init();
    V[0x0] = 6;
    V[0x1] = 7;
    V[0x2] = 8;
    V[0x3] = 9;
    V[0x4] = 0xA;
    V[0x5] = 0xB;
    V[0x6] = 0xC;
    V[0x7] = 0xD;
    V[0x8] = 0xE;  // ignored
    memory[PROG_START_ADDR]     = 0xF8;  // 8 > 7 (limit)
    memory[PROG_START_ADDR + 1] = 0x75;
    chip8_step(0, 0.0);
    f = fopen(SUPER_CHIP_RPL_FILE, "rb");
    assert(f);
    
    // Determine file size
    fseek(f, 0, SEEK_END);
    file_bytes = ftell(f);
    fseek(f, 0, SEEK_SET);
    assert(file_bytes == 8);

    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 6);
    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 7);
    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 8);
    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 9);
    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 0xA);
    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 0xB);
    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 0xC);
    assert(fread(&read_byte, sizeof(uint8_t), 1, f) == 1);
    assert(read_byte == 0xD);
    fclose(f);

    printf("[PASS] test_FX75\n");
}

// Test: Load register values (up to and including V7)
void test_FX85(void) {
    FILE *f;
    uint8_t byte_to_write;

    // 1. Load one register (V0)
    f = fopen(SUPER_CHIP_RPL_FILE, "w");
    byte_to_write = 100;
    fwrite(&byte_to_write, sizeof(uint8_t), 1, f);
    fclose(f);

    chip8_init();
    V[0x0] = 5;
    V[0x1] = 5;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x85;
    chip8_step(0, 0.0);
    assert(V[0x0] == 100);
    assert(V[0x1] == 5);  // unchanged
    
    // 2. Load four registers
    f = fopen(SUPER_CHIP_RPL_FILE, "w");
    byte_to_write = 0;
    fwrite(&byte_to_write, sizeof(uint8_t), 1, f);
    byte_to_write = 1;
    fwrite(&byte_to_write, sizeof(uint8_t), 1, f);
    byte_to_write = 2;
    fwrite(&byte_to_write, sizeof(uint8_t), 1, f);
    byte_to_write = 3;
    fwrite(&byte_to_write, sizeof(uint8_t), 1, f);
    fclose(f);

    chip8_init();
    memory[PROG_START_ADDR]     = 0xF3;
    memory[PROG_START_ADDR + 1] = 0x85;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0);
    assert(V[0x1] == 1);
    assert(V[0x2] == 2);
    assert(V[0x3] == 3);
    
    // 3. Load less than present in file
    f = fopen(SUPER_CHIP_RPL_FILE, "w");
    byte_to_write = 10;
    fwrite(&byte_to_write, sizeof(uint8_t), 1, f);
    byte_to_write = 11;
    fwrite(&byte_to_write, sizeof(uint8_t), 1, f);
    byte_to_write = 12;
    fwrite(&byte_to_write, sizeof(uint8_t), 1, f);
    byte_to_write = 13;
    fwrite(&byte_to_write, sizeof(uint8_t), 1, f);
    byte_to_write = 14;
    fwrite(&byte_to_write, sizeof(uint8_t), 1, f);
    fclose(f);

    chip8_init();
    memory[PROG_START_ADDR]     = 0xF2;
    memory[PROG_START_ADDR + 1] = 0x85;
    chip8_step(0, 0.0);
    assert(V[0x0] == 10);
    assert(V[0x1] == 11);
    assert(V[0x2] == 12);
    assert(V[0x3] == 0);
    assert(V[0x4] == 0);

    printf("[PASS] test_FX85\n");
}

int main(void) {
    printf("* Running init test\n");
    test_super_chip_init();

    printf("\n* Running new/super-chip opcode tests\n");
    test_00CN_high_res();  // Scroll N pixels down/Move pixels n positions down
    test_00FB_high_res();  // Scroll 4 pixels right/Move pixels 4 positions to the right
    test_00FC_high_res();  // Scroll 4 pixels left/Move pixels 4 positions to the left
    test_00FD();  // Exit interpreter
    test_00FE();  // Switch to low res mode/disable high res mode
    test_00FF();  // Switch to high res mode/enable high res mode
    // test_DXY0();  // TODO: Understand 16x16 sprite usage
    test_FX75();  // Write/dump V0..VX (up to 7, inclusive) values to file
    test_FX85();  // Read/load V0..VX (up to 7, inclusive) values from file

    printf("\n* All SUPER-CHIP tests passed\n");
    return 0;
}