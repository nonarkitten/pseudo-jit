/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * This is part of PJIT the Pseudo-JIT 68K emulator.
 *
 * PJIT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * PJIT is licensed under a Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * Under the terms of this license you are free copy and redistribute
 * the material in any medium or format as well as remix, transform,
 * and build upon the material.
 *
 * You must give appropriate credit, provide a link to the license,
 * and indicate if changes were made. You may do so in any reasonable
 * manner, but not in any way that suggests the licensor endorses you
 * or your use.
 *
 * You may not use the material for commercial purposes.
 *
 * If you remix, transform, or build upon the material, you must
 * distribute your contributions under the same license as the original.
 *
 * You may not apply legal terms or technological measures that legally
 * restrict others from doing anything the license permits.
 *
 * Portions of PJIT have been derived from the following:
 *
 *     Castaway (formerly FAST), GPL version 2 License
 *     Copyright (c) 1994-2002 Martin Döring, Joachim Hönig
 *    
 *     Cyclone 68K, GPL version 2 License
 *     Copyright (c) 2004,2011 Dave "FinalDave" Haywood
 *     Copyright (c) 2005-2011 Graûvydas "notaz" Ignotas
 *    
 *     TI StarterWare, modified BSD 3-Clause License
 *     Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 *     libbbb, Apache License, Version 2.0
 *     Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 * 
 *     emu68 (https://github.com/michalsc), Mozilla Public License, v. 2.0
 *     Copyright © 2019 Michal Schulz <michal.schulz@gmx.de>
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ioam3358.h"

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
void InitCorePLL(int clock)
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
    CM_CLKSEL_DPLL_CORE->BIT.DPLL_DIV = 24-1;
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




