#include "main.h"

void arm_flush_cache(uintptr_t addr, uint32_t length) {
    length = (length + 31) & ~31;
    while (length) {
        asm volatile("mcr p15, 0, %0, c7, c14, 1"::"r"(addr));
        addr += 32;
        length -= 32;
    }
    asm volatile("mcr p15, 0, %0, c7, c10, 4"::"r"(addr));
}

void arm_icache_invalidate(uintptr_t addr, uint32_t length) {
    length = (length + 31) & ~31;
    while (length) {
        asm volatile("mcr p15, 0, %0, c7, c5, 1"::"r"(addr));
        addr += 32;
        length -= 32;
    }
    asm volatile("mcr p15, 0, %0, c7, c10, 4"::"r"(addr));
}

void arm_dcache_invalidate(uintptr_t addr, uint32_t length) {
    length = (length + 31) & ~31;
    while (length) {
        asm volatile("mcr p15, 0, %0, c7, c6, 1"::"r"(addr));
        addr += 32;
        length -= 32;
    }
    asm volatile("mcr p15, 0, %0, c7, c10, 4"::"r"(addr));
}