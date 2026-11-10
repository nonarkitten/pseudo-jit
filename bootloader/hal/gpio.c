/*
 * gpio.c -- the GPIO banks, and the Amiga's RESET and HALT lines: driving them low and letting them go.
 *
 * Copyright (c) 2021-2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "am335x_regs.h"
#include "irq.h"

void InitGPIO(void) {
    CM_WKUP_GPIO0_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_WKUP_GPIO0_CLKCTRL->BIT.IDLEST);
    CM_WKUP_GPIO0_CLKCTRL->BIT.OPTFCLKEN_GPIO0_GDBCLK = 1;
    GPIO0_CTRL->BIT.DISABLEMODULE = 0; // enable module
    GPIO0_SYSCONFIG->BIT.IDLEMODE = 1; // no idle

    CM_PER_GPIO1_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_GPIO1_CLKCTRL->BIT.IDLEST);
    CM_PER_GPIO1_CLKCTRL->BIT.OPTFCLKEN_GPIO_1_GDBCLK = 1;
    GPIO1_CTRL->BIT.DISABLEMODULE = 0; // enable module
    GPIO1_SYSCONFIG->BIT.IDLEMODE = 1; // no idle

    CM_PER_GPIO2_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_GPIO2_CLKCTRL->BIT.IDLEST);
    CM_PER_GPIO2_CLKCTRL->BIT.OPTFCLKEN_GPIO_2_GDBCLK = 1;
    GPIO2_CTRL->BIT.DISABLEMODULE = 0; // enable module
    GPIO2_SYSCONFIG->BIT.IDLEMODE = 1; // no idle

    CM_PER_GPIO3_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_GPIO3_CLKCTRL->BIT.IDLEST);
    CM_PER_GPIO3_CLKCTRL->BIT.OPTFCLKEN_GPIO_3_GDBCLK = 1;
    GPIO3_CTRL->BIT.DISABLEMODULE = 0; // enable module
    GPIO3_SYSCONFIG->BIT.IDLEMODE = 1; // no idle
}

void AssertReset(void) {
    IRQResetMask(1);
    // Set as output, drive low
    GPIO0_DATAOUT->BIT.DATAOUT13 = 0; GPIO0_OE->BIT.OUTPUTEN13 = 0;
    GPIO3_DATAOUT->BIT.DATAOUT16 = 0; GPIO3_OE->BIT.OUTPUTEN16 = 0;
}

void ReleaseReset(void) {
    // set as input, pulled high
    GPIO0_OE->BIT.OUTPUTEN13 = 1; GPIO0_DATAOUT->BIT.DATAOUT13 = 0; 
    GPIO3_OE->BIT.OUTPUTEN16 = 1; GPIO3_DATAOUT->BIT.DATAOUT16 = 0; 
    IRQResetMask(0);
}
