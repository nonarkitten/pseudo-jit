/*
 * gpio.c -- the GPIO banks; the Amiga's RESET and HALT lines: driving them low, letting them go, and waiting until the host lets them go too; and the GreenPAK's supply.
 * RESET and HALT are open drain: the board only ever pulls them low or lets them float (the host's pull-ups and the pads' own raise them). Ball M1 (GPMC_ADVN_ALE, here
 * GPIO2_2) powers the GreenPAK (VDD and VDD2) on a v0.8 or later and is not connected on a v0.6: it is driven high from the start of the boot, and a short dip makes the
 * GreenPAK load its NVM again (GPPowerTest in hal/gpak.c).
 *
 * Copyright (c) 2021-2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "am335x_regs.h"
#include "init.h"
#include "irq.h"
#include "pinmux.h"

#define GPIO0_DATAIN (*(volatile uint32_t*)0x44E07138u)
#define GPIO3_DATAIN (*(volatile uint32_t*)0x481AE138u)
#define HALT_BIT     (1u << 13)         /* GPIO0_13 */
#define RESET_BIT    (1u << 16)         /* GPIO3_16 */
#define HELD_NOTE_MS 100                /* held longer than this: the console says what the boot is waiting for */
#define GPIO2_REG(off)      (*(volatile uint32_t*)(0x481AC000u + (off)))
#define G_OE                0x134
#define G_CLEARDATAOUT      0x190
#define G_SETDATAOUT        0x194
#define GP_SUPPLY_BIT       (1u << 2)   /* GPIO2_2, ball M1 */

static const pin_muxing_t gp_supply_pin[] = {
    { PINMUX_CONF_GPMC_ADVN_ALE, (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M7) },     /* the pad's reset default, now with the GPIO driving it */
    { 0xFFFFFFFF, 0xFFFFFFFF },
};

static const char* const held_names[] = { "", "RESET", "HALT", "RESET and HALT" };

static int Esc3(void) { UART0Ready(); return uart_break; }

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

/* Drives M1 high (high first: the pad's pull-up already holds it there, so the GreenPAK sees no dip). Until this, a v0.8's GreenPAK runs on that pull-up. */
void GPSupplyOn(void) {
    GPIO2_REG(G_SETDATAOUT) = GP_SUPPLY_BIT;
    GPIO2_REG(G_OE) &= ~GP_SUPPLY_BIT;
    config_mux(gp_supply_pin);
}

void GPSupplyDip(uint32_t ms) {
    GPIO2_REG(G_CLEARDATAOUT) = GP_SUPPLY_BIT;
    WaitMSDMTimer(ms);
    GPIO2_REG(G_SETDATAOUT) = GP_SUPPLY_BIT;
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

int ResetHeld(void) {
    return (GPIO3_DATAIN & RESET_BIT ? 0 : 1) | (GPIO0_DATAIN & HALT_BIT ? 0 : 2);
}

int WaitResetReleased(uint32_t settle_ms, int (*stop)(void)) {
    double t0 = ReadDMTimerSeconds();
    int held, ms, said = 0;

    if (!stop) stop = Esc3;
    while ((held = ResetHeld()) != 0) {
        ms = (int)((ReadDMTimerSeconds() - t0) * 1000.0);
        if (!said && ms >= HELD_NOTE_MS) { printf("The host holds %s low: waiting (ESC x3: the menu)\n", held_names[held]); said = 1; }
        if (stop()) return -1;
    }
    ms = (int)((ReadDMTimerSeconds() - t0) * 1000.0);
    if (said) printf("Released after %d ms\n", ms);
    WaitMSDMTimer(settle_ms);
    return ms;
}
