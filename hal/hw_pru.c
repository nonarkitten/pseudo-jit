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
 */
#include "main.h"

#include "pru_data.h"
#include "pru_text.h"

// extern unsigned char pru_data_bin[];
// extern unsigned int pru_data_bin_len;
// extern unsigned char pru_text_bin[];
// extern unsigned int pru_text_bin_len;
static pin_muxing_t pru_pins[] = {
    /* Inputs with pull-up */
    { CONF_MCASP0_FSX,       (PIN_CFG_IEN | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M6) }, // BBB_BGACK
    { CONF_MCASP0_AXR0,      (PIN_CFG_IEN | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M6) }, // BBB_RESET
    { CONF_MCASP0_AHCLKR,    (PIN_CFG_IEN | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M6) }, // BBB_BR
    { CONF_XDMA_EVENT_INTR1, (PIN_CFG_IEN | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M5) }, // BBB_VPA
    { CONF_LCD_DATA0,        (PIN_CFG_IEN | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M6) }, // BBB_IPL2
    { CONF_LCD_DATA1,        (PIN_CFG_IEN | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M6) }, // BBB_IPL1
    { CONF_LCD_DATA2,        (PIN_CFG_IEN | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M6) }, // BBB_IPL0

    /* CMOS Inputs */
    { CONF_MCASP0_ACLKR,     (PIN_CFG_IEN | PIN_CFG_PTD | PIN_CFG_DIS | PIN_CFG_M6) }, // BBB_CLK7

    /* Outputs */
    { CONF_MCASP0_ACLKX,     (PIN_CFG_IDIS | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M5) }, // PRU_VMA
    { CONF_MCASP0_FSR,       (PIN_CFG_IDIS | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M5) }, // BBB_BG
    { CONF_MCASP0_AXR1,      (PIN_CFG_IDIS | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M5) }, // PRU_EWAIT
    { CONF_MCASP0_AHCLKX,    (PIN_CFG_IDIS | PIN_CFG_PTU | PIN_CFG_EN | PIN_CFG_M5) }, // PRU_ECLK

    { 0xFFFFFFFF, 0xFFFFFFFF },
};

void InitPRU(void) {
    am335x_pru_init();

    /*Configure pins*/
    config_mux(pru_pins);

    am335x_pru_memcpy(PRU0_IRAM, 0, pru_text_bin_len, pru_text_bin);
    am335x_pru_memcpy(PRU0_DRAM, 0, pru_data_bin_len, pru_data_bin);

    am335x_pru_enable(PRU0);
}

