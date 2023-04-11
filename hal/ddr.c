#include "ddr.h"
#include "init.h"

volatile uint32_t *ddr = (volatile uint32_t *)0x80000000;

#define TEST_SIZE 8

void InitDDR(void) {
    InitDDRPLL();
    InitDDREMIF();
}

void DDRTest(void) {
    uint32_t expected, read;
    int i;
    
    for(i = 0; i < TEST_SIZE; i++) {
        expected = 0x01010101 << i;
        ddr[i] = expected;
    }
    for(i = 0; i < TEST_SIZE; i++) {
        expected = 0x01010101 << i;
        read = ddr[i]; (void)read; // GCC bug
        printf("[DDR0] Read %08lX from %p, expected  %08lX\n", read, (void*)&ddr[i], expected);
    }
}