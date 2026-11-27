/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "am335x_regs.h"
#include "init.h"

/*************************************************************************
 * Function Name: InitCorePLL
 * Parameters: clock, div (CLKDCOLDO = 48 MHz x clock / div), m4 (CORE_CLKOUTM4 = L3F = CLKDCOLDO / m4: the PRU, and halved the GPMC)
 *
 * Return: None
 *
 * Description: Inits Core DPLL. CLKDCOLDO      2000MHZ
 *                               CORE_CLKOUTM6  500MHz
 *                               CORE_CLKOUTM4  2000MHz / m4
 *                               CORE_CLKOUTM5  250MHz
 *
 *************************************************************************/
void InitCorePLL(int clock, int div, int m4)
{
    /*CORE_DPLL in baypass*/
    CM_CLKMODE_DPLL_CORE->BIT.DPLL_EN = 4;
    /*Wait DPLL enter bypass mode*/
    while((1 == CM_IDLEST_DPLL_CORE->BIT.ST_DPLL_CLK) ||
          (0 == CM_IDLEST_DPLL_CORE->BIT.ST_MN_BYPASS));
    /*disable Spread Spectrum Clocking*/
    CM_CLKMODE_DPLL_CORE->BIT.DPLL_SSC_EN = 0;
    /*Set DPLL multiplier factor *1000*/
    CM_CLKSEL_DPLL_CORE->BIT.DPLL_MULT = clock;
    /*Set DPLL divider factor*/
    CM_CLKSEL_DPLL_CORE->BIT.DPLL_DIV = div-1;
    /*Set DPLL post-divider factor M4*/
    CM_DIV_M4_DPLL_CORE->BIT.HSDIVIDER_CLKOUT1_DIV = m4;
    /*Set DPLL post-divider factor M5*/
    CM_DIV_M5_DPLL_CORE->BIT.HSDIVIDER_CLKOUT2_DIV = 8;
    /*Set DPLL post-divider factor M6*/
    CM_DIV_M6_DPLL_CORE->BIT.HSDIVIDER_CLKOUT3_DIV = 4;
    /*CORE_DPLL in baypass*/
    CM_CLKMODE_DPLL_CORE->BIT.DPLL_EN = 7;
    /*Wait DPLL locks*/
    while((0 == CM_IDLEST_DPLL_CORE->BIT.ST_DPLL_CLK) ||
          (1 == CM_IDLEST_DPLL_CORE->BIT.ST_MN_BYPASS));
}

/* L3F (CORE_CLKOUTM4 = CLKDCOLDO / m4) changed with the DPLL locked: the TRM allows the M4, M5 and M6 dividers to change on the fly, acknowledged by a toggle of
   DIVCHACK (bit 5). Returns 0, or -1 when no acknowledge came within 1 ms. */
int SetCoreM4(int m4)
{
    uint32_t v = CM_DIV_M4_DPLL_CORE->LONG, ack = v & 0x20;
    int spin;

    if ((int)(v & 0x1F) == m4) return 0;
    CM_DIV_M4_DPLL_CORE->LONG = (v & ~0x1Fu) | (uint32_t)m4;
    for (spin = 0; spin < 1000; spin++) {
        if ((CM_DIV_M4_DPLL_CORE->LONG & 0x20) != ack) return 0;
        WaitUSDMTimer(1);
    }
    return -1;
}

/* CORE_CLKOUTM4 as the core DPLL is set now, in kHz: the L3F clock, which the PRU and (halved) the GPMC run on. CLKDCOLDO = 2 x 24 MHz x M / (N + 1), divided by M4. */
double CoreM4kHz(void)
{
    uint32_t m = CM_CLKSEL_DPLL_CORE->BIT.DPLL_MULT, n = CM_CLKSEL_DPLL_CORE->BIT.DPLL_DIV + 1, d = CM_DIV_M4_DPLL_CORE->BIT.HSDIVIDER_CLKOUT1_DIV;

    return m && d ? 48000.0 * m / n / d : 200000.0;
}

/* The core DPLL for a host: CORE_CLKOUTM4 (4.8 MHz x M / N, with InitCorePLL's M4 = 10) at `units` times the bus clock, so that `units` GPMC units of 4 of its
   cycles last 4 bus clocks; at least PLL_MARGIN_PPM (init.h) above that (the GPMC's cycle may end a little early, never late: the measurement is good to a few ppm, the two
   crystals drift about 20 ppm apart) and as close above it as M <= 2047 and N <= 32 allow (a reference of 0.75 MHz or more, near the 1 MHz the bench has run). M
   stays above 100, where the DPLL's period jitter is specified. The bus clock is measured by the PRU against the same 24 MHz crystal, so that crystal's error cancels. */
double CorePLLFor(double bus_khz, int units, int* mult, int* div)
{
    double exact = bus_khz * units, want = exact * (1.0 + PLL_MARGIN_PPM * 1e-6), best = 0.0, f;
    int n, m;

    *mult = 1000; *div = 24;
    for (n = 1; n <= 32; n++) {
        m = (int)(want * n / 4800.0);
        if (4800.0 * m / n < want) m++;
        if (m <= 100 || m > 2047) continue;
        f = 4800.0 * m / n;
        if (best == 0.0 || f < best) { best = f; *mult = m; *div = n; }
    }
    return best ? (best / exact - 1.0) * 1e6 : 0.0;
}

/*************************************************************************
 * Function Name: InitMPUPLL
 * Parameters: None
 *
 * Return: None
 *
 * Description: Inits MPU DPLL.  MPU Clock 1000MHZ
 *
 *
 *************************************************************************/
void InitMPUPLL(int clock)
{
    /*MPU_DPLL in baypass*/
    CM_CLKMODE_DPLL_MPU->BIT.DPLL_EN = 4;
    /*Wait DPLL enter bypass mode*/
    while((1 == CM_IDLEST_DPLL_MPU->BIT.ST_DPLL_CLK) ||
          (0 == CM_IDLEST_DPLL_MPU->BIT.ST_MN_BYPASS));
    /*disable Spread Spectrum Clocking*/
    CM_CLKMODE_DPLL_MPU->BIT.DPLL_SSC_EN = 0;
    /*Set DPLL multiplier factor*/
    CM_CLKSEL_DPLL_MPU->BIT.DPLL_MULT = clock;
    /*Set DPLL divider factor*/
    CM_CLKSEL_DPLL_MPU->BIT.DPLL_DIV = 24-1;
    /*Set DPLL post-divider factor M2*/
    CM_DIV_M2_DPLL_MPU->BIT.DPLL_CLKOUT_DIV = 1;
    /*MPU_DPLL in baypass*/
    CM_CLKMODE_DPLL_MPU->BIT.DPLL_EN = 7;
    /*Wait DPLL locks*/
    while((0 == CM_IDLEST_DPLL_MPU->BIT.ST_DPLL_CLK) ||
          (1 == CM_IDLEST_DPLL_MPU->BIT.ST_MN_BYPASS));
}

/*************************************************************************
 * Function Name: InitPERPLL
 * Parameters: None
 *
 * Return: None
 *
 * Description: Inits PER DPLL.  CLKDCOLDO 960MHZ
 *                               CLKOUT    192MHZ
 *
 *************************************************************************/
void InitPERPLL(void)
{
    /*MPU_DPLL in baypass*/
    CM_CLKMODE_DPLL_PER->BIT.DPLL_EN = 4;
    /*Wait DPLL enter bypass mode*/
    while((1 == CM_IDLEST_DPLL_PER->BIT.ST_DPLL_CLK) ||
          (0 == CM_IDLEST_DPLL_PER->BIT.ST_MN_BYPASS));
    /*disable Spread Spectrum Clocking*/
    CM_CLKMODE_DPLL_PER->BIT.DPLL_SSC_EN = 0;
    /*Set DPLL multiplier factor *960*/
    CM_CLKSEL_DPLL_PERIPH->BIT.DPLL_MULT = 960;
    /*Set DPLL divider factor*/
    CM_CLKSEL_DPLL_PERIPH->BIT.DPLL_DIV = 24-1;
    /*Set DPLL Sigma-Delta divider*/
    CM_CLKSEL_DPLL_PERIPH->BIT.DPLL_SD_DIV = 4;
    /*Set DPLL post-divider factor M2*/
    CM_DIV_M2_DPLL_PER->BIT.DPLL_CLKOUT_DIV = 5;
    /*MPU_DPLL in baypass*/
    CM_CLKMODE_DPLL_PER->BIT.DPLL_EN = 7;
    /*Wait DPLL locks*/
    while((0 == CM_IDLEST_DPLL_PER->BIT.ST_DPLL_CLK) ||
          (1 == CM_IDLEST_DPLL_PER->BIT.ST_MN_BYPASS));
}

/*************************************************************************
 * Function Name: InitL3L4Wakeup
 * Parameters: None
 *
 * Return: None
 *
 * Description: Inits L3 and L4 Wakeup
 *
 *************************************************************************/
void InitL3L4Wakeup(void) {
    /*Enable Modules*/
    CM_PER_L3_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_L3_CLKCTRL->BIT.IDLEST);
    CM_PER_L3_INSTR_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_L3_INSTR_CLKCTRL->BIT.IDLEST);

    /*Wakeup Modules*/
    CM_PER_L3_CLKSTCTRL->BIT.CLKTRCTRL = 2;
    CM_PER_OCPWP_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_OCPWP_CLKCTRL->BIT.IDLEST);
    CM_PER_L3S_CLKSTCTRL->BIT.CLKTRCTRL = 2;

    CM_PER_L3_CLKSTCTRL->BIT.CLKACTIVITY_L3_GCLK = 1;
    CM_PER_L3S_CLKSTCTRL->BIT.CLKACTIVITY_L3S_GCLK = 1;

    /*Wakeup Regions*/
    CM_WKUP_CONTROL_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_WKUP_CONTROL_CLKCTRL->BIT.IDLEST);
    CM_WKUP_CLKSTCTRL->BIT.CLKTRCTRL = 2;
    CM_L3_AON_CLKSTCTRL->BIT.CLKTRCTRL = 2;

    CM_L3_AON_CLKSTCTRL->BIT.CLKACTIVITY_L3_AON_GCLK = 1;
    CM_WKUP_L4WKUP_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_WKUP_L4WKUP_CLKCTRL->BIT.IDLEST);
    CM_WKUP_CLKSTCTRL->BIT.CLKACTIVITY_L4_WKUP_GCLK = 1;
    CM_L4_WKUP_AON_CLKSTCTRL->BIT.CLKACTIVITY_L4_WKUP_AON_GCLK = 1;
}




