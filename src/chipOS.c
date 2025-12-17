#include "../include/types.h"

int main () {
    CHIP8_SYSTEM chip;
    memset(&chip, 0, sizeof(chip));

    // Initialize the CHIP8_SYSTEM members
    chip8_init(&chip.cpu);
    
    // Load the games onto VirtualDisk
    chip8_load_disk(&chip);
    
    // Run all of the kernel opcodes defined in chip8_init before continuing
    while (chip.cpu.pc >= 0x1000 && chip.cpu.pc <= 0x1FFF) {
        chip8_cycle(&chip.cpu, &chip.io);
    }

    chip8_run_cli_prompt(&chip);
        
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // Create window (scale up the 64x32 display to 10x for visibility)
    SDL_Window *window = SDL_CreateWindow(
        "CHIP-8 Emulator",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        64 * 10, 32 * 10, // 640 x 320 px
        SDL_WINDOW_SHOWN
    );

    // Create renderer
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    chip8_handle_rom(&chip, renderer);

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}