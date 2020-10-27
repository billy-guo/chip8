#include <stddef.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
// #include "mem.h"

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel) {
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(Uint16 *)p = pixel;
        break;

    case 3:
        if(SDL_BYTEORDER == SDL_BIG_ENDIAN) {
            p[0] = (pixel >> 16) & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = pixel & 0xff;
        } else {
            p[0] = pixel & 0xff;
            p[1] = (pixel >> 8) & 0xff;
            p[2] = (pixel >> 16) & 0xff;
        }
        break;

    case 4:
        *(Uint32 *)p = pixel;
        break;
    }
}

// Initializes the register values for the CPU.
CPU* initialize() {
    CPU* cpu = (CPU*) malloc(sizeof(CPU));
    if (cpu == NULL) {
        return NULL;
    }

    // Fill in the 16 V registers with zeroes
    for (int i = 0; i < sizeof(cpu->v) / sizeof(cpu->v[0]); i++) {
        cpu->v[i] = 0;
    }
    // memset(cpu->v, 0, sizeof(cpu->v));
    cpu->address = 0;

    // ROM starts at 0x200
    cpu->pc = 0x200;
    cpu->sp = 0;
    cpu->dt = 0;
    cpu->st = 0;

    return cpu;
}

// Emulates the CHIP8 CPU. You can choose to initialize the CPU struct
// from outside the run() method, which in that case you bear the responsibility
// of tearing it down.
void run(chip* c, CPU* cpu) {
    // Initialize CPU
    int is_cpu_provided = 1;
    if (cpu == NULL) {
        cpu = initialize();
        is_cpu_provided = 0;
    }

    // Initialize graphics
    if (SDL_Init(SDL_INIT_VIDEO) != 0){
        printf("SDL_Init Error: %s", SDL_GetError());
        return;
    }

    SDL_Window *win = SDL_CreateWindow("Hello World!", 100, 100, SCREEN_WIDTH * 10, SCREEN_HEIGHT * 10, SDL_WINDOW_SHOWN);
    SDL_Renderer *ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Surface *surface = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 8, 0, 0, 0, 0);
    SDL_Palette *palette = SDL_AllocPalette(2);
    if (win == NULL || ren == NULL || surface == NULL || palette == NULL) {
        SDL_Log("SDL initialization failed: %s", SDL_GetError());
        exit(1);
    }

    // CHIP-8 graphics are black/white, so only include two colors in the palette
    SDL_Color colors[2];
    SDL_Color white = {0,0,0,0};
    SDL_Color black = {255, 255, 255, 255};
    colors[0] = black;
    colors[1] = white;
    SDL_SetPaletteColors(palette, colors, 0, 2);

    // Set surface palette
    SDL_SetSurfacePalette(surface, palette);

    // CPU fetch/decode/execute loop
    for(;;) {
        cycle(c, cpu);

        //First clear the renderer
        SDL_RenderClear(ren);

        // Redraw the game screen
        SDL_LockSurface(surface);
        for (int x = 0; x < SCREEN_HEIGHT; x++) {
            for (int y = 0; y < SCREEN_WIDTH; y++) {
                putpixel(surface, x, y, c->game_screen[x][y] * 255);
                // putpixel(surface, x, y, 0);
            }
        }
        SDL_UnlockSurface(surface);

        // Grab texture from surface
        SDL_Texture *tex = SDL_CreateTextureFromSurface(ren, surface);

        //Draw the texture
        SDL_RenderCopy(ren, tex, NULL, NULL);

        //Update the screen
        SDL_RenderPresent(ren);

        //Take a quick break after all that hard work
        // SDL_Delay(1000);

    }

    // Teardown
    if (!is_cpu_provided) {
        free(cpu);
    }

    SDL_FreePalette(palette);
    SDL_FreeSurface(surface);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();
}

void print_stuff(uint16_t data, CPU* cpu) {
    printf("hex representation: %x\n", data);
    printf("v0: %d\n", cpu->v[0]);
    printf("v1: %d\n", cpu->v[1]);
    printf("v3: %d\n\n", cpu->v[3]);
}

// Represents a single CPU clock cycle.
void cycle(chip* c, CPU *cpu) {
    // Don'"t do anything if the SP exceeds the mem
    if (cpu->pc >= EMU_MEMORY) {
        return;
    }

    // Read the opcode at the stack pointer
    uint16_t data = (uint16_t)(c->mem[cpu->pc] << 8 | c->mem[cpu->pc + 1]);

    // Increment program counter to go to the next opcode
    cpu->pc += 2;

    // Decode the opcode and opcode parameters
    enum opcode code = decode(data);
    opcode_params* params = decode_params(data);

    // Execute the instruction
    execute(c, cpu, code, params);

    // Free stuff
    free(params);
}

void execute(chip* c, CPU* cpu, enum opcode code, opcode_params* params) {
    switch(code) {
        // Return from subroutine
        case ret:
            cpu->pc = cpu->sp;
            cpu->sp -= 1;
        // Clear the game screen
        case cls:
            memset(c->game_screen, 0, SCREEN_HEIGHT * SCREEN_WIDTH);
        case jp:
            cpu->pc = (params->x << 8) | params->kk;
        // Call subroutine
        case call:
            // Push current program counter onto stack and jump to
            // specified address.
            c->stack[++cpu->sp] = cpu->pc;
            cpu->pc = (params->x << 8) | params->kk;
        case seb:
            // Compares V-register x with kk and increments the PC if the two values are equal
            if (cpu->v[params->x] == params->kk) {
                cpu->pc += 2;
            }
        case sne:
            // Compares V-register x with kk and increments the PC if the two values are unequal
            if (cpu->v[params->x] != params->kk) {
                cpu->pc += 2;
            }
        case sev:
            // Compares V-register x with V-register y with kk and increments the PC if equal
            if (cpu->v[params->x] == cpu->v[params->y]) {
                cpu->pc += 2;
            }
        case ld:
            cpu->v[params->x] = params->kk;
        // Adder
        case add:
            cpu->v[params->x] += params->kk;
        case ldr:
            cpu->v[params->x] = cpu->v[params->y];
        case ldi:
            cpu->address = (params->x << 8) | params->kk;
        case rng:
            cpu->v[params->x] = (rand() % 255) & params->kk;
        // Draw a sprite on the screen
        case drw:
            // Set 0xF-th V register to 0
            cpu->v[0xf] = 0;

            // Number of bytes to read
            int n = params->kk & 0x000f;

            // Loop for the different vertical lines to draw
            for (int yline = 0; yline < n; yline++) {
                // Each sprite is 8 pixels wide
                char data = c->mem[cpu->address + yline];

                // Iterate over each pixel. Note that the ith pixel is the (7-i)th bit
                // for sprite data.
                for (int xpixel = 0; xpixel < 8; xpixel++) {
                    char mask = 1 << (7 - xpixel);
                    if (data & mask) {
                        uint8_t x = cpu->v[params->x] + xpixel;
                        uint8_t y = cpu->v[params->y] + yline;

                        // Count collisions (i.e. drawing over a screen pixel that is already on)
                        if (c->game_screen[x][y] == 1) {
                            cpu->v[0xf] = 1;
                        }

                        // XOR game screen and data
                        c->game_screen[x][y] ^= 1;
                    }
                }
            }
        // default:
            // printf("ERROR: OpCode %x not recognized.\n\n", code);
    }
}

// Decode the opcode into its corresponding instruction.
enum opcode decode(uint16_t data) {
    uint16_t opcode = data & 0xF000;
    switch(opcode) {
        case 0x0000:
            switch(data & 0x00FF) {
                case 0x00EE:
                    return ret;
                case 0x00E0:
                    return cls;
                // default:
                //     printf("ERROR: weird instruction, %x, encountered\n", data);
            }
        case 0x1000:
            return jp;
        case 0x2000:
            return call;
        case 0x3000:
            return seb;
        case 0x4000:
            return sne;
        case 0x5000:
            return sev;
        case 0x6000:
            return ld;
        case 0x7000:
            return add;
        case 0x8000:
            return ldr;
        case 0xa000:
            return ldi;
        case 0xc000:
            return rng;
        case 0xd000:
            return drw;
        default:
            printf("ERROR: weird instruction, %x, encountered\n", data);
    }
}

opcode_params* decode_params(uint16_t data) {
    opcode_params* params = malloc(sizeof(struct opcode_params));

    params->x = (data & 0x0F00) >> 8;
    params->y = (data & 0x00F0) >> 4;
    params->kk = (uint8_t)data & 0x00FF;

    return params;
}