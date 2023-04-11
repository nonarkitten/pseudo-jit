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

void InitDMTimer(void) {
    CM_WKUP_TIMER1_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_WKUP_TIMER1_CLKCTRL->BIT.IDLEST);

    DMTIMER1_1MS_TIOCP_CFG->BIT.SOFTRESET = 1;
    while(!(DMTIMER1_1MS_TISTAT->BIT.RESETDONE));

    DMTIMER1_1MS_TLDR->LONG = 0;
    DMTIMER1_1MS_TCRR->LONG = 0;
    DMTIMER1_1MS_TTGR->LONG = 0;
    DMTIMER1_1MS_TCLR->BIT.AR = 1;
    DMTIMER1_1MS_TCLR->BIT.ST = 1;
}

uint32_t ReadDMTimer(void) {
    return (DMTIMER1_1MS_TCRR->LONG);
}

double ReadDMTimerSeconds(void) {
    return ((double)DMTIMER1_1MS_TCRR->LONG) / 24000000.0;
}

void WaitUSDMTimer(uint32_t us) {
    uint32_t clocks = us * (24000000 / 1000000) + 1;
    uint32_t st     = DMTIMER1_1MS_TCRR->LONG;
    uint32_t sp     = st;
    while (((sp - st) < clocks)) {
        sp = DMTIMER1_1MS_TCRR->LONG;
    }
}

void WaitMSDMTimer(uint32_t us) {
    uint32_t clocks = us * (24000000 / 1000) + 1;
    uint32_t st     = DMTIMER1_1MS_TCRR->LONG;
    uint32_t sp     = st;
    while (((sp - st) < clocks)) {
        sp = DMTIMER1_1MS_TCRR->LONG;
    }
}

void Reset(void) {
    PRM_RSTCTRL->BIT.RST_GLOBAL_COLD_SW = 1;
}




