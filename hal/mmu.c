#include <stdio.h>
#include <stdint.h>

#include "mmu.h"

int MMUEnabled = 0;

__attribute__ ((used, aligned (MMU_PAGETABLE_ALIGN_SIZE)))
uint32_t mmu_table[MMU_PAGETABLE_NUM_ENTRY] = { 0 };

/**
 * MMU Encoding
 * 
 * 31~20        Section Base Address, Must be 1MB aligned
 * 19       0   NS - Non-secure bit - SBZ
 * 18       0
 * 17       0   NG - Not-Global - SBZ
 * 16       0   S - Shareable. 0 - non-shareable, 1 - shareable
 *              Non-sharable by default unles TEX B/C override
 * 
 * 15       0   AP[2] - SBZ
 * 14~12        TEX[2:0]
 * 11~10        AP[1:0]
 * 9            IMP
 * 8~5          Domain
 * 4        0   XN - Execute Never - SBZ
 * 3            C - Cacheable. 0 - non-cacheable, 1 - cacheable
 * 2            B - writeBack. 0 - no-writeback, 1-enable writeback
 * 1        1   MBO
 * 0        0   MBZ
 * 
 *          fedc ba98 7654 3210
 * 0x0c06 = 0000 1100 0000 0110 AP=011, TEX=000, CB=01
 *  AP      Full Access
 *  TEX+CB  Shareable Device
 * 
 * 0x1c0e = 0001 1100 0000 1110 AP=011, TEX=001, CB=11
 *  AP      Full Access
 *  TEX+CB  Outer and Inner Write-Back, Write-Allocate
 * 
 */

void InitMMU(void) {
    // 0x000 to 0x7FF, GPMC/PIO strongly ordered, no caching
    for (int i=0x000; i < 0x020; i++) mmu_table[i] = ((i | 0x020) << 20) | 0x0c06;    
    for (int i=0x020; i < 0x800; i++) mmu_table[i] = (i << 20) | 0x0c06;
    // 0x800 to 0x9FF, SDRAM, Caches write-through, write allocate, access for all
    for (int i=0x800; i < 0xA00; i++) mmu_table[i] = (i << 20) | 0x1c0e;
    // 0x400 to 0x403, internal ROM and SRAM, Caches write-through, write allocate, access for all
    for (int i=0x400; i < 0x403; i++) mmu_table[i] = (i << 20) | 0x1c0e;

    CP15DCacheFlushBuff((uint32_t)mmu_table, sizeof(mmu_table));
    
    // Load new pointer to the mmu table
    asm volatile(
    "       dsb                         \n"
    "       mcr     p15,0,r0,c7,c5,0    \n" /* invalidate icache */
    "       isb                         \n"
    "       ldr     r4,=mmu_table       \n" /* Load MMU table pointer */
    "       mcr     p15,0,r4,c2,c0,0    \n" /* Write page_dir address to ttbr0 */
    "       mov     r8, #0              \n"
    "       mcr     p15,0,r8,c2,c0,2    \n" /* Write ttbr control N = 0 (use only ttbr0) */
    "       mov     r4, #1              \n"
    "       mcr     p15,0,r4,c3,c0,0    \n" /* Set domains - Dom0 is usable, rest is disabled */
    "       mrc     p15,0,r4,c1,c0,0    \n" /* Load control register */
    "       orr     r4,r4,#0x800000     \n" /* v6 page tables, subpages disabled */
    "       orr     r4,r4,#1            \n" /* MMU enabled */
    // "       orr     r4,r4,#1<<25        \n" /* MMU tables in big endian */
    "       dsb                         \n"
    "       mcr     p15,0,r4,c1,c0,0    \n" /* Set control register and thus really enable mmu */
    "       isb                         \n"
    );

    MMUEnabled = 1;
}