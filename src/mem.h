// Memory space for CHIP8 is 4kb
#define EMU_MEMORY 4096

// ROM data is loaded in starting at 0x200
#define ROM_START 512

// Stack stores up to 48 bytes
#define STACK_SIZE 48

// Display constants
#define SCREEN_HEIGHT 64
#define SCREEN_WIDTH 32

typedef struct chip {
    // Memory 
    char mem[EMU_MEMORY];

    // Game screen
    char game_screen[SCREEN_HEIGHT][SCREEN_WIDTH];

    // Stack memory
    char stack[STACK_SIZE];

} chip;