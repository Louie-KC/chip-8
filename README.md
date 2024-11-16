# CHIP-8 emulator

An exercise in developing the "hello world" of emudev, CHIP-8 (a virtual machine).

## Demo
<img src="https://github.com/user-attachments/assets/46bb7219-3696-494b-9df0-607ae2875e73" alt="RPS-gif" width="400"/>
<img src="https://github.com/user-attachments/assets/96c49dc9-3b19-440e-badb-3b30ca659d11" alt="Octojam7-title-gif" width="400"/>

### How to run
```
# Compile by running `make` to invoke the `all` target.
make

# Run executable
./ch8 [ROM file path]
```

### Debugger
The executable can be built/compiled in a debug mode, enabling the user to step through the execution of a loaded ROM, and inspect the state and memory of the emulator.

<img src="https://github.com/user-attachments/assets/c048e729-5b08-4b6d-b2ec-2eac55366dd7" alt="debugger-demo-gif" width="900"/>

Compile debug mode executable.
```
# Compile the `debug` target
make debug

# Run debug mode executable
./ch8 [ROM file path]
```

Debugger controls:
```
s         - step 1 instruction.
s [n]     - step n instructions.
i         - print chip8 state.
m [a]     - print value in memory at address a (in hex).
m [a] [l] - print values in memory at address a, to a+l (a and l are in hex).
n         - print the next opcode/instruction to be executed.
h         - print this list/controls again.
q         - quit.
```

## Inputs
Starting with the '1' key below the F keys, a 4x4 grid is mapped to the CHIP-8 keypad. Scan codes are used for DVORAK layout compatibility.

## SDL2
Developed on an ARM Mac with SDL2 installed via Homebrew.

## Roms
* IBM.ch8 : https://github.com/loktar00/chip8/blob/master/roms/IBM%20Logo.ch8
* test_opcode.ch8 : https://github.com/corax89/chip8-test-rom/blob/master/test_opcode.ch8

## References
1. https://en.wikipedia.org/wiki/CHIP-8
2. https://tobiasvl.github.io/blog/write-a-chip-8-emulator/
3. https://blog.fredrb.com/2023/08/08/audio-programming-note-sdl/
