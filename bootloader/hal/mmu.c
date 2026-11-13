/*
 * mmu.c -- the bootloader's translation table (sections of 1 MB) and the switch that turns the MMU on.
 *
 * Copyright (c) 2021-2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdio.h>
#include <stdint.h>

#include "mmu.h"
#include "cp15.h"

/* First-level section descriptors (1 MB each): bits 1:0 = 10 (a section), B = bit 2, C = bit 3, AP[1:0] = bits 11:10, TEX[2:0] = bits 14:12 */
#define SEC_SECTION   0x002u
#define SEC_B         0x004u
#define SEC_C         0x008u
#define SEC_AP_FULL   0xC00u                                         /* AP[1:0] = 11: read and write at every privilege level */
#define SEC_TEX(n)    ((uint32_t)(n) << 12)
#define SEC_DEVICE    (SEC_SECTION | SEC_B | SEC_AP_FULL)            /* 0x0c06: TEX 000 C 0 B 1, shareable device; without B (0x0c02) strongly ordered */
#define SEC_NORMAL_WB (SEC_SECTION | SEC_B | SEC_C | SEC_AP_FULL | SEC_TEX(1))   /* 0x1c0e: TEX 001 C 1 B 1, normal memory, write-back, write-allocate */

#ifndef GPMC_ATTR
#define GPMC_ATTR SEC_DEVICE
#endif
int MMUEnabled = 0;

__attribute__ ((used, aligned (MMU_PAGETABLE_ALIGN_SIZE)))
/* the top 16 KB of the SRAM (linker.lds, section .mmu): the table needs a 16 KB alignment, and anywhere else it wastes up to 12 KB when the image grows past a boundary */
static volatile uint32_t mmu_table[MMU_PAGETABLE_NUM_ENTRY] __attribute__((section(".mmu_table"), aligned(16384)));

/* CP15 c1, the system control register: bit 0 turns the MMU on; bit 23 (XP) selects the ARMv6 page-table format, which is the only one an ARMv7 has */
#define SCTLR_M  (1u << 0)
#define SCTLR_XP (1u << 23)

/* Points the MMU at `table`, makes domain 0 a client and every other domain fault, and turns translation on. Section-sized pages only (TTBCR.N = 0). */
static void mmu_start(uint32_t table) {
    const uint32_t zero = 0, domain0_client = 1;
    uint32_t sctlr;

    __asm__ volatile("dsb" ::: "memory");
    __asm__ volatile("mcr p15, 0, %0, c7, c5, 0" :: "r"(zero) : "memory");      /* ICIALLU: invalidate the whole instruction cache */
    __asm__ volatile("isb" ::: "memory");
    __asm__ volatile("mcr p15, 0, %0, c2, c0, 0" :: "r"(table) : "memory");     /* TTBR0 */
    __asm__ volatile("mcr p15, 0, %0, c2, c0, 2" :: "r"(zero) : "memory");      /* TTBCR.N = 0: TTBR0 translates the whole address space */
    __asm__ volatile("mcr p15, 0, %0, c3, c0, 0" :: "r"(domain0_client) : "memory");   /* DACR */
    __asm__ volatile("mrc p15, 0, %0, c1, c0, 0" : "=r"(sctlr));
    sctlr |= SCTLR_XP | SCTLR_M;
    __asm__ volatile("dsb" ::: "memory");
    __asm__ volatile("mcr p15, 0, %0, c1, c0, 0" :: "r"(sctlr) : "memory");     /* SCTLR: the MMU is on after the isb */
    __asm__ volatile("isb" ::: "memory");
}

void InitMMU(void) {
    for (int i = 0; i < MMU_PAGETABLE_NUM_ENTRY; i++) mmu_table[i] = 0;      /* the section is not cleared at startup (NOLOAD at the top of the SRAM): unmapped = fault */
    // 0x000 to 0x7FF: the GPMC window and the peripherals, as device memory (not cached)
    for (int i=0x000; i < 0x020; i++) mmu_table[i] = ((i | 0x020) << 20) | GPMC_ATTR;
    for (int i=0x020; i < 0x800; i++) mmu_table[i] = (i << 20) | GPMC_ATTR;
    // 0x800 to 0x9FF: the DDR3, cached write-back / write-allocate
    for (int i=0x800; i < 0xA00; i++) mmu_table[i] = (i << 20) | SEC_NORMAL_WB;
    // 0x400 to 0x403: the internal ROM and SRAM, cached the same way (0x403 holds the top of the SRAM: the stack and this table; left as device memory every stack access cost ~70 ns)
    for (int i=0x400; i <= 0x403; i++) mmu_table[i] = (i << 20) | SEC_NORMAL_WB;

    CP15DCacheFlushBuff((uint32_t)mmu_table, sizeof(mmu_table));
    
    mmu_start((uint32_t)mmu_table);

    MMUEnabled = 1;
}