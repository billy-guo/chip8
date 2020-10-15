#include <inttypes.h>

typedef struct CPU {
    // 8-bit V registers (V0 to VF)
    int8_t v[16];

    // 16-bit Address register (I)
    int16_t address;

    // Program counter
    int16_t pc;

    // Stack pointer
    int8_t sp;

    // Delay timer
    int8_t dt;

    // Sound timer
    int8_t st;
} CPU;

typedef struct opcode_params {
    // 4-bit index of a V register
    int x;

    // 4-bit index of another V register
    int y;

    // 8-bit constant
    uint8_t kk;
} opcode_params;

enum opcode{ld, add, ldr};

CPU* initialize();

void run(uint8_t* mem, CPU* cpu);

void cycle(struct CPU *cpu, uint8_t* mem);

enum opcode decode(uint16_t data);

struct opcode_params* decode_params(uint16_t data);
