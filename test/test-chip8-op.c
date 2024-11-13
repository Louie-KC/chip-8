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

// Test: Return from subroutine
void test_00EE() {
    // 1. Simple return
    chip8_init();
    sp = 1;
    stack[0] = 0x400;
    memory[PROG_START_ADDR]     = 0x00;
    memory[PROG_START_ADDR + 1] = 0xEE;
    chip8_step();
    assert(pc == 0x400);
    assert(sp == 0);

    printf("[PASS] test_00EE\n");
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

// Test: Call subroutine
void test_2NNN() {
    // 1. Simple call
    chip8_init();
    memory[PROG_START_ADDR]     = 0x2A;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step();
    assert(pc == 0xA00);
    assert(sp == 1);
    assert(stack[0] == 0x202);

    // 2. Call to another call
    chip8_init();
    memory[PROG_START_ADDR]     = 0x24;
    memory[PROG_START_ADDR + 1] = 0x00;
    memory[0x0400] = 0x27;
    memory[0x0401] = 0xFF;
    chip8_step();
    assert(pc == 0x400);
    assert(sp == 1);
    assert(stack[0] == 0x202);
    chip8_step();
    assert(pc == 0x7FF);
    assert(sp == 2);
    assert(stack[0] == 0x202);
    assert(stack[1] == 0x402);

    printf("[PASS] test_2NNN\n");
}

// Test: if (VX == NN) skip 1 instruction
void test_3XNN() {
    // 1. true/skip: V0 == 0, where V0 = 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x30;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step();
    assert(pc == 0x204);

    // 2. false/no skip: V0 == 1 where V0 = 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x30;
    memory[PROG_START_ADDR + 1] = 0x01;
    chip8_step();
    assert(pc == 0x202);

    // 3. true/skip: VB == 0xFF, where VB = 0xFF
    chip8_init();
    V[0xB] = 0xFF;
    memory[PROG_START_ADDR]     = 0x3B;
    memory[PROG_START_ADDR + 1] = 0xFF;
    chip8_step();
    assert(pc == 0x204);

    printf("[PASS] test_3XNN\n");
}

// Test: if (VX != NN) skip 1 instruction
void test_4XNN() {
    // 2. true/skip: V0 != 1 where V0 = 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x40;
    memory[PROG_START_ADDR + 1] = 0x01;
    chip8_step();
    assert(pc == 0x204);

    // 1. false/no skip: V0 != 0, where V0 = 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x40;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step();
    assert(pc == 0x202);

    // 3. true/skip: VC != 0xA1, where VB = 0xDD
    chip8_init();
    V[0xC] = 0xDD;
    memory[PROG_START_ADDR]     = 0x4C;
    memory[PROG_START_ADDR + 1] = 0xA1;
    chip8_step();
    assert(pc == 0x204);

    printf("[PASS] test_4XNN\n");
}

// Test: if (VX == VY) skip 1 instruction
void test_5XY0() {
    // 1. Test on self. true/skip: V0 == V0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x50;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step();
    assert(pc == 0x204);

    // 2. true/skip: V0 == V1 where V0 & V1 = 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x50;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step();
    assert(pc == 0x204);

    // 3. false/no skip: V0 == V1 where V0 = 0 & V1 = 1
    chip8_init();
    V[0x1] = 1;
    memory[PROG_START_ADDR]     = 0x50;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step();
    assert(pc == 0x202);

    // 4. true/skip: V5 == V2 where V5 = 1 & V1 = 1
    chip8_init();
    V[0x2] = 1;
    V[0x5] = 1;
    memory[PROG_START_ADDR]     = 0x52;
    memory[PROG_START_ADDR + 1] = 0x50;
    chip8_step();
    assert(pc == 0x204);

    // 5. false/no skip: VA == VB where VA = 0D & VB = A1
    chip8_init();
    V[0xA] = 0x0D;
    V[0xB] = 0xA1;
    memory[PROG_START_ADDR]     = 0x5A;
    memory[PROG_START_ADDR + 1] = 0xB0;
    chip8_step();
    assert(pc == 0x202);

    printf("[PASS] test_5XY0\n");
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

// Test: Set VX = VY
void test_8XY0() {
    // 1. V0 = V1 where both are 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step();
    assert(V[0x0] == V[0x1]);

    // 2. V0 = V1 where VY = 5
    chip8_init();
    V[0x1] = 5;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step();
    assert(V[0x0] == 5);
    assert(V[0x1] == 5);

    // 3. V5 = V2 where V5 = 4, V2 = 0x9A
    chip8_init();
    V[0x5] = 0x04;
    V[0x2] = 0x9A;
    memory[PROG_START_ADDR]     = 0x85;
    memory[PROG_START_ADDR + 1] = 0x20;
    chip8_step();
    assert(V[0x5] == 0x9A);
    assert(V[0x2] == 0x9A);

    printf("[PASS] test_8XY0\n");
}

// Test: VX = VX | VY
void test_8XY1() {
    // 1. V0 = 0 | 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x11;
    chip8_step();
    assert(V[0x0] == 0b00000000);
    assert(V[0x1] == 0b00000000);

    // 2. V0 = V0 (0b00000000) | V1 (0b10101010)
    chip8_init();
    V[0x0] = 0b00000000;
    V[0x1] = 0b10101010;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x11;
    chip8_step();
    assert(V[0x0] == 0b10101010);
    assert(V[0x1] == 0b10101010);

    // 3. V0 = V0 (0b11110000) | V1 (0b10101010)
    chip8_init();
    V[0x0] = 0b11110000;
    V[0x1] = 0b10101010;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x11;
    chip8_step();
    assert(V[0x0] == 0b11111010);
    assert(V[0x1] == 0b10101010);
    
    // 4. V0 = V0 (0b11110000) | V1 (0b00110000)
    chip8_init();
    V[0x0] = 0b11110000;
    V[0x1] = 0b00110000;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x11;
    chip8_step();
    assert(V[0x0] == 0b11110000);
    assert(V[0x1] == 0b00110000);
    
    // 5. V9 = V9 (0b10101010) | VA (0b01010101)
    chip8_init();
    V[0x9] = 0b10101010;
    V[0xA] = 0b01010101;
    memory[PROG_START_ADDR]     = 0x89;
    memory[PROG_START_ADDR + 1] = 0xA1;
    chip8_step();
    assert(V[0x9] == 0b11111111);
    assert(V[0xA] == 0b01010101);

    printf("[PASS] test_8XY1\n");
}

// Test: VX = VX & VY
void test_8XY2() {
    // 1. V0 = V0 (0b00000000) & V1 (0b10101010)
    chip8_init();
    V[0x0] = 0b00000000;
    V[0x1] = 0b10101010;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x12;
    chip8_step();
    assert(V[0x0] == 0b00000000);
    assert(V[0x1] == 0b10101010);

    // 2. V0 = V0 (0b11000011) & V1 (0b10101010)
    chip8_init();
    V[0x0] = 0b11000011;
    V[0x1] = 0b10101010;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x12;
    chip8_step();
    assert(V[0x0] == 0b10000010);
    assert(V[0x1] == 0b10101010);

    // 3. V4 = V4 (0b11111111) & V4 (0b01010101)
    chip8_init();
    V[0x4] = 0b11111111;
    V[0x5] = 0b01010101;
    memory[PROG_START_ADDR]     = 0x84;
    memory[PROG_START_ADDR + 1] = 0x52;
    chip8_step();
    assert(V[0x4] == 0b01010101);
    assert(V[0x5] == 0b01010101);

    printf("[PASS] test_8XY2\n");
}

// Test: VX = VX ^ VY
void test_8XY3() {
    // 1. V0 = V0 (0b00001111) ^ V1 (0b10101010)
    chip8_init();
    V[0x0] = 0b00001111;
    V[0x1] = 0b10101010;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x13;
    chip8_step();
    assert(V[0x0] == 0b10100101);
    assert(V[0x1] == 0b10101010);
    
    // 2. V2 = V2 (0b10101010) ^ V3 (0b10101010)
    chip8_init();
    V[0x2] = 0b10101010;
    V[0x3] = 0b10101010;
    memory[PROG_START_ADDR]     = 0x82;
    memory[PROG_START_ADDR + 1] = 0x33;
    chip8_step();
    assert(V[0x2] == 0b00000000);
    assert(V[0x3] == 0b10101010);

    printf("[PASS] test_8XY3\n");
}

// Test: VX = VX + VY (with carry flag). VF = 1 on overflow.
void test_8XY4() {
    // 1. V0 = V0 (0) + V1 (0)
    chip8_init();
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x14;
    chip8_step();
    assert(V[0x0] == 0);
    assert(V[0x1] == 0);
    assert(V[0xF] == 0);  // no overflow

    // 2. V0 = V0 (0xCD) + V1 (0x12)
    chip8_init();
    V[0x0] = 0xCD;
    V[0x1] = 0x12;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x14;
    chip8_step();
    assert(V[0x0] == 0xDF);
    assert(V[0x1] == 0x12);
    assert(V[0xF] == 0);  // no overflow

    // 3. V0 = V0 (0xFF) + V1 (0x01)
    chip8_init();
    V[0x0] = 0xFF;
    V[0x1] = 0x01;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x14;
    chip8_step();
    assert(V[0x0] == 0x00);
    assert(V[0x1] == 0x01);
    assert(V[0xF] == 1);  // overflow
    
    // 4. V7 = V7 (0x33) + V7 (0x33)
    chip8_init();
    V[0x7] = 0x33;
    memory[PROG_START_ADDR]     = 0x87;
    memory[PROG_START_ADDR + 1] = 0x74;
    chip8_step();
    assert(V[0x7] == 0x66);
    assert(V[0x4] == 0);  // no overflow
    
    // 5. V7 = V7 (0x34) + VE (0x5A)
    chip8_init();
    V[0x7] = 0x33;
    V[0xE] = 0xDA;
    memory[PROG_START_ADDR]     = 0x87;
    memory[PROG_START_ADDR + 1] = 0xE4;
    chip8_step();
    assert(V[0x7] == 0x0D);
    assert(V[0xE] == 0xDA);
    assert(V[0xF] == 1);  // overflow

    printf("[PASS] test_8XY4\n");
}

// Test: VX = VX - VY (with carry flag). VF = 0 on underflow.
void test_8XY5() {
    // 1. V0 = V0 (0x05) - V1 (0x01)
    chip8_init();
    V[0x0] = 0x05;
    V[0x1] = 0x01;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x15;
    chip8_step();
    assert(V[0x0] == 0x04);
    assert(V[0x1] == 0x01);
    assert(V[0xF] == 1);  // no underflow
    
    // 2. V0 = V0 (0x05) - V1 (0x06)
    chip8_init();
    V[0x0] = 0x05;
    V[0x1] = 0x06;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x15;
    chip8_step();
    assert(V[0x0] == 0xFF);
    assert(V[0x1] == 0x06);
    assert(V[0xF] == 0);  // underflow

    printf("[PASS] test_8XY5\n");
}

// Test: Right shift 1. VX = VX >> 1. Ambiguous instruction
void test_8XY6() {
    // 1.
    chip8_init();
    V[0x0] = 0b10101010;
    V[0x1] = 0b00000000;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x16;
    chip8_step();
    assert(V[0x0] == 0b01010101);
    // assert(V[0x1] == 0b10101010);  // Ambiguous
    assert(V[0xF] == 0);  // 0 was shifted out of the register
    
    // 2.
    chip8_init();
    V[0x0] = 0b11111111;
    V[0x1] = 0b00000000;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x16;
    chip8_step();
    assert(V[0x0] == 0b01111111);
    // assert(V[0x1] == 0b11111111);  // Ambiguous
    assert(V[0xF] == 1);  // 1 was shifted out of the register

    printf("[PASS] test_8XY6\n");
}

// Test: VX = VY - VX (with carry flag)
void test_8XY7() {
    // 1. V0 = V1 (0x10) - V0 (0x01)
    chip8_init();
    V[0x0] = 0x01;
    V[0x1] = 0x10;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x17;
    chip8_step();
    assert(V[0x0] == 0x0F);
    assert(V[0x1] == 0x10);
    assert(V[0xF] == 1);  // no underflow
    
    // 2. V0 = V1 (0x05) - V0 (0x06)
    chip8_init();
    V[0x0] = 0x06;
    V[0x1] = 0x05;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x17;
    chip8_step();
    assert(V[0x0] == 0xFF);
    assert(V[0x1] == 0x05);
    assert(V[0xF] == 0);  // underflow

    printf("[PASS] test_8XY7\n");
}

// Test: Left shift 1. VX = VX << 1. Ambiguous instruction
void test_8XYE() {
    // 1.
    chip8_init();
    V[0x0] = 0b01010101;
    V[0x1] = 0b00000000;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x1E;
    chip8_step();
    assert(V[0x0] == 0b10101010);
    // assert(V[0x1] == 0b01010101);  // Ambiguous
    assert(V[0xF] == 0);  // 0 was shifted out of the register
    
    // 2.
    chip8_init();
    V[0x0] = 0b11111111;
    V[0x1] = 0b00000000;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x1E;
    chip8_step();
    assert(V[0x0] == 0b11111110);
    // assert(V[0x1] == 0b11111111);  // Ambiguous
    assert(V[0xF] == 1);  // 1 was shifted out of the register

    printf("[PASS] test_8XYE\n");
}

// Test: if (VX != VY) skip 1 instruction
void test_9XY0() {
    // 1. false/no skip: V0 != V1 where V0 & V1 = 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x90;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step();
    assert(pc == 0x202);

    // 2. true/skip: V0 != V1 where V0 = 0 & V1 = 1
    chip8_init();
    V[0x1] = 1;
    memory[PROG_START_ADDR]     = 0x90;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step();
    assert(pc == 0x204);

    // 3. false/no skip: V5 != V2 where V5 = 1 & V1 = 1
    chip8_init();
    V[0x2] = 1;
    V[0x5] = 1;
    memory[PROG_START_ADDR]     = 0x92;
    memory[PROG_START_ADDR + 1] = 0x50;
    chip8_step();
    assert(pc == 0x202);

    // 4. true/skip: VA != VB where VA = 0D & VB = A1
    chip8_init();
    V[0xA] = 0x0D;
    V[0xB] = 0xA1;
    memory[PROG_START_ADDR]     = 0x9A;
    memory[PROG_START_ADDR + 1] = 0xB0;
    chip8_step();
    assert(pc == 0x204);

    printf("[PASS] test_9XY0\n");
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
    test_00EE();  // Return from subroutine
    test_1NNN();  // Jump
    test_2NNN();  // Call subroutine
    test_3XNN();  // if VX == NN skip 1 instruction
    test_4XNN();  // if VX != NN skip 1 instruction
    test_5XY0();  // if VX == VY skip 1 instruction
    test_6XNN();  // Set register to immediate
    test_7XNN();  // Add (no carry) to register
    test_8XY0();  // Set register VX = VY
    test_8XY1();  // Binary OR
    test_8XY2();  // Binary AND
    test_8XY3();  // Logical XOR
    test_8XY4();  // Add VX = VX + VY
    test_8XY5();  // Subtract VX = VX - VY (with carry flag)
    test_8XY6();  // Right shift (ambiguous instruction)
    test_8XY7();  // Subtract VX = VY - VX (with carry flag)
    test_8XYE();  // Left shift (ambiguous instruction)
    test_9XY0();  // if VX != VY skip 1 instruction
    test_ANNN();  // Set index

    printf("\n* All tests passed\n");
    return 0;
}