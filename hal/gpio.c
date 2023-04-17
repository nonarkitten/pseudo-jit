/*
 * hw_init.c
 *
 *  Created on: Feb 9, 2022
 *      Author: Renee Cousins
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ioam3358.h"

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
    // Set as output, drive low
    GPIO0_DATAOUT->BIT.DATAOUT13 = 0; GPIO0_OE->BIT.OUTPUTEN13 = 0;
    GPIO3_DATAOUT->BIT.DATAOUT16 = 0; GPIO3_OE->BIT.OUTPUTEN16 = 0;
}

void ReleaseReset(void) {
    // set as input, pulled high
    GPIO0_OE->BIT.OUTPUTEN13 = 1; GPIO0_DATAOUT->BIT.DATAOUT13 = 0; 
    GPIO3_OE->BIT.OUTPUTEN16 = 1; GPIO3_DATAOUT->BIT.DATAOUT16 = 0; 
}

uint16_t GetIPLx(void) {
    uint16_t sr = 0;
    GPIO2_OE->BIT.OUTPUTEN6 = 1;
    GPIO2_OE->BIT.OUTPUTEN7 = 1;
    GPIO2_OE->BIT.OUTPUTEN8 = 1;
    asm volatile("nop");
    if(!(GPIO2_DATAIN->BIT.DATAIN6)) sr |= 0x0400;
    if(!(GPIO2_DATAIN->BIT.DATAIN7)) sr |= 0x0200;
    if(!(GPIO2_DATAIN->BIT.DATAIN8)) sr |= 0x0100;
    return sr;
}
