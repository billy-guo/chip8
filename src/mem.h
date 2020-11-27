// Memory space for CHIP8 is 4kb
#define EMU_MEMORY 4096

// ROM data is loaded in starting at 0x200
#define ROM_START 512

// Stack stores up to 48 bytes
#define STACK_SIZE 24

// Display constants
#define SCREEN_HEIGHT 32
#define SCREEN_WIDTH 64

typedef struct chip {
    // Memory 
    uint8_t mem[EMU_MEMORY];

    // Game screen
    uint8_t game_screen[SCREEN_HEIGHT][SCREEN_WIDTH];

    // Stack memory
    uint16_t stack[STACK_SIZE];
} chip;

void init_sprites(chip* c);