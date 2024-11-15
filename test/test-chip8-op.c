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
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
    assert(pc == PROG_START_ADDR);

    // 2. Jump forward small (0x220)
    chip8_init();
    memory[PROG_START_ADDR]     = 0x12;
    memory[PROG_START_ADDR + 1] = 0x20;
    chip8_step(0, 0.0);
    assert(pc == 0x220);

    // 3. Jump forward big (0x95b)
    chip8_init();
    memory[PROG_START_ADDR]     = 0x19;
    memory[PROG_START_ADDR + 1] = 0x5B;
    chip8_step(0, 0.0);
    assert(pc == 0x95b);

    printf("[PASS] test_1NNN\n");
}

// Test: Call subroutine
void test_2NNN() {
    // 1. Simple call
    chip8_init();
    memory[PROG_START_ADDR]     = 0x2A;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step(0, 0.0);
    assert(pc == 0xA00);
    assert(sp == 1);
    assert(stack[0] == 0x202);

    // 2. Call to another call
    chip8_init();
    memory[PROG_START_ADDR]     = 0x24;
    memory[PROG_START_ADDR + 1] = 0x00;
    memory[0x0400] = 0x27;
    memory[0x0401] = 0xFF;
    chip8_step(0, 0.0);
    assert(pc == 0x400);
    assert(sp == 1);
    assert(stack[0] == 0x202);
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
    assert(pc == 0x204);

    // 2. false/no skip: V0 == 1 where V0 = 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x30;
    memory[PROG_START_ADDR + 1] = 0x01;
    chip8_step(0, 0.0);
    assert(pc == 0x202);

    // 3. true/skip: VB == 0xFF, where VB = 0xFF
    chip8_init();
    V[0xB] = 0xFF;
    memory[PROG_START_ADDR]     = 0x3B;
    memory[PROG_START_ADDR + 1] = 0xFF;
    chip8_step(0, 0.0);
    assert(pc == 0x204);

    printf("[PASS] test_3XNN\n");
}

// Test: if (VX != NN) skip 1 instruction
void test_4XNN() {
    // 2. true/skip: V0 != 1 where V0 = 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x40;
    memory[PROG_START_ADDR + 1] = 0x01;
    chip8_step(0, 0.0);
    assert(pc == 0x204);

    // 1. false/no skip: V0 != 0, where V0 = 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x40;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step(0, 0.0);
    assert(pc == 0x202);

    // 3. true/skip: VC != 0xA1, where VB = 0xDD
    chip8_init();
    V[0xC] = 0xDD;
    memory[PROG_START_ADDR]     = 0x4C;
    memory[PROG_START_ADDR + 1] = 0xA1;
    chip8_step(0, 0.0);
    assert(pc == 0x204);

    printf("[PASS] test_4XNN\n");
}

// Test: if (VX == VY) skip 1 instruction
void test_5XY0() {
    // 1. Test on self. true/skip: V0 == V0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x50;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step(0, 0.0);
    assert(pc == 0x204);

    // 2. true/skip: V0 == V1 where V0 & V1 = 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x50;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step(0, 0.0);
    assert(pc == 0x204);

    // 3. false/no skip: V0 == V1 where V0 = 0 & V1 = 1
    chip8_init();
    V[0x1] = 1;
    memory[PROG_START_ADDR]     = 0x50;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step(0, 0.0);
    assert(pc == 0x202);

    // 4. true/skip: V5 == V2 where V5 = 1 & V1 = 1
    chip8_init();
    V[0x2] = 1;
    V[0x5] = 1;
    memory[PROG_START_ADDR]     = 0x52;
    memory[PROG_START_ADDR + 1] = 0x50;
    chip8_step(0, 0.0);
    assert(pc == 0x204);

    // 5. false/no skip: VA == VB where VA = 0D & VB = A1
    chip8_init();
    V[0xA] = 0x0D;
    V[0xB] = 0xA1;
    memory[PROG_START_ADDR]     = 0x5A;
    memory[PROG_START_ADDR + 1] = 0xB0;
    chip8_step(0, 0.0);
    assert(pc == 0x202);

    printf("[PASS] test_5XY0\n");
}

// Test: Set VX
void test_6XNN() {
    // 1. Set V0 to 0x00 (no change)
    chip8_init();
    memory[PROG_START_ADDR]     = 0x60;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0x00);

    // 2. Set V0 to 0xFF
    chip8_init();
    memory[PROG_START_ADDR]     = 0x60;
    memory[PROG_START_ADDR + 1] = 0xFF;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0xFF);
    
    // 3. Set V1 to 0xFF
    chip8_init();
    memory[PROG_START_ADDR]     = 0x61;
    memory[PROG_START_ADDR + 1] = 0x5B;
    chip8_step(0, 0.0);
    assert(V[0x1] == 0x5B);
    
    // 4. Set V9 to 0xAA
    chip8_init();
    memory[PROG_START_ADDR]     = 0x69;
    memory[PROG_START_ADDR + 1] = 0xAA;
    chip8_step(0, 0.0);
    assert(V[0x9] == 0xAA);

    // 4. Set VB to 0xF4
    chip8_init();
    memory[PROG_START_ADDR]     = 0x6B;
    memory[PROG_START_ADDR + 1] = 0xF4;
    chip8_step(0, 0.0);
    assert(V[0xB] == 0xF4);

    // 5. Set VF to 0x02
    chip8_init();
    memory[PROG_START_ADDR]     = 0x6F;
    memory[PROG_START_ADDR + 1] = 0x02;
    chip8_step(0, 0.0);
    assert(V[0xF] == 0x02);

    // 6. Set VA to 0x50 then 0x01
    chip8_init();
    memory[PROG_START_ADDR]     = 0x6A;
    memory[PROG_START_ADDR + 1] = 0x50;
    memory[PROG_START_ADDR + 2] = 0x6A;
    memory[PROG_START_ADDR + 3] = 0x01;
    chip8_step(0, 0.0);
    assert(V[0xA] == 0x50);
    chip8_step(0, 0.0);
    assert(V[0xA] == 0x01);

    printf("[PASS] test_6XNN\n");
}

// Test: Add NN to X (no carry flag)
void test_7XNN() {
    // 1. Add 1 to untouched register (0)
    chip8_init();
    memory[PROG_START_ADDR]     = 0x70;
    memory[PROG_START_ADDR + 1] = 0x01;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0x01);

    // 2. Add 0x10 to untouched register (0)
    chip8_init();
    memory[PROG_START_ADDR]     = 0x71;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step(0, 0.0);
    assert(V[0x1] == 0x10);

    // 3. Add 0x20 then 0x01
    chip8_init();
    memory[PROG_START_ADDR]     = 0x72;
    memory[PROG_START_ADDR + 1] = 0x20;
    memory[PROG_START_ADDR + 2] = 0x72;
    memory[PROG_START_ADDR + 3] = 0x01;
    chip8_step(0, 0.0);
    assert(V[0x2] == 0x20);
    chip8_step(0, 0.0);
    assert(V[0x2] == 0x21);

    // 4. Add 0xDF then 0x20
    chip8_init();
    memory[PROG_START_ADDR]     = 0x70;
    memory[PROG_START_ADDR + 1] = 0xDF;
    memory[PROG_START_ADDR + 2] = 0x70;
    memory[PROG_START_ADDR + 3] = 0x20;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0xDF);
    chip8_step(0, 0.0);
    assert(V[0x0] == 0xFF);

    // 5. Overflow. Add 0xFF then 0x01
    chip8_init();
    memory[PROG_START_ADDR]     = 0x70;
    memory[PROG_START_ADDR + 1] = 0xFF;
    memory[PROG_START_ADDR + 2] = 0x70;
    memory[PROG_START_ADDR + 3] = 0x01;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0xFF);
    chip8_step(0, 0.0);
    assert(V[0x0] == 0x00);

    printf("[PASS] test_7XNN\n");
}

// Test: Set VX = VY
void test_8XY0() {
    // 1. V0 = V1 where both are 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step(0, 0.0);
    assert(V[0x0] == V[0x1]);

    // 2. V0 = V1 where VY = 5
    chip8_init();
    V[0x1] = 5;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step(0, 0.0);
    assert(V[0x0] == 5);
    assert(V[0x1] == 5);

    // 3. V5 = V2 where V5 = 4, V2 = 0x9A
    chip8_init();
    V[0x5] = 0x04;
    V[0x2] = 0x9A;
    memory[PROG_START_ADDR]     = 0x85;
    memory[PROG_START_ADDR + 1] = 0x20;
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
    assert(V[0x0] == 0b00000000);
    assert(V[0x1] == 0b00000000);

    // 2. V0 = V0 (0b00000000) | V1 (0b10101010)
    chip8_init();
    V[0x0] = 0b00000000;
    V[0x1] = 0b10101010;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x11;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0b10101010);
    assert(V[0x1] == 0b10101010);

    // 3. V0 = V0 (0b11110000) | V1 (0b10101010)
    chip8_init();
    V[0x0] = 0b11110000;
    V[0x1] = 0b10101010;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x11;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0b11111010);
    assert(V[0x1] == 0b10101010);
    
    // 4. V0 = V0 (0b11110000) | V1 (0b00110000)
    chip8_init();
    V[0x0] = 0b11110000;
    V[0x1] = 0b00110000;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x11;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0b11110000);
    assert(V[0x1] == 0b00110000);
    
    // 5. V9 = V9 (0b10101010) | VA (0b01010101)
    chip8_init();
    V[0x9] = 0b10101010;
    V[0xA] = 0b01010101;
    memory[PROG_START_ADDR]     = 0x89;
    memory[PROG_START_ADDR + 1] = 0xA1;
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
    assert(V[0x0] == 0b00000000);
    assert(V[0x1] == 0b10101010);

    // 2. V0 = V0 (0b11000011) & V1 (0b10101010)
    chip8_init();
    V[0x0] = 0b11000011;
    V[0x1] = 0b10101010;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x12;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0b10000010);
    assert(V[0x1] == 0b10101010);

    // 3. V4 = V4 (0b11111111) & V4 (0b01010101)
    chip8_init();
    V[0x4] = 0b11111111;
    V[0x5] = 0b01010101;
    memory[PROG_START_ADDR]     = 0x84;
    memory[PROG_START_ADDR + 1] = 0x52;
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
    assert(V[0x0] == 0b10100101);
    assert(V[0x1] == 0b10101010);
    
    // 2. V2 = V2 (0b10101010) ^ V3 (0b10101010)
    chip8_init();
    V[0x2] = 0b10101010;
    V[0x3] = 0b10101010;
    memory[PROG_START_ADDR]     = 0x82;
    memory[PROG_START_ADDR + 1] = 0x33;
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
    assert(V[0x0] == 0);
    assert(V[0x1] == 0);
    assert(V[0xF] == 0);  // no overflow

    // 2. V0 = V0 (0xCD) + V1 (0x12)
    chip8_init();
    V[0x0] = 0xCD;
    V[0x1] = 0x12;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x14;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0xDF);
    assert(V[0x1] == 0x12);
    assert(V[0xF] == 0);  // no overflow

    // 3. V0 = V0 (0xFF) + V1 (0x01)
    chip8_init();
    V[0x0] = 0xFF;
    V[0x1] = 0x01;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x14;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0x00);
    assert(V[0x1] == 0x01);
    assert(V[0xF] == 1);  // overflow
    
    // 4. V7 = V7 (0x33) + V7 (0x33)
    chip8_init();
    V[0x7] = 0x33;
    memory[PROG_START_ADDR]     = 0x87;
    memory[PROG_START_ADDR + 1] = 0x74;
    chip8_step(0, 0.0);
    assert(V[0x7] == 0x66);
    assert(V[0x4] == 0);  // no overflow
    
    // 5. V7 = V7 (0x34) + VE (0x5A)
    chip8_init();
    V[0x7] = 0x33;
    V[0xE] = 0xDA;
    memory[PROG_START_ADDR]     = 0x87;
    memory[PROG_START_ADDR + 1] = 0xE4;
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
    assert(V[0x0] == 0x04);
    assert(V[0x1] == 0x01);
    assert(V[0xF] == 1);  // no underflow
    
    // 2. V0 = V0 (0x05) - V1 (0x06)
    chip8_init();
    V[0x0] = 0x05;
    V[0x1] = 0x06;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x15;
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
    assert(V[0x0] == 0b01010101);
    // assert(V[0x1] == 0b10101010);  // Ambiguous
    assert(V[0xF] == 0);  // 0 was shifted out of the register
    
    // 2.
    chip8_init();
    V[0x0] = 0b11111111;
    V[0x1] = 0b00000000;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x16;
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
    assert(V[0x0] == 0x0F);
    assert(V[0x1] == 0x10);
    assert(V[0xF] == 1);  // no underflow
    
    // 2. V0 = V1 (0x05) - V0 (0x06)
    chip8_init();
    V[0x0] = 0x06;
    V[0x1] = 0x05;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x17;
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
    assert(V[0x0] == 0b10101010);
    // assert(V[0x1] == 0b01010101);  // Ambiguous
    assert(V[0xF] == 0);  // 0 was shifted out of the register
    
    // 2.
    chip8_init();
    V[0x0] = 0b11111111;
    V[0x1] = 0b00000000;
    memory[PROG_START_ADDR]     = 0x80;
    memory[PROG_START_ADDR + 1] = 0x1E;
    chip8_step(0, 0.0);
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
    chip8_step(0, 0.0);
    assert(pc == 0x202);

    // 2. true/skip: V0 != V1 where V0 = 0 & V1 = 1
    chip8_init();
    V[0x1] = 1;
    memory[PROG_START_ADDR]     = 0x90;
    memory[PROG_START_ADDR + 1] = 0x10;
    chip8_step(0, 0.0);
    assert(pc == 0x204);

    // 3. false/no skip: V5 != V2 where V5 = 1 & V1 = 1
    chip8_init();
    V[0x2] = 1;
    V[0x5] = 1;
    memory[PROG_START_ADDR]     = 0x92;
    memory[PROG_START_ADDR + 1] = 0x50;
    chip8_step(0, 0.0);
    assert(pc == 0x202);

    // 4. true/skip: VA != VB where VA = 0D & VB = A1
    chip8_init();
    V[0xA] = 0x0D;
    V[0xB] = 0xA1;
    memory[PROG_START_ADDR]     = 0x9A;
    memory[PROG_START_ADDR + 1] = 0xB0;
    chip8_step(0, 0.0);
    assert(pc == 0x204);

    printf("[PASS] test_9XY0\n");
}

// Test: Set index register
void test_ANNN() {
    // 1. Set to 0 (no change)
    chip8_init();
    memory[PROG_START_ADDR]     = 0xA0;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step(0, 0.0);
    assert(I == 0x0000);
    
    // 2. Set to 0x00A
    chip8_init();
    memory[PROG_START_ADDR]     = 0xA0;
    memory[PROG_START_ADDR + 1] = 0x0A;
    chip8_step(0, 0.0);
    assert(I == 0x000A);

    // 3. Set to 0x100
    chip8_init();
    memory[PROG_START_ADDR]     = 0xA1;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step(0, 0.0);
    assert(I == 0x0100);

    printf("[PASS] test_ANNN\n");
}

// Test: Jump with offset
void test_BNNN() {
    // 1. V0 = 0
    chip8_init();
    memory[PROG_START_ADDR]     = 0xB3;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step(0, 0.0);
    assert(pc == 0x300);

    // 1. V0 = 5
    chip8_init();
    V[0] = 5;
    memory[PROG_START_ADDR]     = 0xB3;
    memory[PROG_START_ADDR + 1] = 0x00;
    chip8_step(0, 0.0);
    assert(pc == 0x305);

    printf("[PASS] test_BNNN\n");
}

// Test: Skip if key is down/pressed.
void test_EX9E() {
    // 1. No input
    chip8_init();
    V[0x0] = 0;  // skip trigger key
    memory[PROG_START_ADDR]     = 0xE0;
    memory[PROG_START_ADDR + 1] = 0x9E;
    chip8_step(0, 0.0);
    assert(pc == PROG_START_ADDR + 2);

    // 2. 0 key down and is skip key
    chip8_init();
    V[0x0] = 0;
    memory[PROG_START_ADDR]     = 0xE0;
    memory[PROG_START_ADDR + 1] = 0x9E;
    chip8_step(0x10, 0.0);
    assert(pc == PROG_START_ADDR + 4);

    // 3. Non-skip key down
    chip8_init();
    V[0x0] = 0xB;
    memory[PROG_START_ADDR]     = 0xE0;
    memory[PROG_START_ADDR + 1] = 0x9E;
    chip8_step(0x15, 0.0);
    assert(pc == PROG_START_ADDR + 2);
    
    // 4. Different register holding skip key
    chip8_init();
    V[0xA] = 0xA;
    memory[PROG_START_ADDR]     = 0xEA;
    memory[PROG_START_ADDR + 1] = 0x9E;
    chip8_step(0x1A, 0.0);
    assert(pc == PROG_START_ADDR + 4);

    printf("[PASS] test_EX9E\n");
}

// Test: Skip if key is up/not pressed.
void test_EXA1() {
    // 1. No input
    chip8_init();
    V[0x0] = 0x0;
    memory[PROG_START_ADDR]     = 0xE0;
    memory[PROG_START_ADDR + 1] = 0xA1;
    chip8_step(0, 0.0);
    assert(pc == PROG_START_ADDR + 4);
    
    // 2. 0 key down and is no skip key
    chip8_init();
    V[0x0] = 0x0;
    memory[PROG_START_ADDR]     = 0xE0;
    memory[PROG_START_ADDR + 1] = 0xA1;
    chip8_step(0x10, 0.0);
    assert(pc == PROG_START_ADDR + 2);

    // 3. Non-0 no skip key
    chip8_init();
    V[0x0] = 0x4;
    memory[PROG_START_ADDR]     = 0xE0;
    memory[PROG_START_ADDR + 1] = 0xA1;
    chip8_step(0x14, 0.0);
    assert(pc == PROG_START_ADDR + 2);

    // 4. Different register
    chip8_init();
    V[0xC] = 0xA;
    memory[PROG_START_ADDR]     = 0xEC;
    memory[PROG_START_ADDR + 1] = 0xA1;
    chip8_step(0x18, 0.0);
    assert(pc == PROG_START_ADDR + 4);

    printf("[PASS] test_EXA1\n");
}

// Test: Get delay timer value
void test_FX07() {
    chip8_init();
    V[0x0] = 0xFF;
    delay_timer = 0;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x07;
    chip8_step(0, 0.0);
    assert(V[0x0] == 0);

    chip8_init();
    V[0x5] = 0xFF;
    delay_timer = 20;
    memory[PROG_START_ADDR]     = 0xF5;
    memory[PROG_START_ADDR + 1] = 0x07;
    chip8_step(0, 0.0);
    assert(V[0x5] == 20);

    printf("[PASS] test_FX07\n");
}

// Test: Set delay timer
void test_FX15() {
    chip8_init();
    V[0x0] = 100;
    delay_timer = 0;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x15;
    chip8_step(0, 0.0);
    assert(delay_timer == 100);

    chip8_init();
    V[0x5] = 50;
    delay_timer = 20;
    memory[PROG_START_ADDR]     = 0xF5;
    memory[PROG_START_ADDR + 1] = 0x15;
    chip8_step(0, 0.0);
    assert(delay_timer == 50);

    printf("[PASS] test_FX15\n");
}

// Test: Set sound timer
void test_FX18() {
    chip8_init();
    V[0x0] = 0;
    sound_timer = 100;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x18;
    chip8_step(0, 0.0);
    assert(sound_timer == 0);

    chip8_init();
    V[0x5] = 40;
    sound_timer = 20;
    memory[PROG_START_ADDR]     = 0xF5;
    memory[PROG_START_ADDR + 1] = 0x18;
    chip8_step(0, 0.0);
    assert(sound_timer == 40);

    printf("[PASS] test_FX18\n");
}

// Test: Get key. Block if no key
void test_FX0A() {
    // 1. No key
    chip8_init();
    V[0x0] = 0xFF;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x0A;
    chip8_step(0x00, 0.0);
    assert(pc == PROG_START_ADDR);
    assert(V[0x0] == 0xFF);

    // 2. 0 key
    chip8_init();
    V[0x0] = 0xFF;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x0A;
    chip8_step(0x10, 0.0);
    assert(pc == PROG_START_ADDR + 2);
    assert(V[0x0] == 0x00);

    // 3. B key
    chip8_init();
    V[0x0] = 0xFF;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x0A;
    chip8_step(0x1B, 0.0);
    assert(pc == PROG_START_ADDR + 2);
    assert(V[0x0] == 0x0B);

    // 4. B key into V4
    chip8_init();
    V[0x4] = 0xFF;
    memory[PROG_START_ADDR]     = 0xF4;
    memory[PROG_START_ADDR + 1] = 0x0A;
    chip8_step(0x1B, 0.0);
    assert(pc == PROG_START_ADDR + 2);
    assert(V[0x4] == 0x0B);

    printf("[PASS] test_FX0A\n");
}

// Test: Add to index I
void test_FX1E() {
    chip8_init();
    V[0x0] = 5;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x1E;
    chip8_step(0, 0.0);
    assert(I == 5);

    chip8_init();
    V[0x4] = 9;
    memory[PROG_START_ADDR]     = 0xF4;
    memory[PROG_START_ADDR + 1] = 0x1E;
    chip8_step(0, 0.0);
    assert(I == 9);

    printf("[PASS] test_FX1E\n");
}

// Test: Set address of font for VX value to I
void test_FX29() {
    // 1. Set index to the first sprite 0
    chip8_init();
    V[0x0] = 0;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x29;
    chip8_step(0, 0.0);
    assert(I == FONT_START_ADDR);

    // 2. Set index to the 8th sprite 8
    chip8_init();
    V[0x0] = 0x8;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x29;
    chip8_step(0, 0.0);
    assert(I == FONT_START_ADDR + 0x28);

    // 3. Set index to the last sprite F
    chip8_init();
    V[0x0] = 0xF;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x29;
    chip8_step(0, 0.0);
    assert(I == FONT_START_ADDR + 0x4B);

    printf("[PASS] test_FX29\n");
}

// Test: Write decimal digits of the value in VX at I
void test_FX33() {
    // 1. All 0s
    chip8_init();
    V[0x0] = 0;
    I = 0;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x33;
    chip8_step(0, 0.0);
    assert(I == 0);
    assert(memory[0] == 0);
    assert(memory[1] == 0);
    assert(memory[2] == 0);

    // 2. Write 246
    chip8_init();
    V[0x0] = 246;
    I = 0;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x33;
    chip8_step(0, 0.0);
    assert(I == 0);
    assert(memory[0] == 2);
    assert(memory[1] == 4);
    assert(memory[2] == 6);

    // 3. Write 185 from V2 
    chip8_init();
    V[0x2] = 185;
    I = 0x400;
    memory[PROG_START_ADDR]     = 0xF2;
    memory[PROG_START_ADDR + 1] = 0x33;
    chip8_step(0, 0.0);
    assert(I == 0x400);
    assert(memory[0x400] == 1);
    assert(memory[0x401] == 8);
    assert(memory[0x402] == 5);

    printf("[PASS] test_FX33\n");
}

// Test: Dump first X + 1 register values to memory at I
void test_FX55() {
    // 1. Write up to the 0th index
    chip8_init();
    V[0x0] = 44;
    V[0x1] = 55;
    V[0x2] = 66;
    I = 0x300;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x55;
    chip8_step(0, 0.0);
    assert(I == 0x300);  // ambiguous
    assert(memory[0x300] == 44);
    assert(memory[0x301] == 0);
    assert(memory[0x302] == 0);

    // 2. Write up to the 2nd index
    chip8_init();
    V[0x0] = 44;
    V[0x1] = 55;
    V[0x2] = 66;
    I = 0x300;
    memory[PROG_START_ADDR]     = 0xF2;
    memory[PROG_START_ADDR + 1] = 0x55;
    chip8_step(0, 0.0);
    assert(I == 0x300);  // ambiguous
    assert(memory[0x300] == 44);
    assert(memory[0x301] == 55);
    assert(memory[0x302] == 66);
    
    // 3. Write up to the Fth index
    chip8_init();
    V[0x0] = 44;
    V[0x1] = 55;
    V[0x2] = 66;
    V[0x3] = 77;
    V[0xC] = 91;
    V[0xF] = 123;
    I = 0x300;
    memory[PROG_START_ADDR]     = 0xFF;
    memory[PROG_START_ADDR + 1] = 0x55;
    chip8_step(0, 0.0);
    assert(I == 0x300);  // ambiguous
    assert(memory[0x300] == 44);
    assert(memory[0x301] == 55);
    assert(memory[0x302] == 66);
    assert(memory[0x303] == 77);
    assert(memory[0x304] == 0);
    assert(memory[0x305] == 0);
    assert(memory[0x306] == 0);
    assert(memory[0x307] == 0);
    assert(memory[0x308] == 0);
    assert(memory[0x309] == 0);
    assert(memory[0x30A] == 0);
    assert(memory[0x30B] == 0);
    assert(memory[0x30C] == 91);
    assert(memory[0x30D] == 0);
    assert(memory[0x30E] == 0);
    assert(memory[0x30F] == 123);

    printf("[PASS] test_FX55\n");
}

// Test: Load first X + 1 values from memory at address I to registers
void test_FX65() {
    // 1. Load up to the 0th register
    chip8_init();
    I = 0x300;
    memory[PROG_START_ADDR]     = 0xF0;
    memory[PROG_START_ADDR + 1] = 0x65;
    memory[0x300] = 12;
    memory[0x301] = 42;
    memory[0x302] = 55;
    chip8_step(0, 0.0);
    assert(I == 0x300);  // ambiguous
    assert(V[0x0] == 12);
    assert(V[0x1] == 0);
    assert(V[0x2] == 0);

    // 2. Load up to the 2nd register
    chip8_init();
    I = 0x300;
    memory[PROG_START_ADDR]     = 0xF2;
    memory[PROG_START_ADDR + 1] = 0x65;
    memory[0x300] = 12;
    memory[0x301] = 42;
    memory[0x302] = 55;
    chip8_step(0, 0.0);
    assert(I == 0x300);  // ambiguous
    assert(V[0x0] == 12);
    assert(V[0x1] == 42);
    assert(V[0x2] == 55);

    // 3. Load up to the 7th register
    chip8_init();
    I = 0x300;
    memory[PROG_START_ADDR]     = 0xF7;
    memory[PROG_START_ADDR + 1] = 0x65;
    memory[0x300] = 12;
    memory[0x301] = 42;
    memory[0x302] = 55;
    memory[0x303] = 11;
    memory[0x304] = 99;
    memory[0x305] = 104;
    memory[0x306] = 250;
    memory[0x307] = 33;
    memory[0x308] = 255;
    chip8_step(0, 0.0);
    assert(I == 0x300);  // ambiguous
    assert(V[0x0] == 12);
    assert(V[0x1] == 42);
    assert(V[0x2] == 55);
    assert(V[0x3] == 11);
    assert(V[0x4] == 99);
    assert(V[0x5] == 104);
    assert(V[0x6] == 250);
    assert(V[0x7] == 33);
    assert(V[0x8] == 0);

    printf("[PASS] test_FX65\n");
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
    test_BNNN();  // Jump with offset (ambiguous)
    test_EX9E();  // if key is down skip 1 instruction
    test_EXA1();  // if key is up skip 1 instruction
    test_FX07();  // Get delay timer value
    test_FX15();  // Set delay timer value
    test_FX18();  // Set sound timer value
    test_FX0A();  // Get key.
    test_FX1E();  // Add to index
    test_FX29();  // Set index to font described by VX
    test_FX33();  // Write the 3 decimal digits of VX to I
    test_FX55();  // Dump registers to memory at I
    test_FX65();  // Load values to registers from memory at I

    printf("\n* All tests passed\n");
    return 0;
}