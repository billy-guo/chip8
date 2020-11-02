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

    // Handle keyboard/mouse input
    SDL_Event event;

    // CPU fetch/decode/execute loop
    for(;;) {

        // Exit program
        if (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(1);
            }
        }

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
    printf("[LOC %d]:   %x ", cpu->pc - 2, data);

    switch(data & 0xF000) {
        case 0x0000:
            switch(data & 0x00FF) {
                case 0x00EE:
                    opcode_0x00ee(cpu);
                    break;
                case 0x00E0:
                    opcode_0x00e0(c);
                    break;
            }
            break;
        case 0x1000:
            opcode_0x1000(cpu, params);
            break;
        case 0x2000:
            opcode_0x2000(c, cpu, params);
            break;
        case 0x3000:
            opcode_0x3000(cpu, params);
            break;
        case 0x4000:
            opcode_0x4000(cpu, params);
            break;
        case 0x5000:
            opcode_0x5000(cpu, params);
            break;
        case 0x6000:
            opcode_0x6000(cpu, params);
            break;
        case 0x7000:
            opcode_0x7000(cpu, params);
            break;
        case 0x8000:
            switch(data & 0x000F) {
                case 0x0000:
                    opcode_0x8xy0(cpu, params);
                    break;
                case 0x0001:
                    opcode_0x8xy1(cpu, params);
                    break;
                case 0x0002:
                    opcode_0x8xy2(cpu, params);
                    break;
                case 0x0003:
                    opcode_0x8xy3(cpu, params);
                    break;
                case 0x0004:
                    opcode_0x8xy4(cpu, params);
                    break;
                case 0x0005:
                    opcode_0x8xy5(cpu, params);
                    break;
                case 0x0006:
                    opcode_0x8xy6(cpu, params);
                    break;
                case 0x0007:
                    opcode_0x8xy7(cpu, params);
                    break;
                case 0x000e:
                    opcode_0x8xye(cpu, params);
                    break;
            }
            break;
        case 0x9000:
            opcode_0x9000(cpu, params);
            break;
        case 0xA000:
            opcode_0xa000(cpu, params);
            break;
        case 0xB000:
            opcode_0xb000(cpu, params);
            break;
        case 0xC000:
            opcode_0xc000(cpu, params);
            break;
        case 0xD000:
            opcode_0xd000(c, cpu, params);
            return 0xD000;
        case 0xF000:
            switch(data & 0xF0FF) {
                case 0xF007:
                    opcode_0xfx07(cpu, params);
                    break;
                case 0xF00a:
                    opcode_0xfx0a(cpu, params);
                    break;
                case 0xF015:
                    opcode_0xfx15(cpu, params);
                    break;
                case 0xF018:
                    opcode_0xfx18(cpu, params);
                    break;
                case 0xF01E:
                    opcode_0xfx1e(cpu, params);
                    break;
                case 0xF029:
                    opcode_0xfx29(c, cpu, params);
                    break;
                case 0xF033:
                    opcode_0xfx33(c, cpu, params);
                    break;
                case 0xF055:
                    opcode_0xfx55(c, cpu, params);
                    break;
                case 0xF065:
                    opcode_0xfx65(c, cpu, params);
                    break;
                default:
                    printf("ERROR: OpCode %x not recognized.\n\n", data);
            }
            break;
        default:
            printf("ERROR: OpCode %x not recognized.\n\n", data);
    }

    return data;
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
    cpu->v[params->x] ^= cpu->v[params->y];
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
    printf("CALL %x\n", (params->x << 8) | params->kk);

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
    printf("LD I, %x\n", (params->x << 8) | params->kk);
    cpu->address = (params->x << 8) | params->kk;
}

void opcode_0xb000(CPU* cpu, opcode_params* params) {
    printf("JP V0, %d\n", (params->x << 8) | params->kk);
    cpu->pc = cpu->v[0] + (params->x << 8) | params->kk;
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
        uint8_t data = c->mem[cpu->address + yline];

        // Iterate over each pixel. Note that the ith pixel is the (7-i)th bit
        // for sprite data.
        for (int xpixel = 0; xpixel < 8; xpixel++) {
            uint8_t mask = 1 << (7 - xpixel);
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

// Ex9E - SKP Vx
// Skip next instruction if key with the value of Vx is pressed.

// Checks the keyboard, and if the key corresponding to the value of Vx is currently in the down position, PC is increased by 2.
void opcode_0xe9e(CPU* cpu, opcode_params* params) {
    printf("SKP V%d\n", params->x);

    // Poll keyboard
    SDL_Event event;
    if (SDL_PollEvent(&event) && event.key.keysym.sym == val_to_key(cpu->v[params->x]) && event.key.state == SDL_PRESSED) {
        cpu->pc += 2;
    }
}


// ExA1 - SKNP Vx
// Skip next instruction if key with the value of Vx is not pressed.

// Checks the keyboard, and if the key corresponding to the value of Vx is currently in the up position, PC is increased by 2.
void opcode_0xea1(CPU* cpu, opcode_params* params) {
    printf("SKNP V%d\n", params->x);

    // Poll keyboard
    SDL_Event event;
    if (!SDL_PollEvent(&event) || (event.key.keysym.sym == val_to_key(cpu->v[params->x]) && event.key.state == SDL_RELEASED)) {
        cpu->pc += 2;
    }
}

// Fx07 - LD Vx, DT
// Set Vx = delay timer value.

// The value of DT is placed into Vx.
void opcode_0xfx07(CPU* cpu, opcode_params* params) {
    printf("LD V%d, DT\n", params->x);

    cpu->v[params->x] = cpu->dt;
}

// Fx0A - LD Vx, K
// Wait for a key press, store the value of the key in Vx.

// All execution stops until a key is pressed, then the value of that key is stored in Vx.
void opcode_0xfx0a(CPU* cpu, opcode_params* params) {
    printf("LD V%d, K\n", params->x);

    SDL_Event event;
    while (SDL_WaitEvent(&event) && event.key.state == SDL_PRESSED) {
        cpu->v[params->x] = key_to_v_register(event.key);
    }
}

// Fx15 - LD DT, Vx
// Set delay timer = Vx.

// DT is set equal to the value of Vx.
void opcode_0xfx15(CPU* cpu, opcode_params* params) {
    printf("LD DT, V%d\n", params->x);

    cpu->dt = cpu->v[params->x];
}

// Fx18 - LD ST, Vx
// Set sound timer = Vx.

// ST is set equal to the value of Vx.
void opcode_0xfx18(CPU* cpu, opcode_params* params) {
    printf("LD ST, V%d\n", params->x);

    cpu->st = cpu->v[params->x];
}

// Fx1E - ADD I, Vx
// Set I = I + Vx.

// The values of I and Vx are added, and the results are stored in I.
void opcode_0xfx1e(CPU* cpu, opcode_params* params) {
    printf("ADD I, V%d\n", params->x);

    cpu->address += cpu->v[params->x];
}

// Fx29 - LD F, Vx
// Set I = location of sprite for digit Vx.

// The value of I is set to the location for the hexadecimal sprite corresponding to the value of Vx. See section 2.4, Display, for more information on the Chip-8 hexadecimal font.
void opcode_0xfx29(chip* c, CPU* cpu, opcode_params* params) {
    printf("LD F, V%d\n", params->x);

    cpu->address = c->mem[cpu->v[params->x] * 5];
}

// Fx33 - LD B, Vx
// Store BCD representation of Vx in memory locations I, I+1, and I+2.

// The interpreter takes the decimal value of Vx, and places the hundreds digit in memory at location in I, the tens digit at location I+1, and the ones digit at location I+2.
void opcode_0xfx33(chip* c, CPU* cpu, opcode_params* params) {
    printf("LD B, V%d\n", params->x);

    c->mem[cpu->address] = (cpu->v[params->x] / 100) % 10;
    c->mem[cpu->address + 1] = (cpu->v[params->x] / 10) % 10;
    c->mem[cpu->address + 2] = cpu->v[params->x]% 10;
}

// Fx55 - STRR Vx
// Store registers V0 through Vx in memory starting at location I.

// The interpreter copies the values of registers V0 through Vx into memory, starting at the address in I.
void opcode_0xfx55(chip* c, CPU* cpu, opcode_params* params) {
    printf("STRR, V%d\n", params->x);

    for (int i = 0; i <= params->x; i++) {
        c->mem[cpu->address + i] = cpu->v[i];
    }
}

// Fx65 - LD Vx, [I]
// Read registers V0 through Vx from memory starting at location I.

// The interpreter reads values from memory starting at location I into registers V0 through Vx.
void opcode_0xfx65(chip* c, CPU* cpu, opcode_params* params) {
    printf("STRI, V%d\n", params->x);

    for (int i = 0; i <= params->x; i++) {
        cpu->v[i] = c->mem[cpu->address + i];
    }
}