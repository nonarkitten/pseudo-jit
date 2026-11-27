/* control_board.c -- the hardware a request from the control page (control.c) needs besides guest memory: the SPI flash and the I2C bus set up for the bootloader's
 * code that the payload carries, the ARM's clock, its caches, and the reset. Built with the bootloader's register header (bootloader_shim.h maps the registers to where
 * the payload sees them). Everything here runs with the guest stopped at its request.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdint.h>
#include "am335x_regs.h"
#include "init.h"
#include "cp15.h"
#include "control.h"
#include "buffee.h"

volatile int uart_break;                        /* (the bootloader's store.c reads it; nothing sets it here) */

void ctl_hw_init(void)
{
    static int done;

    if (done) return;
    InitSPI(24000000);
    I2C0Init(100000);
    done = 1;
}

void ctl_reboot(void)
{
    WaitMSDMTimer(20);                          /* (the UART's FIFO empties) */
    Reset();
    for (;;) { }
}

/* ---- the ARM's clock ---------------------------------------------------------------------------------------------------------------
   CLKOUT = 24 MHz x M / (N + 1) / M2: the bootloader sets N + 1 = 24 and M2 = 1, so M is the clock in MHz. Below 300 MHz the PLL stays locked at 600 MHz or more (its DCO
   runs at twice CLKOUT x M2) and M2 divides it down, so from about 10 MHz up the clock can be set to the MHz; 24 MHz is also the PLL's bypass. The core voltage stays as
   the bootloader set it for the clock the board booted at, which is therefore the most that can be set here. */
unsigned buffee_cpu_mhz(void)
{
    uint32_t m = CM_CLKSEL_DPLL_MPU->BIT.DPLL_MULT, n = CM_CLKSEL_DPLL_MPU->BIT.DPLL_DIV, m2 = CM_DIV_M2_DPLL_MPU->BIT.DPLL_CLKOUT_DIV;

    if (CM_IDLEST_DPLL_MPU->BIT.ST_MN_BYPASS) return 24;
    return 24u * m / (n + 1) / (m2 ? m2 : 1);
}

__attribute__((weak)) void buffee_clock_changed(void) { }
__attribute__((weak)) int ctl_pace(int percent) { (void)percent; return -1; }

int ctl_clock(int mhz)
{
    static unsigned top;
    int ctl_printf(const char *fmt, ...);
    uint32_t m2;

    if (!top) top = buffee_cpu_mhz();
    if (mhz <= 0) return (int)buffee_cpu_mhz();
    if ((unsigned)mhz > top) { ctl_printf("At most %u MHz, the boot clock: CLOCK=%d SAVE, then reboot\n", top, mhz); return -2; }
    if (mhz < 10) { ctl_printf("At least 10 MHz\n"); return -2; }
    CM_CLKMODE_DPLL_MPU->BIT.DPLL_EN = 4;                                       /* bypass (24 MHz) */
    while (CM_IDLEST_DPLL_MPU->BIT.ST_DPLL_CLK || !CM_IDLEST_DPLL_MPU->BIT.ST_MN_BYPASS) { }
    if (mhz != 24) {
        m2 = mhz >= 300 ? 1 : (300u + (unsigned)mhz - 1) / (unsigned)mhz;
        CM_CLKSEL_DPLL_MPU->BIT.DPLL_MULT = (uint32_t)mhz * m2;
        CM_CLKSEL_DPLL_MPU->BIT.DPLL_DIV = 24 - 1;
        CM_DIV_M2_DPLL_MPU->BIT.DPLL_CLKOUT_DIV = m2;
        CM_CLKMODE_DPLL_MPU->BIT.DPLL_EN = 7;                                   /* lock */
        while (!CM_IDLEST_DPLL_MPU->BIT.ST_DPLL_CLK || CM_IDLEST_DPLL_MPU->BIT.ST_MN_BYPASS) { }
    }
    buffee_clock_changed();
    return (int)buffee_cpu_mhz();
}

/* ---- the caches ------------------------------------------------------------------------------------------------------------------------
   L1 instruction and data caches (SCTLR I and C), the L2 (ACTLR L2EN: the payload runs in the secure world, where ACTLR is writable). Data is cleaned out of a cache before
   it is switched off, and a cache that comes back on starts empty. Which code is translated is fixed in a payload: SLOTS and BLOCKS report what it does. */
static uint32_t sctlr(void) { uint32_t v; __asm__ volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r"(v)); return v; }
static void set_sctlr(uint32_t v) { __asm__ volatile ("mcr p15, 0, %0, c1, c0, 0\n isb" :: "r"(v) : "memory"); }
static uint32_t actlr(void) { uint32_t v; __asm__ volatile ("mrc p15, 0, %0, c1, c0, 1" : "=r"(v)); return v; }
static void set_actlr(uint32_t v) { __asm__ volatile ("mcr p15, 0, %0, c1, c0, 1\n isb" :: "r"(v) : "memory"); }
#define SCTLR_C (1u << 2)
#define SCTLR_I (1u << 12)
#define ACTLR_L2EN (1u << 1)

int ctl_cache(unsigned set, unsigned clear)
{
    uint32_t s = sctlr(), a = actlr();
    int now;

    if ((clear & BFC_CACHE_L1D) && (s & SCTLR_C)) { CP15DCacheCleanFlush(); s &= ~SCTLR_C; set_sctlr(s); CP15DCacheCleanFlush(); }
    if ((clear & BFC_CACHE_L1I) && (s & SCTLR_I)) { s &= ~SCTLR_I; set_sctlr(s); CP15ICacheFlush(); }
    if ((clear & BFC_CACHE_L2) && (a & ACTLR_L2EN)) { CP15DCacheCleanFlush(); a &= ~ACTLR_L2EN; set_actlr(a); }
    if ((set & BFC_CACHE_L2) && !(a & ACTLR_L2EN)) { CP15DCacheCleanFlush(); a |= ACTLR_L2EN; set_actlr(a); }
    if ((set & BFC_CACHE_L1I) && !(s & SCTLR_I)) { CP15ICacheFlush(); s |= SCTLR_I; set_sctlr(s); }
    if ((set & BFC_CACHE_L1D) && !(s & SCTLR_C)) { CP15DCacheCleanFlush(); s |= SCTLR_C; set_sctlr(s); }
    __asm__ volatile ("dsb sy\n isb" ::: "memory");
    s = sctlr(); a = actlr();
    now = BFC_CACHE_SLOTS | BFC_CACHE_BLOCKS;
    if (s & SCTLR_I) now |= BFC_CACHE_L1I;
    if (s & SCTLR_C) now |= BFC_CACHE_L1D;
    if (a & ACTLR_L2EN) now |= BFC_CACHE_L2;
    return now;
}
