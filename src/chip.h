// Memory space for CHIP8 is 4K
const int EMU_MEMORY = 4096;

// ROM data is loaded in starting at 0x200
const int ROM_START = 512;

typedef struct chip {
    // Memory 
    int* mem;
} chip;