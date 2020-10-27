#include <inttypes.h>
#include <SDL.h>
#include "mem.h"

typedef struct CPU {
    // 8-bit V registers (V0 to VF)
    uint8_t v[16];

    // 16-bit Address register (I)
    uint16_t address;

    // Program counter
    uint16_t pc;

    // Stack pointer
    uint8_t sp;

    // Delay timer
    uint8_t dt;

    // Sound timer
    uint8_t st;
} CPU;

typedef struct opcode_params {
    // 4-bit index of a V register
    int x;

    // 4-bit index of another V register
    int y;

    // 8-bit constant
    uint8_t kk;
} opcode_params;

enum opcode{cls, ret, jp, seb, sne, sev, call, ld, add, ldr, ldi, rng, drw};

CPU* initialize();

void run(chip* c, CPU* cpu);

void cycle(chip* c, CPU *cpu);

void execute(chip* c, CPU* cpu, enum opcode code, opcode_params* params);

enum opcode decode(uint16_t data);

struct opcode_params* decode_params(uint16_t data);

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);