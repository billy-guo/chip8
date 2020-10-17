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

int main(int argc, char** argv) {
    chip* chip = init();

    // Require a filename be present (nothing more than 100 chars)
    char* filename = malloc(100);
    if (argc < 2) {
        filename = "../roms/Maze.ch8";
    } else {
        filename = argv[1]; 
    }

    // Load ROM file into memory
    load_rom(chip, filename);

    // for (int i = 0x200; i < EMU_MEMORY; i++) {
    //     printf("addr %d: %x\n", i, chip->mem[i]);
    // }

    // Run the ROM
    run(chip, NULL);

    // // init GLUT and create Window
	// glutInit(&argc, argv);
	// glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	// glutInitWindowPosition(100,100);
	// glutInitWindowSize(320,320);
	// glutCreateWindow(filename);

    // glutMainLoop();

    // Free memory
    free(chip);

    // TODO: figure out why C doesn't want me freeing this filename string
    // free(filename);
}