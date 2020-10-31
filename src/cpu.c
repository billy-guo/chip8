#include <stddef.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "cpu.h"
// #include "mem.h"

void putpixel(SDL_Surface *surface, int x, int y, uint32_t pixel) {
    int bpp = surface->format->BytesPerPixel;
    /* Here p is the address to the pixel we want to set */
    uint8_t *p = (uint8_t *)surface->pixels + y * surface->pitch + x * bpp;

    switch(bpp) {
    case 1:
        *p = pixel;
        break;

    case 2:
        *(uint16_t *)p = pixel;
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
        *(uint32_t *)p = pixel;
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

    SDL_Window *win = SDL_CreateWindow("CHIP-8 Emulator", 100, 100, SCREEN_WIDTH * 10, SCREEN_HEIGHT * 10, SDL_WINDOW_SHOWN);
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

    // Keep track of cycle execution time
    struct timeval tval_before, tval_after;

    // CPU fetch/decode/execute loop
    for(;;) {
        // Starting time
        gettimeofday(&tval_before, NULL);
    
        // Run CPU cycle
        uint16_t opcode = cycle(c, cpu);

        // Redraw screen, if necessary.
        if (opcode == 0xD000) {
            //First clear the renderer
            SDL_RenderClear(ren);

            // Redraw the game screen
            SDL_LockSurface(surface);
            for (int y = 0; y < SCREEN_HEIGHT; y++) {
                for (int x = 0; x < SCREEN_WIDTH; x++) {
                    putpixel(surface, x, y, c->game_screen[y][x] * 255);
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
        }

        // Wait for 1/60 of a second to elapse
        gettimeofday(&tval_after, NULL);
        float time_to_wait = 16666.7 - (tval_after.tv_usec - tval_before.tv_usec);
        if (time_to_wait > 0) {
            usleep(time_to_wait);
        }
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

// Represents a single CPU clock cycle. Returns the opcode that was
// executed during the cycle.
uint16_t cycle(chip* c, CPU *cpu) {
    // Don't do anything if the SP exceeds the mem
    if (cpu->pc >= EMU_MEMORY) {
        return 0;
    }

    // Read the opcode at the stack pointer
    uint16_t data = (uint16_t)(c->mem[cpu->pc] << 8 | c->mem[cpu->pc + 1]);

    // Increment program counter to go to the next opcode
    cpu->pc += 2;

    // Execute the instruction
    return execute(c, cpu, data);
}

uint16_t execute(chip* c, CPU* cpu, uint16_t data) {
    // Decode opcode parameters
    opcode_params* params = decode_params(data);
    printf("[LOC %d]:   %x ", cpu->pc, data);

    switch(data & 0xF000) {
        case 0x0000:
            switch(data & 0x00FF) {
                case 0x00EE:
                    opcode_0x00ee(cpu);
                    return 0x00EE;
                case 0x00E0:
                    opcode_0x00e0(c);
                    return 0x00E0;
            }
        case 0x1000:
            opcode_0x1000(cpu, params);
            return 0x1000;
        case 0x2000:
            opcode_0x2000(c, cpu, params);
            return 0x2000; 
        case 0x3000:
            opcode_0x3000(cpu, params);
            return 0x3000;
        case 0x4000:
            opcode_0x4000(cpu, params);
            return 0x4000;
        case 0x5000:
            opcode_0x5000(cpu, params);
            return 0x5000;
        case 0x6000:
            opcode_0x6000(cpu, params);
            return 0x6000;
        case 0x7000:
            opcode_0x7000(cpu, params);
            return 0x7000;
        case 0x8000:
            switch(data & 0x000F) {
                case 0x0000:
                    opcode_0x8xy0(cpu, params);
                case 0x0001:
                    opcode_0x8xy1(cpu, params);
                case 0x0002:
                    opcode_0x8xy2(cpu, params);
                case 0x0003:
                    opcode_0x8xy3(cpu, params);
                case 0x0004:
                    opcode_0x8xy4(cpu, params);
                case 0x0005:
                    opcode_0x8xy5(cpu, params);
                case 0x0006:
                    opcode_0x8xy6(cpu, params);
                case 0x0007:
                    opcode_0x8xy7(cpu, params);
                case 0x000e:
                    opcode_0x8xye(cpu, params);
            }
            return 0x8000;
        case 0x9000:
            opcode_0x9000(cpu, params);
            return 0x9000;
        case 0xA000:
            opcode_0xa000(cpu, params);
            return 0xA000;
        case 0xC000:
            opcode_0xc000(cpu, params);
            return 0xC000;
        case 0xD000:
            opcode_0xd000(c, cpu, params);
            return 0xD000;
        // default:
            // printf("ERROR: OpCode %x not recognized.\n\n", code);
    }

    free(params);
}

opcode_params* decode_params(uint16_t data) {
    opcode_params* params = malloc(sizeof(struct opcode_params));

    params->x = (data & 0x0F00) >> 8;
    params->y = (data & 0x00F0) >> 4;
    params->kk = (uint8_t)data & 0x00FF;

    return params;
}

void opcode_0x8xy0(CPU* cpu, opcode_params* params) {
    printf("LD V%d, V%d\n", params->x, params->y);
    cpu->v[params->x] = cpu->v[params->y];
}

void opcode_0x8xy1(CPU* cpu, opcode_params* params) {
    printf("OR V%d, V%d\n", params->x, params->y);
    cpu->v[params->x] = cpu->v[params->x] | cpu->v[params->y];
}

void opcode_0x8xy2(CPU* cpu, opcode_params* params) {
    printf("AND V%d, V%d\n", params->x, params->y);
    cpu->v[params->x] = cpu->v[params->x] & cpu->v[params->y];
}

void opcode_0x8xy3(CPU* cpu, opcode_params* params) {
    printf("XOR V%d, V%d\n", params->x, params->y);
    cpu->v[params->x] = cpu->v[params->x] ^ cpu->v[params->y];
}

void opcode_0x8xy4(CPU* cpu, opcode_params* params) {
    printf("ADD V%d, V%d\n", params->x, params->y);
    uint16_t sum = cpu->v[params->x] + cpu->v[params->y];

    // Lowest 8 bits are added to the Vx register, modify carry register as needed
    if (sum > 255) {
        cpu->v[0xf] = 1;
    }

    cpu->v[params->x] = (uint8_t)(sum & 255);
}

void opcode_0x8xy5(CPU* cpu, opcode_params* params) {
    printf("SUB V%d, V%d\n", params->x, params->y);

    // Set carry bit to 1 if Vx > Vy and 0 otherwise
    if (cpu->v[params->x] > cpu->v[params->y]) {
        cpu->v[0xf] = 1;
    } else {
        cpu->v[0xf] = 0;
    }

    cpu->v[params->x] -= cpu->v[params->y];
}

void opcode_0x8xy6(CPU* cpu, opcode_params* params) {
    printf("SHR V%d, V%d\n", params->x, params->y);

    // Set Vf to 1 if Vx's least significant bit is 1
    if (cpu->v[params->x] & 1 == 1) {
        cpu->v[0xf] = 1;
    } else {
        cpu->v[0xf] = 0;
    }

    cpu->v[params->x] /= 2;
}

void opcode_0x8xy7(CPU* cpu, opcode_params* params) {
    printf("SUBN V%d, V%d\n", params->x, params->y);

    // Set carry bit to 1 if Vx < Vy and 0 otherwise
    if (cpu->v[params->x] < cpu->v[params->y]) {
        cpu->v[0xf] = 1;
    } else {
        cpu->v[0xf] = 0;
    }

    cpu->v[params->x] = cpu->v[params->y] - cpu->v[params->x];
}

void opcode_0x8xye(CPU* cpu, opcode_params* params) {
    printf("SHL V%d, V%d\n", params->x, params->y);

    // Set Vf to 1 if Vx's least significant bit is 1
    if (cpu->v[params->x] >> 7 == 1) {
        cpu->v[0xf] = 1;
    } else {
        cpu->v[0xf] = 0;
    }

    cpu->v[params->x] *= 2;
}

// Clear the game screen
void opcode_0x00e0(chip* c) {
    printf("CLS\n");
    memset(c->game_screen, 0, SCREEN_HEIGHT * SCREEN_WIDTH);
}

// Return from subroutine
void opcode_0x00ee(CPU* cpu) {
    printf("RET\n");
    cpu->pc = cpu->sp;
    cpu->sp -= 1;
}

void opcode_0x1000(CPU* cpu, opcode_params* params) {
    printf("JP %d\n", (params->x << 8) | params->kk);
    cpu->pc = (params->x << 8) | params->kk;
}

// Call subroutine
void opcode_0x2000(chip* c, CPU* cpu, opcode_params* params) {
    printf("CALL %d\n", (params->x << 8) | params->kk);

    // Push current program counter onto stack and jump to
    // specified address.
    c->stack[++cpu->sp] = cpu->pc;
    cpu->pc = (params->x << 8) | params->kk;
}

void opcode_0x3000(CPU* cpu, opcode_params* params) {
    printf("SE V%d, %d\n", params->x, params->kk);

    // Compares V-register x with kk and increments the PC if the two values are equal
    if (cpu->v[params->x] == params->kk) {
        cpu->pc += 2;
    }
}

void opcode_0x4000(CPU* cpu, opcode_params* params) {
    printf("SNE V%d, %d\n", params->x, params->kk);

    // Compares V-register x with kk and increments the PC if the two values are unequal
    if (cpu->v[params->x] != params->kk) {
        cpu->pc += 2;
    }
}

void opcode_0x5000(CPU* cpu, opcode_params* params) {
    printf("SE V%d, V%d\n", params->x, params->y);

    // Compares V-register x with V-register y with kk and increments the PC if equal
    if (cpu->v[params->x] == cpu->v[params->y]) {
        cpu->pc += 2;
    }
}

void opcode_0x6000(CPU* cpu, opcode_params* params) {
    printf("LD V%d, %d\n", params->x, params->kk);
    cpu->v[params->x] = params->kk;
}

void opcode_0x7000(CPU* cpu, opcode_params* params) {
    printf("ADD V%d, %d\n", params->x, params->kk);
    cpu->v[params->x] += params->kk;
}

void opcode_0x9000(CPU* cpu, opcode_params* params) {
    printf("SNE V%d, V%d\n", params->x, params->y);
    if (cpu->v[params->x] != cpu->v[params->y]) {
        cpu->pc += 2;
    }
}

void opcode_0xa000(CPU* cpu, opcode_params* params) {
    printf("LD I, %d\n", (params->x << 8) | params->kk);
    cpu->address = (params->x << 8) | params->kk;
}

void opcode_0xc000(CPU* cpu, opcode_params* params) {
    printf("RND V%d, %d\n", params->x, params->kk);
    cpu->v[params->x] = (rand() % 255) & params->kk;
}

// Draw a sprite on the screen
void opcode_0xd000(chip* c, CPU* cpu, opcode_params* params) {
    printf("DRW V%d, V%d, %d\n", params->x, params->y, params->kk & 0x000f);

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
                if (c->game_screen[y][x] == 1) {
                    cpu->v[0xf] = 1;
                }

                // XOR game screen and data
                c->game_screen[y][x] ^= 1;
            }
        }
    }
}