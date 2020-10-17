#include <stddef.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
// #include "mem.h"

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

    // CPU fetch/decode/execute loop
    for(;;) {
        cycle(c, cpu);
    }

    // Teardown
    if (!is_cpu_provided) {
        free(cpu);
    }
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

        case ld:
            cpu->v[params->x] = params->kk;
        // Adder
        case add:
            cpu->v[params->x] += params->kk;
        case ldr:
            cpu->v[params->x] = cpu->v[params->y];
        default:
            printf("ERROR: OpCode %x not recognized.\n\n", code);
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
                default:
                    // printf("ERROR: weird instruction encountered");
            }
        case 0x6000:
            return ld;
        case 0x7000:
            return add;
        case 0x8000:
            return ldr;
    }
}

opcode_params* decode_params(uint16_t data) {
    opcode_params* params = malloc(sizeof(struct opcode_params));

    params->x = (data & 0x0F00) >> 8;
    params->y = (data & 0x00F0) >> 4;
    params->kk = (uint8_t)data & 0x00FF;

    return params;
}