/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 *
 * InitDMTimer(), WaitUSDMTimer() and WaitMSDMTimer() are derived from src/bbb/am335x_dmtimer1.c of libbbb, and modified:
 *
 *     libbbb, Apache License, Version 2.0 (the text is in LICENSES/Apache-2.0.txt)
 *     Copyright 2019 University of Applied Sciences Western Switzerland / Fribourg
 *     https://gitlab.forge.hefr.ch/embsys/libbbb
 */

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "am335x_regs.h"

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




