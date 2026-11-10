/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "am335x_regs.h"

/*************************************************************************
 * Function Name: InitCorePLL
 * Parameters: None
 *
 * Return: None
 *
 * Description: Inits Core DPLL. CLKDCOLDO      2000MHZ
 *                               CORE_CLKOUTM6  500MHz
 *                               CORE_CLKOUTM4  200MHz
 *                               CORE_CLKOUTM5  250MHz
 *
 *************************************************************************/
void InitCorePLL(int clock, int div)
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
    CM_DIV_M4_DPLL_CORE->BIT.HSDIVIDER_CLKOUT1_DIV = 10;
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




