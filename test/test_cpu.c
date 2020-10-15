#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/cpu.h"

// Ensures we initialize the CPU correctly.
void test_initialize() {
    CPU* cpu = initialize();

    assert(cpu->address == 0);
    assert(cpu->pc == 0x200);
    assert(cpu->sp == 0);
    assert(cpu->dt == 0);
    assert(cpu->st == 0);

    printf("TEST_INITIALIZE PASS\n");
}

// Checks the contents of a simple dummy ROM for correctness.
void test_cycle() {
    // Initialize CPU
    CPU* cpu = initialize();

    // 4 instructions, each 2 bytes long
    uint8_t* test_mem = malloc(4 * 2);
    test_mem[0] = 96;
    test_mem[1] = 0;
    test_mem[2] = 97;
    test_mem[3] = 2;
    test_mem[4] = 112;
    test_mem[5] = 17;
    test_mem[6] = 131;
    test_mem[7] = 0;

    run(test_mem, cpu);

    // Check the CPU registers
    assert(cpu->v[0] == 17);
    assert(cpu->v[1] == 2);
    assert(cpu->v[3] == 17);

    // Free memory
    free(cpu);
    free(test_mem);
}

int main() {
    test_initialize();
    test_cycle();
}