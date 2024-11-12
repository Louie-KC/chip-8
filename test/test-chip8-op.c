#include <stdio.h>
#include <assert.h>

#include "../src/chip8.c"

void test_chip8_init() {
    chip8_init();

    // Check memory content
    // Interpreter space: before fonts
    for (int i = 0; i < FONT_START_ADDR; i++) {
        assert(memory[i] == 0);
    }
    // Interpreter space: after fonts
    for (int i = FONT_START_ADDR + 80; i < PROG_START_ADDR; i++) {
        assert(memory[i] == 0);
    }
    // Program/ROM space
    for (int i = PROG_START_ADDR; i < TOTAL_MEMORY; i++) {
        assert(memory[i] == 0);
    }

    // Registers
    assert(V[0x0] == 0);
    assert(V[0x1] == 0);
    assert(V[0x2] == 0);
    assert(V[0x3] == 0);
    assert(V[0x4] == 0);
    assert(V[0x5] == 0);
    assert(V[0x6] == 0);
    assert(V[0x7] == 0);
    assert(V[0x8] == 0);
    assert(V[0x9] == 0);
    assert(V[0xA] == 0);
    assert(V[0xB] == 0);
    assert(V[0xC] == 0);
    assert(V[0xD] == 0);
    assert(V[0xE] == 0);
    assert(V[0xF] == 0);

    assert(pc == PROG_START_ADDR);
    assert(I == 0);

    // Stack
    for (int i = 0; i < STACK_SIZE; i++) {
        assert(stack[i] == 0);
    }
    assert(sp == 0);

    // Display
    assert(chip8_display_updated == 0);
    for (int i = 0; i < DISPLAY_RES_X * DISPLAY_RES_Y; i++) {
        assert(chip8_display[i] == 0);
    }

    printf("[PASS] test_chip8_init\n");
}

// Test: Clear display
void test_00E0() {
    // 1. Clear -> Clear
    chip8_init();
    memset(chip8_display, 0, DISPLAY_RES_X * DISPLAY_RES_Y);
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xE0;
    chip8_step();
    for (int i = 0; i < DISPLAY_RES_X * DISPLAY_RES_Y; i++) {
        assert(chip8_display[i] == 0);
    }
    assert(chip8_display_updated == 1);

    // 2. Some -> Clear
    chip8_init();
    memset(chip8_display, 0, DISPLAY_RES_X * DISPLAY_RES_Y);
    for (int i = 0; i < DISPLAY_RES_X * DISPLAY_RES_Y; i += 2) {
        chip8_display[i] = 1;
    }
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xE0;
    chip8_step();
    for (int i = 0; i < DISPLAY_RES_X * DISPLAY_RES_Y; i++) {
        assert(chip8_display[i] == 0);
    }
    assert(chip8_display_updated == 1);

    // 3. Full/all on -> Clear
    chip8_init();
    memset(chip8_display, 0, DISPLAY_RES_X * DISPLAY_RES_Y);
    for (int i = 0; i < DISPLAY_RES_X * DISPLAY_RES_Y; i++) {
        chip8_display[i] = 1;
    }
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xE0;
    chip8_step();
    for (int i = 0; i < DISPLAY_RES_X * DISPLAY_RES_Y; i++) {
        assert(chip8_display[i] == 0);
    }
    assert(chip8_display_updated == 1);

    printf("[PASS] test_00E0\n");
}

// Test: Jump to NNN
void test_1NNN() {
    // 1. Jump to current address/start address
    chip8_init();
    memory[PROG_START_ADDR]     = 0x12;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step();
    assert(pc == PROG_START_ADDR);

    // 2. Jump forward small (0x220)
    chip8_init();
    memory[PROG_START_ADDR]     = 0x12;
    memory[PROG_START_ADDR + 1] = 0x20;
    chip8_step();
    assert(pc == 0x220);

    // 3. Jump forward big (0x95b)
    chip8_init();
    memory[PROG_START_ADDR]     = 0x19;
    memory[PROG_START_ADDR + 1] = 0x5B;
    chip8_step();
    assert(pc == 0x95b);

    printf("[PASS] test_1NNN\n");
}

// Test: Set VX
void test_6XNN() {
    // 1. Set V0 to 0x00 (no change)
    chip8_init();
    memory[PROG_START_ADDR]     = 0x60;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step();
    assert(V[0x0] == 0x00);

    // 2. Set V0 to 0xFF
    chip8_init();
    memory[PROG_START_ADDR]     = 0x60;
    memory[PROG_START_ADDR + 1] = 0xFF;
    chip8_step();
    assert(V[0x0] == 0xFF);
    
    // 3. Set V1 to 0xFF
    chip8_init();
    memory[PROG_START_ADDR]     = 0x61;
    memory[PROG_START_ADDR + 1] = 0x5B;
    chip8_step();
    assert(V[0x1] == 0x5B);
    
    // 4. Set V9 to 0xAA
    chip8_init();
    memory[PROG_START_ADDR]     = 0x69;
    memory[PROG_START_ADDR + 1] = 0xAA;
    chip8_step();
    assert(V[0x9] == 0xAA);

    // 4. Set VB to 0xF4
    chip8_init();
    memory[PROG_START_ADDR]     = 0x6B;
    memory[PROG_START_ADDR + 1] = 0xF4;
    chip8_step();
    assert(V[0xB] == 0xF4);

    // 5. Set VF to 0x02
    chip8_init();
    memory[PROG_START_ADDR]     = 0x6F;
    memory[PROG_START_ADDR + 1] = 0x02;
    chip8_step();
    assert(V[0xF] == 0x02);

    // 6. Set VA to 0x50 then 0x01
    chip8_init();
    memory[PROG_START_ADDR]     = 0x6A;
    memory[PROG_START_ADDR + 1] = 0x50;
    memory[PROG_START_ADDR + 2] = 0x6A;
    memory[PROG_START_ADDR + 3] = 0x01;
    chip8_step();
    assert(V[0xA] == 0x50);
    chip8_step();
    assert(V[0xA] == 0x01);

    printf("[PASS] test_6XNN\n");
}

// Test: Add NN to X (no carry flag)
void test_7XNN() {
    // 1. Add 1 to untouched register (0)
    chip8_init();
    memory[PROG_START_ADDR]     = 0x70;
    memory[PROG_START_ADDR + 1] = 0x01;
    chip8_step();
    assert(V[0x0] == 0x01);

    // 2. Add 0x10 to untouched register (0)
    chip8_init();
    memory[PROG_START_ADDR]     = 0x71;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step();
    assert(V[0x1] == 0x10);

    // 3. Add 0x20 then 0x01
    chip8_init();
    memory[PROG_START_ADDR]     = 0x72;
    memory[PROG_START_ADDR + 1] = 0x20;
    memory[PROG_START_ADDR + 2] = 0x72;
    memory[PROG_START_ADDR + 3] = 0x01;
    chip8_step();
    assert(V[0x2] == 0x20);
    chip8_step();
    assert(V[0x2] == 0x21);

    // 4. Add 0xDF then 0x20
    chip8_init();
    memory[PROG_START_ADDR]     = 0x70;
    memory[PROG_START_ADDR + 1] = 0xDF;
    memory[PROG_START_ADDR + 2] = 0x70;
    memory[PROG_START_ADDR + 3] = 0x20;
    chip8_step();
    assert(V[0x0] == 0xDF);
    chip8_step();
    assert(V[0x0] == 0xFF);

    // 5. Overflow. Add 0xFF then 0x01
    chip8_init();
    memory[PROG_START_ADDR]     = 0x70;
    memory[PROG_START_ADDR + 1] = 0xFF;
    memory[PROG_START_ADDR + 2] = 0x70;
    memory[PROG_START_ADDR + 3] = 0x01;
    chip8_step();
    assert(V[0x0] == 0xFF);
    chip8_step();
    assert(V[0x0] == 0x00);

    printf("[PASS] test_7XNN\n");
}

// Test: Set index register
void test_ANNN() {
    // 1. Set to 0 (no change)
    chip8_init();
    memory[PROG_START_ADDR]     = 0xA0;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step();
    assert(I == 0x0000);
    
    // 2. Set to 0x00A
    chip8_init();
    memory[PROG_START_ADDR]     = 0xA0;
    memory[PROG_START_ADDR + 1] = 0x0A;
    chip8_step();
    assert(I == 0x000A);

    // 3. Set to 0x100
    chip8_init();
    memory[PROG_START_ADDR]     = 0xA1;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step();
    assert(I == 0x0100);

    printf("[PASS] test_ANNN\n");
}

int main(void) {
    printf("* Beginning chip-8 init test\n");
    test_chip8_init();

    printf("\n* Beginning chip-8 opcode tests\n");
    test_00E0();  // Clear screen
    test_1NNN();  // Jump
    test_6XNN();  // Set register
    test_7XNN();  // Add (no carry) to register
    test_ANNN();  // Set index

    printf("\n* All tests passed\n");
    return 0;
}