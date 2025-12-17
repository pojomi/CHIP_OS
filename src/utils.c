#include "../include/types.h"

void chip8_init(CPU *cpu) {
    cpu->mode = KERNEL_MODE;
    cpu->pc = 0x1000;

    uint8_t chip8_fontset[80] = {
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

    // Run kernel opcodes to initialize system
    cpu->memory[0x1000] = 0xF0;
    cpu->memory[0x1001] = 0x0;
    cpu->memory[0x1002] = 0xF0;
    cpu->memory[0x1003] = 0x2;
    cpu->memory[0x1004] = 0xF0;
    cpu->memory[0x1005] = 0x4;
    cpu->memory[0x1006] = 0xF0;
    cpu->memory[0x1007] = 0x6;
    cpu->memory[0x1008] = 0xF0;
    cpu->memory[0x1009] = 0x7;
    cpu->memory[0x1010] = 0xF0;
    cpu->memory[0x1011] = 0x8;


    // Load the font into memory
    for (int i = 0; i < 80; i++) {
        cpu->memory[0x50 + i] = chip8_fontset[i];
    }

    printf("CHIP-8 initialized\n");
    cpu->mode = USER_MODE;
}

void chip8_load_disk(CHIP8_SYSTEM *chip) {
    DiskFile *breakout = &chip->io.disk.files[0];
    DiskFile *tetris = &chip->io.disk.files[1];
    FILE *f = fopen("./roms/breakout.ch8", "rb");
    breakout->name = "Breakout";
    tetris->name = "Tetris";
    
    if (f == NULL) {
        printf("Failed to open file\n");
        fclose(f);
    } else {
        breakout->size = fread(breakout->data, 1, sizeof(breakout->data), f);
        chip->io.disk.file_count++;
        printf("Loaded Breakout to VirtualDisk. Byte size: %lu\n", breakout->size);
        fclose(f);
    }
    
    f = fopen("./roms/tetris.ch8", "rb");
    if (f == NULL) {
        printf("Failed to open Tetris\n");
        fclose(f);
    } else {
        tetris->size = fread(tetris->data, 1, sizeof(tetris->data), f);
        chip->io.disk.file_count++;
        printf("Loaded Tetris to VirtualDisk. Byte size: %lu\n", sizeof(tetris->size));
        fclose(f);
    }
}

void chip8_run_cli_prompt(CHIP8_SYSTEM *chip) {
    char os_prompt[256]; 
    int prompt_bytes_received = 
        snprintf(os_prompt, sizeof(os_prompt),
             "**********CHIP_OS**********\n"
             "***************************\n"
             "*******ROMs Available******\n"
             "********1. Breakout********\n"
             "********2. Tetris***********\n"
             "***************************\n"
             "******Input Selection******\n"
             "********3 To Exit**********\n");
    if (prompt_bytes_received == 0) {
        printf("Error: String was not written\n");
    }

    printf("%s", os_prompt);
    
    // Handle choice input
    int valid_choice = 0;
    do {
        int user_input;
        scanf("%d", &user_input);

        switch (user_input) {
            case 1:
                printf("Loading Breakout\n");
                valid_choice = 1;
                chip8_load_rom(chip, "Breakout");
                break;
            case 2:
                printf("Loading Tetris\n");
                valid_choice = 1;
                chip8_load_rom(chip, "Tetris");
                break;
            case 3:
                printf("Closing CHIP_OS\n");
                valid_choice = 1;
                chip->running = false;
                exit(-1);
                break;
            default:
                printf("Invalid command entered. Please enter number 1 through 3\n");
                valid_choice = 0;
        }
    } while (valid_choice == 0);
}

void chip8_load_rom(CHIP8_SYSTEM *chip, const char *filename) {
    // THIS WILL EVENTUALLY BE REMOVED, PUT INTO ASSEMBLY INSTRUCTIONS, AND BE A KERNEL MODE OPERATION
    chip->cpu.pc = 0x200;
    VirtualDisk *disk = &chip->io.disk;
    for (int i = 0; i < disk->file_count; i++) {
        if (strcmp(chip->io.disk.files[i].name, filename) == 0) {
            memcpy(&chip->cpu.memory[0x200], chip->io.disk.files[i].data, chip->io.disk.files[i].size);
            break;
        }
    }
    chip->running = true;
}

void chip8_handle_rom(CHIP8_SYSTEM *chip, SDL_Renderer *renderer) {
    while (chip->running) {
        // Execute multiple instructions per frame
        // CHIP-8 runs at ~540Hz, so at 60FPS that's ~9 instructions per frame
        for (int i = 0; i < 9; i++){
            chip8_cycle(&chip->cpu, &chip->io);
        }

        // Handle events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                chip->running = false;
            }

            // Keyboard down
            if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_1: chip->io.keys[0x1] = 1; break;
                    case SDLK_2: chip->io.keys[0x2] = 1; break;
                    case SDLK_3: chip->io.keys[0x3] = 1; break;
                    case SDLK_4: chip->io.keys[0xC] = 1; break;
                    case SDLK_q: chip->io.keys[0x4] = 1; break;
                    case SDLK_w: chip->io.keys[0x5] = 1; break;
                    case SDLK_e: chip->io.keys[0x6] = 1; break;
                    case SDLK_r: chip->io.keys[0xD] = 1; break;
                    case SDLK_a: chip->io.keys[0x7] = 1; break;
                    case SDLK_s: chip->io.keys[0x8] = 1; break;
                    case SDLK_d: chip->io.keys[0x9] = 1; break;
                    case SDLK_f: chip->io.keys[0xE] = 1; break;
                    case SDLK_z: chip->io.keys[0xA] = 1; break;
                    case SDLK_x: chip->io.keys[0x0] = 1; break;
                    case SDLK_c: chip->io.keys[0xB] = 1; break;
                    case SDLK_v: chip->io.keys[0xF] = 1; break;
                }
            }

            // Keyboard up
            if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_1: chip->io.keys[0x1] = 0; break;
                    case SDLK_2: chip->io.keys[0x2] = 0; break;
                    case SDLK_3: chip->io.keys[0x3] = 0; break;
                    case SDLK_4: chip->io.keys[0xC] = 0; break;
                    case SDLK_q: chip->io.keys[0x4] = 0; break;
                    case SDLK_w: chip->io.keys[0x5] = 0; break;
                    case SDLK_e: chip->io.keys[0x6] = 0; break;
                    case SDLK_r: chip->io.keys[0xD] = 0; break;
                    case SDLK_a: chip->io.keys[0x7] = 0; break;
                    case SDLK_s: chip->io.keys[0x8] = 0; break;
                    case SDLK_d: chip->io.keys[0x9] = 0; break;
                    case SDLK_f: chip->io.keys[0xE] = 0; break;
                    case SDLK_z: chip->io.keys[0xA] = 0; break;
                    case SDLK_x: chip->io.keys[0x0] = 0; break;
                    case SDLK_c: chip->io.keys[0xB] = 0; break;
                    case SDLK_v: chip->io.keys[0xF] = 0; break;
                }
            }
        }

        // Decrement timers (60Hz)
        chip8_tick_timers(&chip->io);

        // Render display
        chip8_render(&chip->io, renderer);

        SDL_Delay(16); // ~16ms per frame = ~60 FPS
    }
}

// Fetch 2, 2-byte code instructions (1 byte == 8 bits, 2 bytes == 16), combines into 16 bit format
void chip8_cycle(CPU *chip, IO *io) {
    uint16_t opcode = (chip->memory[chip->pc] << 8) | chip->memory[chip->pc + 1];
    bool kernel_opcode = (opcode & 0x0F00) == 0;
    chip->pc += 2;
    
    // Break opcode into nibbles (4 bits = 1 nibble) ex: [6][A][0][2]
    // In order to do this, we need to mask 12 of the 16 bits
    uint16_t masked_value;
    uint16_t instruction_code;
    // 0xF000 in binary = 1111 0000 0000 0000 -- opcode & 0xF000 = [6]0000 0000 0000
    masked_value = opcode & 0xF000;
    // instruction_code = masked_value shifted 12 positions to right = 0000 0000 0000 [6]
    instruction_code = masked_value >> 12;
    uint16_t register_num = (opcode << 4 & 0xF000) >> 12;
    uint16_t immediate_value = opcode & 0x00FF;

    switch (instruction_code) {
        case 6:
            chip->V[register_num] = immediate_value;
            break;
        default:
            printf("Unknown instruction: 0x%X\n", opcode);
            break;
        case 0:
            switch (opcode) {
                // Clear display
                case 0x00E0:
                    memset(io->display, 0, sizeof(io->display));
                    break;
                // Return from subroutine
                case 0x00EE:
                    chip->sp--;
                    chip->pc = chip->stack[chip->sp];
                    break;
            }
            break;
        // Jump to address
        case 1:
            chip->pc = opcode & 0x0FFF;
            break;
        // Call subroutine
        case 2:
            chip->stack[chip->sp] = chip->pc;
            chip->sp++;
            chip->pc = opcode & 0x0FFF;
            break;
        // Skip if VX == NN (VXNN)
        case 3:
            if (chip->V[register_num] == immediate_value) {
                chip->pc += 2;
            }
            break;
        // Skip if VX != NN
        case 4:
            if (chip->V[register_num] != immediate_value) {
                chip->pc += 2;
            }
            break;
        // Skip if VX == VY (5XY0)
        case 5:
            if (chip->V[register_num] == chip->V[(opcode >> 4 & 0x0F)]) {
                chip->pc += 2;
            }
            break;
        // Add NN to VX
        case 7:
            chip->V[register_num] = chip->V[register_num] + immediate_value;
            break;
        // Register operations
        case 8:
            switch (opcode & 0x000F) {
                case 0x0:
                    chip->V[register_num] = chip->V[(opcode >> 4 & 0x0F)];
                    break;
                case 0x1:
                    chip->V[register_num] = chip->V[register_num] | chip->V[(opcode >> 4 & 0x0F)];
                    break;
                case 0x2:
                    chip->V[register_num] = chip->V[register_num] & chip->V[(opcode >> 4 & 0x0F)];
                    break;
                case 0x3:
                    chip->V[register_num] = chip->V[register_num] ^ chip->V[(opcode >> 4 & 0x0F)];
                    break;
                // Add with carry (8XY4) - VX = VX + VY - if > 255, VF = 1, else VF = 0
                case 0x4:
                    uint16_t add_VX_VY = chip->V[register_num] + chip->V[(opcode >> 4) & 0x0F];
                    chip->V[register_num] = add_VX_VY;
                    if (add_VX_VY > 0xFF) {
                        chip->V[0xF] = 1;
                    } else {
                        chip->V[0xF] = 0;
                    }
                    break;
                // Subtract with borrow (8XY5)
                case 0x5:
                    uint8_t sub_VX_VY = chip->V[register_num] - chip->V[(opcode >> 4) & 0x0F];
                    if (chip->V[register_num] >= chip->V[(opcode >> 4) & 0x0F]) {
                        chip->V[0xF] = 1;
                    } else {
                        chip->V[0xF] = 0;
                    }
                    chip->V[register_num] = sub_VX_VY;
                        break;
                // Shift VX right by 1. Set VF = the bit that was shifted out
                case 0x6:
                    uint8_t r_shifted_bit = chip->V[register_num] & 1;
                    chip->V[register_num] = chip->V[register_num] >> 1;
                    chip->V[0xF] = r_shifted_bit;
                    break;
                // Set VX = VY - VX (reverse subtract) Set VF = 1 if VY >= VX (no borrow), else 0
                case 0x7:
                    if (chip->V[(opcode >> 4) & 0x0F] >= chip->V[register_num]) {
                        chip->V[0xF] = 1;
                    } else {
                        chip->V[0xF] = 0;
                    }
                    chip->V[register_num] = (chip->V[(opcode >> 4) & 0x0F] - chip->V[register_num]);
                    break;
                // Shift VX left by 1. Set VF = the bit that was shifted out
                case 0xE:
                    uint8_t l_shifted_bit = (chip->V[register_num] >> 7) & 1;
                    chip->V[register_num] = chip->V[register_num] << 1;
                    chip->V[0xF] = l_shifted_bit;

            }
            break;
        // Skip if VX != VY (9XY0)
        case 9:
            if (chip->V[register_num] != chip->V[(opcode >> 4) & 0x0F]) {
                chip->pc += 2;
            }
            break;
        // Set the I register (ANNN)
        case 0xA:
            chip->I = opcode & 0x0FFF;
            break;
        // Jump with offset (BNNN) -- jump to address NNN + V0
        case 0xB:
            chip->pc = (opcode & 0x0FFF) + chip->V[0];
            break;
        // Random (CXNN) - Set VX to a random byte AND NN -- 0-255 & NN
        case 0xC:
            chip->V[register_num] = rand() & immediate_value;
            break;
        // Draw sprite (DXYN) - Draw an N byte sprite at coords VX, VY
        case 0xD:
            // N = HEIGHT of sprite
            // Sprite is ALWAYS 8 bits wide (1 byte = 8 bits rule)
            uint8_t n = opcode & 0x0F;
            chip->V[0xF] = 0;
            // Wrap starting coordinates
            uint8_t x_start = chip->V[register_num] % 64;
            uint8_t y_start = chip->V[(opcode >> 4) & 0xF] % 32;
            // For every row while row < N (height of sprite), repeat for next row
            for (uint8_t row = 0; row < n; row++) {
                uint8_t y = y_start + row;
                // Clip at bottom edge
                if (y >= 32) {
                    break;
                }
                // For every column while col < 8 (8px = 8 sprites x 8 columns = 64), repeat for next column
                for (uint8_t col = 0; col < 8; col++) {
                    uint8_t x = x_start + col;
                    // Clip at right edge
                    if (x >= 64) {
                        break;
                    }

                    // sprite_byte = row (0-2048)
                    uint8_t sprite_byte = chip->memory[chip->I + row];
                    // pixel_bit =  ex col 2: sprite_byte (ex: 0010 1101), >> 5 = 001 (ON!) 
                    uint8_t pixel_bit = (sprite_byte >> (7 - col)) & 1;
                    // Coordinate notes:
                    // X = 0-63, Y = 0-31 --- X is 64 COLUMNS, Y is 32 ROWS
                    // So each ROW is 64px LONG and each COL is 32px WIDE
                    // To find and place the coord, use y * 64 + x
                    // E.g. (20, 25): 25 * 64 + 20 
                    // This tells us: go to row 25, multiply by 64 to get display index of (0, 25), add X coord
                    uint16_t display_index = y * 64 + x;

                    // This checks for pixel collision, if detected, V[F] set to 1
                    if (pixel_bit == 1 && io->display[display_index] == 1) {
                        chip->V[0xF] = 1;
                    }
                    
                    // XOR pixel_bit against what's at the index
                    io->display[display_index] ^= pixel_bit;
                    
                }
            }   
            break;
        // Keyboard operations
        case 0xE:
            switch (opcode & 0x00FF) {
                // Skip next instruction if key VX is NOT pressed
                // EX9E
                case 0xA1:
                    if (io->keys[chip->V[register_num]] == 0) {
                        chip->pc += 2;
                    }
                    break;
                // Skip next instruction if key VX IS pressed
                case 0x9E:
                    if (io->keys[chip->V[register_num]] == 1) {
                        chip->pc += 2;
                    }
                    break;
            }
        case 0xF:
            // Check if it's a syscall (0xF0NN pattern)
            if (kernel_opcode) {
                switch (immediate_value) {
                    // Bootloader init
                    case 0:
                        printf("Initializing booatloader...\n");
                        printf("Initializing CPU register...\n");
                        memset(&chip->memory[0], 0, (sizeof(chip->memory) - 1024));
                        printf("Retrieving memory\n");
                        if (sizeof(chip->memory) == 5120) {
                            printf("Success\n");
                        }
                        memset(chip->V, 0, sizeof(chip->V));
                        if (sizeof(chip->V) == 16) {
                            printf("Success\n");
                        }
                        printf("Initializing I register...\n");
                        chip->I = 0;
                        printf("Done\n");
                        printf("Initializing stack...\n");
                        memset(chip->stack, 0, sizeof(chip->stack));
                        if (sizeof(chip->stack) == 16) {
                            printf("Success\n");
                        }
                        printf("Initializing stack pointer...\n");
                        chip->sp = 0;
                        printf("Done\n");
                        break;
                    case 2:
                        // Maybe add in malloc(cpu->memory, 5120) instead of letting system handle it?
                        break;
                    // I/O Init
                    case 4:
                        printf("Initializing I/O\n");
                        memset(io->display, 0, sizeof(io->display));
                        if (sizeof(io->display) == 2048) {
                            printf("Display: Success\n");
                        } else {
                            printf("Display Fail - Size doesn't match. Received %lu\n", sizeof(io->display));
                        }
                        io->delay_timer = 0;
                        io->sound_timer = 0;
                        if (sizeof(io->disk) == 1128) {
                            printf("Disk: Success\n");
                        } else {
                            printf("Disk fail - retrieved %lu bytes\n", sizeof(io->disk));
                        }
                        memset(io->keys, 0, sizeof(io->keys)); 
                        if (sizeof(io->keys) == 16) {
                            printf("Keys: Success\n");
                        }
                        break;
                    // CLI OS Init
                    case 6:
                        // TODO: define CHIP_OS_INIT()
                        printf("Initializing CHIP_OS\n");
                        break;
                    // Load ROM
                    case 8:
                        // TODO: Load CLI "OS"
                        break;
                    case 9:
                        // Handle syscall requests
                        break;
                }
            } else {
                switch (immediate_value) {
                    case 7:
                        chip->V[register_num] = io->delay_timer;
                        break;
                    // Wait for key press, store key in VX [blocking]
                    case 0xA:
                        bool key_pressed = false;
                        for (int i = 0; i < 16; i++) {
                            if (io->keys[i]) {
                                chip->V[register_num] = i;
                                key_pressed = true;
                                break;
                            }
                        }
                        if (!key_pressed) {
                            chip->pc -= 2; // Repeat this instruction
                        }
                        break;
                    case 15:
                        io->delay_timer = chip->V[register_num];
                        break;
                    case 18:
                        io->sound_timer = chip->V[register_num];
                        break;
                    case 0x1E:
                        chip->I += chip->V[register_num];
                        break;
                    // Font location
                    case 0x29:
                        chip->I = 0x50 + (chip->V[register_num] * 5);
                        break;
                    // Convert decimal VX to BCD (binary coded decimal) across 3 memory locations (NIBBLES!)
                    case 0x33:
                        uint8_t value = chip->V[register_num]; // Ex: 156
                        uint8_t hundreds = value / 100; // Ex: 156 / 100 = 1.56 
                        uint8_t tens = (value / 10) % 10; // Ex: 156 / 10 = 15.6 % 10 = 1r5 = 5
                        uint8_t ones = value % 10; // Ex: 156 % 10 = 15r6 = 6

                        chip->memory[chip->I] = hundreds;
                        chip->memory[chip->I+1] = tens;
                        chip->memory[chip->I+2] = ones;
                        break;
                    case 0x55:
                        for (uint8_t reg = 0; reg <= register_num; reg++) {
                            chip->memory[chip->I + reg] = chip->V[reg];
                        }
                        break;
                    case 0x65:
                        for (uint8_t reg = 0; reg <= register_num; reg++) {
                            chip->V[reg] = chip->memory[chip->I + reg];
                        }
                        break;
                }
                break;
            }
    }
}

void chip8_tick_timers(IO *io) {
    if (io->delay_timer > 0) {
        io->delay_timer--;
    }
    if (io->sound_timer > 0) {
        io->sound_timer--;
    }
}

void chip8_render(IO *io, SDL_Renderer *renderer) {
    // Clear screen to black
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    // Draw white pixels for each "on" pixel in display
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (io->display[y * 64 + x]) {
                // Scale up 10x so it's visibile
                SDL_Rect rect = {x * 10, y * 10, 10, 10};
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
    SDL_RenderPresent(renderer);
}