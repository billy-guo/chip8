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

CPU* initialize();

void run(chip* c, CPU* cpu);

uint16_t cycle(chip* c, CPU *cpu);

uint16_t execute(chip* c, CPU* cpu, uint16_t data);

// Opcode-specific methods
void opcode_0x00e0(chip* c);
void opcode_0x00ee(CPU* cpu);
void opcode_0x1000(CPU* cpu, opcode_params* params);
void opcode_0x2000(chip* c, CPU* cpu, opcode_params* params);
void opcode_0x3000(CPU* cpu, opcode_params* params);
void opcode_0x4000(CPU* cpu, opcode_params* params);
void opcode_0x5000(CPU* cpu, opcode_params* params);
void opcode_0x6000(CPU* cpu, opcode_params* params);
void opcode_0x7000(CPU* cpu, opcode_params* params);
void opcode_0x8xy0(CPU* cpu, opcode_params* params);
void opcode_0x8xy1(CPU* cpu, opcode_params* params);
void opcode_0x8xy2(CPU* cpu, opcode_params* params);
void opcode_0x8xy3(CPU* cpu, opcode_params* params);
void opcode_0x8xy4(CPU* cpu, opcode_params* params);
void opcode_0x8xy5(CPU* cpu, opcode_params* params);
void opcode_0x8xy6(CPU* cpu, opcode_params* params);
void opcode_0x8xy7(CPU* cpu, opcode_params* params);
void opcode_0x8xye(CPU* cpu, opcode_params* params);
void opcode_0x9000(CPU* cpu, opcode_params* params);
void opcode_0xa000(CPU* cpu, opcode_params* params);
void opcode_0xc000(CPU* cpu, opcode_params* params);
void opcode_0xd000(chip* c, CPU* cpu, opcode_params* params);

struct opcode_params* decode_params(uint16_t data);

void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);