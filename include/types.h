#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL2/SDL.h>

typedef enum {
    KERNEL_MODE,
    USER_MODE
} CPU_MODE;

typedef struct {
    // 5KB memory
    uint8_t memory[5120];

    // 24 byte register V0-23
    // ***UPDATE*** Reverted to 16 byte because of the way kernel opcodes were implemented
    uint8_t V[16];

    // I register - 16 bits (holds memory addresses)
    uint16_t I;

    // Program Counter (PC) -- Holds addresses for advance and recall
    // Increment in 2's (pc+=2 pc-=2) because of 2 bit instructions
    uint16_t pc;

    // Call stack
    // Stack holds PC
    uint16_t stack[16];

    // Stack pointer -- Used to point at index location of stack
    uint8_t sp;

    CPU_MODE mode;

    // Example outline:
    // 6A02 -> 6 = instruction code -> A = register number -> [0][2] = immediate value -> V[A] = 02
    // I is used to hold a memory address until it is redefined
    // The stack array is used to hold addresses for constantly changing memory addresses
    // The stack pointer is used to point at a specific memory address stored in the stack
    // PC increments and decrements by 2 because each opcode is 2 8-bit values
    // Automatically increments unless specified by the opcode
    // PC is used to hold the memory address of the currently running instruction
    // An opcode may decrement to jump to another operation
    // Example: PC = 0x200 -> opcode runs -> new opcode at PC = 0x202 -> new subroutine opcode at PC = 0x204 -> new opcode runs at PC = 0x206 ->
    // return opcode at PC = 0x204

} CPU;

typedef struct {
    char* name;

    char data[512];

    size_t size;

    bool loaded;
} DiskFile;

typedef struct {
    DiskFile files[4];

    int file_count;

} VirtualDisk;

typedef struct {

    // Disk storage
    // uint8_t disk[1024];

    VirtualDisk disk;

    // 64 x 32 display
    uint8_t display[2048];

    // Delay timer
    uint8_t delay_timer;

    uint8_t sound_timer;

    // Keyboard output
    bool keys[16];
} IO;

typedef struct {
    // CPU struct
    CPU cpu;
    // IO struct
    IO io;
    // Running state
    bool running;
} CHIP8_SYSTEM;


void chip8_init(CPU *cpu);
void chip8_load_disk(CHIP8_SYSTEM *chip);
void chip8_run_cli_prompt(CHIP8_SYSTEM *chip);
void chip8_cycle(CPU *cpu, IO *io);
void chip8_tick_timers(IO *io);
void chip8_render(IO *io, SDL_Renderer *renderer);
void chip8_load_rom(CHIP8_SYSTEM *chip, const char *filename);
void chip8_handle_rom(CHIP8_SYSTEM *chip, SDL_Renderer *renderer);

#endif