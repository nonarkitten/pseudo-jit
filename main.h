/*
 * main.h
 *
 *  Created on: Apr. 13, 2021
 *      Author: renee.cousins
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#ifndef MAIN_H_
#define MAIN_H_


/*
** The input clock speed
*/
#define FREQUENCY                    (24000000)

/*
** Number of entries in a page table. This is only for first level page table
*/
#define MMU_PAGETABLE_NUM_ENTRY      (4096)

/*
** Alignment for Page Table. The Page Table shall be aligned to 16KB boundary.
*/
#define MMU_PAGETABLE_ALIGN_SIZE     (16 * 1024)

/*
** Speed in MHz to set the MCU DPLL to
*/
#define STARTUP_CLOCK                (1000)


extern void arm_flush_cache(uintptr_t addr, uint32_t length);
extern void arm_icache_invalidate(uintptr_t addr, uint32_t length);
extern void arm_dcache_invalidate(uintptr_t addr, uint32_t length);

extern void init_hw(int);
extern double now(void);
extern bool pll_locked(void);
extern double get_cpu_clock(void);
extern void set_cpu_clock(int);

extern void test_native_bogomips(void);
extern void test_pjit_bogomips(void);
extern void test_flash(void);

#ifndef DEBUG
#define printf(...)
#endif

#endif /* MAIN_H_ */
