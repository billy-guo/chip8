#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/gl.h>
#include <GL/freeglut.h>
#include "cpu.h"

// Initializes a CHIP8 emulation. 
chip* init() {
    // Allocate struct pointer and all internal members
    chip* c = malloc(sizeof(chip));

    // Initialize sprites
    init_sprites(c);

    return c;
}

void load_rom(chip* chip, char* filename) {
    // Open as binary file
    FILE* rom;
    rom = fopen(filename, "rb");

    // Put the rom into the CHIP8's internal memory
    fread(&chip->mem[0x200], 0xfff, 1, rom);
    
    // Close the ROM file
    fclose(rom);
}

void init_sprites(chip* c) {
    c->mem[0] = 0xF0;
    c->mem[1] = 0x90;
    c->mem[2] = 0x90;
    c->mem[3] = 0x90;
    c->mem[4] = 0xF0;

    c->mem[5] = 0x20;
    c->mem[6] = 0x60;
    c->mem[7] = 0x20;
    c->mem[8] = 0x20;
    c->mem[9] = 0x70;

    c->mem[10] = 0xF0;
    c->mem[11] = 0x10;
    c->mem[12] = 0xF0;
    c->mem[13] = 0x80;
    c->mem[14] = 0xF0;

    c->mem[15] = 0xF0;
    c->mem[16] = 0x10;
    c->mem[17] = 0xF0;
    c->mem[18] = 0x10;
    c->mem[19] = 0xF0;

    c->mem[20] = 0x90;
    c->mem[21] = 0x90;
    c->mem[22] = 0xF0;
    c->mem[23] = 0x10;
    c->mem[24] = 0x10;

    c->mem[25] = 0xF0;
    c->mem[26] = 0x80;
    c->mem[27] = 0xF0;
    c->mem[28] = 0x10;
    c->mem[29] = 0xF0;

    c->mem[30] = 0xF0;
    c->mem[31] = 0x80;
    c->mem[32] = 0xF0;
    c->mem[33] = 0x90;
    c->mem[34] = 0xF0;

    c->mem[35] = 0xF0;
    c->mem[36] = 0x10;
    c->mem[37] = 0x20;
    c->mem[38] = 0x40;
    c->mem[39] = 0x40;

    c->mem[40] = 0xF0;
    c->mem[41] = 0x90;
    c->mem[42] = 0xF0;
    c->mem[43] = 0x90;
    c->mem[44] = 0xF0;

    c->mem[45] = 0xF0;
    c->mem[46] = 0x90;
    c->mem[47] = 0xF0;
    c->mem[48] = 0x10;
    c->mem[49] = 0xF0;

    c->mem[50] = 0xF0;
    c->mem[51] = 0x90;
    c->mem[52] = 0xF0;
    c->mem[53] = 0x90;
    c->mem[54] = 0x90;

    c->mem[55] = 0xE0;
    c->mem[56] = 0x90;
    c->mem[57] = 0xE0;
    c->mem[58] = 0x90;
    c->mem[59] = 0xE0;

    c->mem[60] = 0xF0;
    c->mem[61] = 0x80;
    c->mem[62] = 0x80;
    c->mem[63] = 0x80;
    c->mem[64] = 0xF0;

    c->mem[65] = 0xE0;
    c->mem[66] = 0x90;
    c->mem[67] = 0x90;
    c->mem[68] = 0x90;
    c->mem[69] = 0xE0;

    c->mem[70] = 0xF0;
    c->mem[71] = 0x80;
    c->mem[72] = 0xF0;
    c->mem[73] = 0x80;
    c->mem[74] = 0xF0;

    c->mem[75] = 0xF0;
    c->mem[76] = 0x80;
    c->mem[77] = 0xF0;
    c->mem[78] = 0x80;
    c->mem[79] = 0x80;
}

int main(int argc, char** argv) {
    chip* chip = init();

    // Require a filename be present (nothing more than 100 chars)
    char* filename = malloc(100);
    if (argc < 2) {
        filename = "../roms/BLINKY.ch8";
    } else {
        filename = argv[1]; 
    }

    // Load ROM file into memory
    load_rom(chip, filename);

    // Run the ROM
    run(chip, NULL);

    // Free memory
    free(chip);

    // TODO: figure out why C doesn't want me freeing this filename string
    // free(filename);
}