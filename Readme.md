# CHIP_OS
CHIP_OS is an expansion on the traditional CHIP-8 CPU interpreter. CHIP_OS is a hobby project to learn more about C programming,
memory management, operating systems architecture, and the way hardware sends and receives instructions

# Features
- All original CHIP-8 opcodes
- New structs designed more like an OS
- Added a virtual disk to simulate how an OS stores files
- Additional KERNEL_MODE opcodes used to simulate a bootloader
- Added a CLI menu after successful bootload which waits for an input selection to:
    - Load a selected ROM
    - Close the program

## Installation
 # Prerequisites:
     - GCC or Clang
     - Make
     - SDL2 development libraries
         - Ubuntu/Debian:
             - sudo apt-get install build-essential libsdl2-dev
         - macOS:
             - brew install sdl2

 # Build:
     - git clone https://github.com/pojomi/chip_os.git
     - cd chip_os
     - make
 
 # Directory:
     - ./CHIP_OS
         - build/
             - chipOS.o
             - utils.o
         - include/
             - types.h
         - roms/
             - breakout.ch8
             - tetris.ch8
         - src/
             - chipOS.c
             - utils.c
         - chip_os
         - Makefile
         - Readme.md


## Usage
 # From root directory post-build:
 `./chip_os`
 
 # Replace/Add ROMs:
   - Replace or add any ".ch8" ROMs by placing in `./roms`
   - Update names in `chip8_load_rom()` found in `./src/utils.c`
   - Update CLI menu in `chip8_run_cli_prompt()` found in `./src/utils.c`
   - `make`
   - `./chip_os`

# Keyboard Controls:
  - Usable keys are: 
    - 1, 2, 3, 4,
    - Q, W, E, R,
    - A, S, D, F,
    - Z, X, C, V





#### Roadmap
  
  ### TODO:
    - Kernel stack
    - OS data structures
    - Implement protection checks in memory read/write
      - USER_MODE can't access kernel memory (0x1000+)
      - USER_MODE can't access kernel registers (V[16-23])
      - Trigger fault on violation

  ### **DONE**

    - Integrate an OS kernel into the CPU architecture
    
    - Develop a bootloader to initialize OS
      - Initialize memory
      - Initialize IO
      - Initialize CLI menu

    - Develop a menu for the OS in CLI - Design idea directly below
      - |-------------------------------------|
      - |------------ 1. Load Snake ----------|
      - |------------ 2. Load Breakout -------|
      - |------------ 3. Load ... ------------|
      - |------------ 4. Shutdown ------------|

    - Use scanf() for int to initialize selection

    - Redesign structs --- struct CPU (reg, memory, stack, sp, pc, I, mode) --- components of a struct are called **MEMBERS**
    - Expand CHIP-8 CPU register count from 16 to 24
      - Keep registers 1-16 as-is to maintain .ch8 compatibility
      - Implement CPU.mode member to designate user (V[0-14]) vs system (V[16-23])
      - Reserve V[15] for flag operations (currently integrated in original code)
  
    - Increase memory from 4KB to 5KB (kernel registers will use the last 1KB for data and code)
      - Memory layout (0x000-0x1FF) [512B] - Interpreter/fonts (original)
      - (0x200-0xFFF) [3.5KB] - User program space (original)
      - (0x1000 - 0x13FF) [1KB] - Kernel space (new)
    
    - Syscall handlers

    - Create struct for I/O ex: struct io (display[2048], keys[16])
      - This is for organizational purposes to separate the hardware from the CPU
      - Adjust current opcodes to properly point to the new structs and members
   
    - Create struct CHIP8_SYSTEM to hold everything
      - CPU
      - IO
      - Kernel
      - uint8_t memory[5120]
      - bool running

 
    - Create system kernel opcodes for register 16

    - Develop bootloader concept

    - Implement instructions for bootloader

    - Once bootloader is operational:
      - Implement instructions for CLI inputs
      - Implement action from selection submission (chip8_load_rom()) 

    - Design syscall mechanism
      - Define syscall opcode format (e.g. 0xF0NN)
      - Implement mode switching on syscall
      - Create syscall opcode handling in chip8_cycle

    - Implement basic syscalls

    - Create ROM "filesystem" (embedded ROMs in C code)
      - Array of ROM data
      - ROM metadata (name, size)
      - Syscall to load by index