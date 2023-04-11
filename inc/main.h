/*
 * main.h
 *
 *  Created on: Apr. 13, 2021
 *      Author: renee.cousins
 */
#ifndef MAIN_H_
#define MAIN_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "power.h"
#include "ddr.h"
#include "flash.h"
#include "gpmc.h"
#include "init.h"
#include "pru.h"
#include "mmu.h"
#include "mem_be.h"
#include "pinmux.h"

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

extern void arm_flush_cache(uintptr_t addr, uint32_t length);
extern void arm_icache_invalidate(uintptr_t addr, uint32_t length);
extern void arm_dcache_invalidate(uintptr_t addr, uint32_t length);

extern void test_native_bogomips(void);
extern void test_pjit_bogomips(void);

extern void SetEcho(int);
extern void test_printf(void);

extern int pmic_detected;
extern int gpak_detected;
extern int prom_detected;

extern char _image_start, _image_end;
extern char _stack_end, _stack_top;
extern void test_printf(void);

extern void WriteImage(void* start, void* end);

#ifndef _DEBUG
#define printf(...)
#endif

#endif /* MAIN_H_ */
