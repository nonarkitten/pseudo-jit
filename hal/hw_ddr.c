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

#include "hw_ddr.h"

#include "am335x.h"
#include "hw_init.h"
#include "pinmux.h"
#include "support.h"

volatile uint32_t *ddr = (volatile uint32_t *)0x80000000;

#define TEST_SIZE                    8

//*******************************************************************
// DDR3 PHY parameters
//*******************************************************************

#define CMD_PHY_CTRL_SLAVE_RATIO     0x80
#define CMD_PHY_INVERT_CLKOUT        0x0

// GEL
#define DATA_PHY_RD_DQS_SLAVE_RATIO  0x38
#define DATA_PHY_WR_DQS_SLAVE_RATIO  0x44
#define DATA_PHY_FIFO_WE_SLAVE_RATIO 0x94  // RD DQS GATE
#define DATA_PHY_WR_DATA_SLAVE_RATIO 0x7D  // WRITE DATA

// OSD Recommended
// #define  DATA_PHY_RD_DQS_SLAVE_RATIO    0x3A
// #define  DATA_PHY_WR_DQS_SLAVE_RATIO    0x45
// #define  DATA_PHY_FIFO_WE_SLAVE_RATIO   0x95
// #define  DATA_PHY_WR_DATA_SLAVE_RATIO   0x7F

#define DDR_IOCTRL_VALUE             (0x18B)

#define ALLOPP_DDR3_READ_LATENCY \
    0x07  // RD_Latency = (CL + 2) - 1; CL=6 for DDR3-1600 speed grade and CK
          // of 2.5ns

//******************************************************************
// EMIF parameters
//******************************************************************
// GEL
// 400MHz settings
#define ALLOPP_DDR3_SDRAM_TIMING1 0x0AAAD4DB
#define ALLOPP_DDR3_SDRAM_TIMING2 0x266B7FDA
#define ALLOPP_DDR3_SDRAM_TIMING3 0x501F867F

#define ALLOPP_DDR3_SDRAM_CONFIG \
    0x61C05332                                          // termination = 1 (RZQ/4)
                                                        // dynamic ODT = 2 (RZQ/2)
                                                        // SDRAM drive = 0 (RZQ/6)
                                                        // CWL = 0 (CAS write latency = 5)
                                                        // CL = 4 (CAS latency = 6)
                                                        // ROWSIZE = 6 (15 row bits)
                                                        // PAGESIZE = 2 (10 column bits)
#define ALLOPP_DDR3_REF_CTRL                0x00000C30  // 400 * 7.8us = 0xC30
#define ALLOPP_DDR3_ZQ_CONFIG               0x50074BE4

// OSD Recommended
// #define ALLOPP_DDR3_READ_LATENCY    0x00100007
// #define ALLOPP_DDR3_SDRAM_TIMING1   0x0AAAD4DB
// #define ALLOPP_DDR3_SDRAM_TIMING2   0x266B7FDA
// #define ALLOPP_DDR3_SDRAM_TIMING3   0x501F867F
// #define ALLOPP_DDR3_SDRAM_CONFIG    0x61C05332
// #define ALLOPP_DDR3_REF_CTRL        0x00000C30
// #define ALLOPP_DDR3_ZQ_CONFIG       0x50074BE4

#define VTP_CTRL                            *(volatile uint32_t *)0x44E10E0C

#define CMD0_REG_PHY_CTRL_SLAVE_RATIO_0     *(volatile uint32_t *)0x44E1201C
#define CMD0_REG_PHY_DLL_LOCK_DIFF_0        *(volatile uint32_t *)0x44E12028
#define CMD0_REG_PHY_INVERT_CLKOUT_0        *(volatile uint32_t *)0x44E1202C
#define CMD1_REG_PHY_CTRL_SLAVE_RATIO_0     *(volatile uint32_t *)0x44E12050
#define CMD1_REG_PHY_DLL_LOCK_DIFF_0        *(volatile uint32_t *)0x44E1205C
#define CMD1_REG_PHY_INVERT_CLKOUT_0        *(volatile uint32_t *)0x44E12060
#define CMD2_REG_PHY_CTRL_SLAVE_RATIO_0     *(volatile uint32_t *)0x44E12084
#define CMD2_REG_PHY_DLL_LOCK_DIFF_0        *(volatile uint32_t *)0x44E12090
#define CMD2_REG_PHY_INVERT_CLKOUT_0        *(volatile uint32_t *)0x44E12094
#define DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0  *(volatile uint32_t *)0x44E120C8
#define DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0  *(volatile uint32_t *)0x44E120DC
#define DATA0_REG_PHY_WRLVL_INIT_RATIO_0    *(volatile uint32_t *)0x44E120F0
#define DATA0_REG_PHY_WRLVL_INIT_MODE_0     *(volatile uint32_t *)0x44E120F8
#define DATA0_REG_PHY_GATELVL_INIT_RATIO_0  *(volatile uint32_t *)0x44E120FC
#define DATA0_REG_PHY_GATELVL_INIT_MODE_0   *(volatile uint32_t *)0x44E12104
#define DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0 *(volatile uint32_t *)0x44E12108
#define DATA0_REG_PHY_DQ_OFFSET_0           *(volatile uint32_t *)0x44E1211C
#define DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0 *(volatile uint32_t *)0x44E12120
#define DATA0_REG_PHY_USE_RANK0_DELAYS      *(volatile uint32_t *)0x44E12134
#define DATA0_REG_PHY_DLL_LOCK_DIFF_0       *(volatile uint32_t *)0x44E12138
#define DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_0  *(volatile uint32_t *)0x44E1216C
#define DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_0  *(volatile uint32_t *)0x44E12180
#define DATA1_REG_PHY_WRLVL_INIT_RATIO_0    *(volatile uint32_t *)0x44E12194
#define DATA1_REG_PHY_WRLVL_INIT_MODE_0     *(volatile uint32_t *)0x44E1219C
#define DATA1_REG_PHY_GATELVL_INIT_RATIO_0  *(volatile uint32_t *)0x44E121A0
#define DATA1_REG_PHY_GATELVL_INIT_MODE_0   *(volatile uint32_t *)0x44E121A8
#define DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_0 *(volatile uint32_t *)0x44E121AC
#define DATA1_REG_PHY_DQ_OFFSET_0           *(volatile uint32_t *)0x44E121C0
#define DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_0 *(volatile uint32_t *)0x44E121C4
#define DATA1_REG_PHY_USE_RANK0_DELAYS      *(volatile uint32_t *)0x44E121D8
#define DATA1_REG_PHY_DLL_LOCK_DIFF_0       *(volatile uint32_t *)0x44E121DC

#define DDR_CMD0_IOCTRL                     *(volatile uint32_t *)0x44E11404
#define DDR_CMD1_IOCTRL                     *(volatile uint32_t *)0x44E11408
#define DDR_CMD2_IOCTRL                     *(volatile uint32_t *)0x44E1140C
#define DDR_DATA0_IOCTRL                    *(volatile uint32_t *)0x44E11440
#define DDR_DATA1_IOCTRL                    *(volatile uint32_t *)0x44E11444
#define DDR_CKE_CTRL                        *(volatile uint32_t *)0x44E1131C
#define DDR_IO_CTRL                         *(volatile uint32_t *)0x44E10E04

#define EMIF_DDR_PHY_CTRL_1                 *(volatile uint32_t *)0x4C0000E4
#define EMIF_DDR_PHY_CTRL_1_SHDW            *(volatile uint32_t *)0x4C0000E8
#define EMIF_DDR_PHY_CTRL_2                 *(volatile uint32_t *)0x4C0000EC
#define EMIF_SDRAM_TIM_1                    *(volatile uint32_t *)0x4C000018
#define EMIF_SDRAM_TIM_1_SHDW               *(volatile uint32_t *)0x4C00001C
#define EMIF_SDRAM_TIM_2                    *(volatile uint32_t *)0x4C000020
#define EMIF_SDRAM_TIM_2_SHDW               *(volatile uint32_t *)0x4C000024
#define EMIF_SDRAM_TIM_3                    *(volatile uint32_t *)0x4C000028
#define EMIF_SDRAM_TIM_3_SHDW               *(volatile uint32_t *)0x4C00002C
#define EMIF_SDRAM_REF_CTRL                 *(volatile uint32_t *)0x4C000010
#define EMIF_SDRAM_REF_CTRL_SHDW            *(volatile uint32_t *)0x4C000014
#define EMIF_ZQ_CONFIG                      *(volatile uint32_t *)0x4C0000C8
#define EMIF_SDRAM_CONFIG                   *(volatile uint32_t *)0x4C000008

#define EMIF_STATUS                         *(volatile uint32_t *)0x4C000004

void DDRInit(void) {
    static const pin_muxing_t pinmux[] = {
        {CONF_ECAP0_IN_PWM0_OUT, PIN_CFG_M7},
        {0xFFFFFFFF},
    };
    am335x_clock_init_ddr_pll(266);
    am335x_clock_enable_emif();

    /*Enable VTT*/
    am335x_clock_enable_gpio_module(AM335X_CLOCK_GPIO0);
    config_mux(pinmux);
    /*Make VTT_EN (GPIO0_7) pin output and EN*/
    am335x_gpio_setup_pin_out(AM335X_CLOCK_GPIO0, 7, 1);

    /*VTP Init*/
    /*Dynamic VTP compensation mode*/
    VTP_CTRL = 0;
    VTP_CTRL = LE32(6);
    VTP_CTRL = LE32(70);
    /*clears flops  start count again*/
    VTP_CTRL = LE32(71);
    /*wait ready*/
    while (!(VTP_CTRL & LE32(32)))
        ;

    /*DDR Phy CMD0*/
    CMD0_REG_PHY_CTRL_SLAVE_RATIO_0 = LE32(CMD_PHY_CTRL_SLAVE_RATIO);
    CMD0_REG_PHY_INVERT_CLKOUT_0    = LE32(CMD_PHY_INVERT_CLKOUT);
    /*DDR Phy CMD1*/
    CMD1_REG_PHY_CTRL_SLAVE_RATIO_0 = LE32(CMD_PHY_CTRL_SLAVE_RATIO);
    CMD1_REG_PHY_INVERT_CLKOUT_0    = LE32(CMD_PHY_INVERT_CLKOUT);
    /*DDR Phy CMD2*/
    CMD2_REG_PHY_CTRL_SLAVE_RATIO_0 = LE32(CMD_PHY_CTRL_SLAVE_RATIO);
    CMD2_REG_PHY_INVERT_CLKOUT_0    = LE32(CMD_PHY_INVERT_CLKOUT);

    /*Init Data Macro 0*/
    DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0 =
        LE32(DATA_PHY_RD_DQS_SLAVE_RATIO);
    DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0 =
        LE32(DATA_PHY_WR_DQS_SLAVE_RATIO);
    DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0 =
        LE32(DATA_PHY_FIFO_WE_SLAVE_RATIO);
    DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0 =
        LE32(DATA_PHY_WR_DATA_SLAVE_RATIO);
    /*Init Data Macro 1*/
    DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_0 =
        LE32(DATA_PHY_RD_DQS_SLAVE_RATIO);
    DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_0 =
        LE32(DATA_PHY_WR_DQS_SLAVE_RATIO);
    DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_0 =
        LE32(DATA_PHY_FIFO_WE_SLAVE_RATIO);
    DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_0 =
        LE32(DATA_PHY_WR_DATA_SLAVE_RATIO);

    /*Rank 0 delays are used for all ranks. This must be set to 0 for DDR3*/
    DATA0_REG_PHY_USE_RANK0_DELAYS = 0;
    DATA1_REG_PHY_USE_RANK0_DELAYS = 0;

    /**/
    DDR_CMD0_IOCTRL  = LE32(DDR_IOCTRL_VALUE);
    DDR_CMD1_IOCTRL  = LE32(DDR_IOCTRL_VALUE);
    DDR_CMD2_IOCTRL  = LE32(DDR_IOCTRL_VALUE);
    DDR_DATA0_IOCTRL = LE32(DDR_IOCTRL_VALUE);
    DDR_DATA1_IOCTRL = LE32(DDR_IOCTRL_VALUE);

    // IO to work for DDR3
    DDR_IO_CTRL &= ~LE32(0x10000000);

    // CKE controlled by EMIF/DDR_PHY
    DDR_CKE_CTRL = LE32(0x00000001);

    for (int i = 0; i < 50000; i++) asm("    nop");

    /**/
    EMIF_DDR_PHY_CTRL_1      = LE32(ALLOPP_DDR3_READ_LATENCY);
    EMIF_DDR_PHY_CTRL_1_SHDW = LE32(ALLOPP_DDR3_READ_LATENCY);
    EMIF_DDR_PHY_CTRL_2      = LE32(ALLOPP_DDR3_READ_LATENCY);
    /**/
    EMIF_SDRAM_TIM_1      = LE32(ALLOPP_DDR3_SDRAM_TIMING1);
    EMIF_SDRAM_TIM_1_SHDW = LE32(ALLOPP_DDR3_SDRAM_TIMING1);
    EMIF_SDRAM_TIM_2      = LE32(ALLOPP_DDR3_SDRAM_TIMING2);
    EMIF_SDRAM_TIM_2_SHDW = LE32(ALLOPP_DDR3_SDRAM_TIMING2);
    EMIF_SDRAM_TIM_3      = LE32(ALLOPP_DDR3_SDRAM_TIMING3);
    EMIF_SDRAM_TIM_3_SHDW = LE32(ALLOPP_DDR3_SDRAM_TIMING3);

    EMIF_SDRAM_REF_CTRL      = LE32(ALLOPP_DDR3_REF_CTRL);
    EMIF_SDRAM_REF_CTRL_SHDW = LE32(ALLOPP_DDR3_REF_CTRL);

    EMIF_ZQ_CONFIG    = LE32(ALLOPP_DDR3_ZQ_CONFIG);
    EMIF_SDRAM_CONFIG = LE32(ALLOPP_DDR3_SDRAM_CONFIG);

    while (!(EMIF_STATUS & LE32(4)))
        ;
}

void DDRTest(void) {
    uint32_t expected, read;

    for (int i = 0; i < TEST_SIZE; i++) {
        expected = 0x01010101 << i;
        ddr[i]   = expected;
    }
    for (int i = 0; i < TEST_SIZE; i++) {
        expected = 0x01010101 << i;
        read     = ddr[i];
        (void)read;  // GCC bug
        printf("[DDR0] Read %08lX from %p  expected  %08lX\n", read,
               (void *)&ddr[i], expected);
    }
}