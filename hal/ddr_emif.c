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

//*******************************************************************
//DDR3 PHY parameters
//*******************************************************************

#define  CMD_PHY_CTRL_SLAVE_RATIO       0x80
#define  CMD_PHY_INVERT_CLKOUT          0x0

// GEL
#define  DATA_PHY_RD_DQS_SLAVE_RATIO    0x38
#define  DATA_PHY_WR_DQS_SLAVE_RATIO    0x44
#define  DATA_PHY_FIFO_WE_SLAVE_RATIO   0x94 //RD DQS GATE
#define  DATA_PHY_WR_DATA_SLAVE_RATIO   0x7D  //WRITE DATA

#define  DDR_IOCTRL_VALUE               (0x18B)

#define ALLOPP_DDR3_READ_LATENCY          0x07 // RD_Latency = (CL + 2) - 1; CL=6 for DDR3-1600 speed grade and CK of 2.5ns

//******************************************************************
//EMIF parameters
//******************************************************************
// GEL
// 400MHz settings
#define ALLOPP_DDR3_SDRAM_TIMING1 0x0AAAD4DB
#define ALLOPP_DDR3_SDRAM_TIMING2 0x266B7FDA
#define ALLOPP_DDR3_SDRAM_TIMING3 0x501F867F

#define ALLOPP_DDR3_SDRAM_CONFIG 0x61C05332 // termination = 1 (RZQ/4)
                                            // dynamic ODT = 2 (RZQ/2)
                                            // SDRAM drive = 0 (RZQ/6)
                                            // CWL = 0 (CAS write latency = 5)
                                            // CL = 4 (CAS latency = 6)
                                            // ROWSIZE = 6 (15 row bits)
                                            // PAGESIZE = 2 (10 column bits)
#define ALLOPP_DDR3_REF_CTRL 0x00000C30 // 400 * 7.8us = 0xC30
#define ALLOPP_DDR3_ZQ_CONFIG 0x50074BE4

/*************************************************************************
 * Function Name: InitDDRPLL
 * Parameters: None
 *
 * Return: None
 *
 * Description: Inits DDR DPLL.  DDR Clock 266MHZ
 *
 *
 *************************************************************************/

static void InitDDRPLL(void)
{
    /*DDR DPLL in baypass*/
    CM_CLKMODE_DPLL_DDR->BIT.DPLL_EN = 4;
    /*Wait DPLL enter bypass mode*/
    while((1 == CM_IDLEST_DPLL_DDR->BIT.ST_DPLL_CLK) ||
          (0 == CM_IDLEST_DPLL_DDR->BIT.ST_MN_BYPASS));
    /*disable Spread Spectrum Clocking*/
    CM_CLKMODE_DPLL_DDR->BIT.DPLL_SSC_EN = 0;
    /*Set DPLL multiplier factor *266*/
    CM_CLKSEL_DPLL_DDR->BIT.DPLL_MULT = 266;
    /*Set DPLL divider factor*/
    CM_CLKSEL_DPLL_DDR->BIT.DPLL_DIV = 24-1;
    /*Set DPLL post-divider factor M2*/
    CM_DIV_M2_DPLL_MPU->BIT.DPLL_CLKOUT_DIV = 1;
    /*MPU_DPLL in baypass*/
    CM_CLKMODE_DPLL_DDR->BIT.DPLL_EN = 7;
    /*Wait DPLL locks*/
    while((0 == CM_IDLEST_DPLL_DDR->BIT.ST_DPLL_CLK) ||
          (1 == CM_IDLEST_DPLL_DDR->BIT.ST_MN_BYPASS));
}

/*************************************************************************
 * Function Name: InitDDR
 * Parameters: None
 *
 * Return: None
 *
 * Description:
 *
 *
 *************************************************************************/
static void InitDDREMIF(void) {
    uint32_t i;

    /*Enable GPIO Interface Clock*/
    CM_WKUP_L4WKUP_CLKCTRL->BIT.MODULEMODE = 2;
    CM_WKUP_GPIO0_CLKCTRL->BIT.MODULEMODE = 2;
    /*Wait clocks to get active*/
    while(CM_WKUP_GPIO0_CLKCTRL->BIT.IDLEST ||
          CM_WKUP_L4WKUP_CLKCTRL->BIT.IDLEST);

    /*Enable Func Clock*/
    CM_PER_EMIF_CLKCTRL->BIT.MODULEMODE = 2;
    /*Wait clock to get active*/
    while(CM_PER_EMIF_CLKCTRL->BIT.IDLEST);

    /*VTT Enable*/
    /*CONF_ECAP0_IN_PWM0_OUT (GPIO0_7) Pin configure*/
    /*pull disabled*/
    CONF_ECAP0_IN_PWM0_OUT->BIT.PUDEN = 1;
    /*Select GPIO mode*/
    CONF_ECAP0_IN_PWM0_OUT->BIT.MMODE = 7;
    /*Make VTT_EN (GPIO0_7) pin output*/
    GPIO0_OE->LONG &= ~(1<<7) ;
    /*Set VTT_EN*/
    GPIO0_SETDATAOUT->LONG = (1<<7);

    /*VTP Init*/
    /*Dynamic VTP compensation mode*/
    VTP_CTRL->LONG = 0;
    VTP_CTRL->BIT.FILTER = 3;
    VTP_CTRL->BIT.ENABLE = 1;
    /*clears flops, start count again*/
    VTP_CTRL->BIT.CLRZ = 0;
    VTP_CTRL->BIT.CLRZ = 1;
    /*wait ready*/
    while(!VTP_CTRL->BIT.READY);

    /*DDR Phy CMD0*/
    CMD0_REG_PHY_CTRL_SLAVE_RATIO_0->LONG = CMD_PHY_CTRL_SLAVE_RATIO;
    CMD0_REG_PHY_INVERT_CLKOUT_0->LONG = CMD_PHY_INVERT_CLKOUT;
    /*DDR Phy CMD1*/
    CMD1_REG_PHY_CTRL_SLAVE_RATIO_0->LONG = CMD_PHY_CTRL_SLAVE_RATIO;
    CMD1_REG_PHY_INVERT_CLKOUT_0->LONG = CMD_PHY_INVERT_CLKOUT;
    /*DDR Phy CMD2*/
    CMD2_REG_PHY_CTRL_SLAVE_RATIO_0->LONG = CMD_PHY_CTRL_SLAVE_RATIO;
    CMD2_REG_PHY_INVERT_CLKOUT_0->LONG = CMD_PHY_INVERT_CLKOUT;

    /*Init Data Macro 0*/
    DATA0_REG_PHY_RD_DQS_SLAVE_RATIO_0->LONG = DATA_PHY_RD_DQS_SLAVE_RATIO;
    DATA0_REG_PHY_WR_DQS_SLAVE_RATIO_0->LONG = DATA_PHY_WR_DQS_SLAVE_RATIO;
    DATA0_REG_PHY_FIFO_WE_SLAVE_RATIO_0->LONG = DATA_PHY_FIFO_WE_SLAVE_RATIO;
    DATA0_REG_PHY_WR_DATA_SLAVE_RATIO_0->LONG = DATA_PHY_WR_DATA_SLAVE_RATIO;
    /*Init Data Macro 1*/
    DATA1_REG_PHY_RD_DQS_SLAVE_RATIO_0->LONG = DATA_PHY_RD_DQS_SLAVE_RATIO;
    DATA1_REG_PHY_WR_DQS_SLAVE_RATIO_0->LONG = DATA_PHY_WR_DQS_SLAVE_RATIO;
    DATA1_REG_PHY_FIFO_WE_SLAVE_RATIO_0->LONG = DATA_PHY_FIFO_WE_SLAVE_RATIO;
    DATA1_REG_PHY_WR_DATA_SLAVE_RATIO_0->LONG = DATA_PHY_WR_DATA_SLAVE_RATIO;

    /*Rank 0 delays are used for all ranks. This must be set to 0 for DDR3*/
    DATA0_REG_PHY_USE_RANK0_DELAYS->LONG = 0;
    DATA1_REG_PHY_USE_RANK0_DELAYS->LONG = 0;

    /**/
    DDR_CMD0_IOCTRL->LONG = DDR_IOCTRL_VALUE;
    DDR_CMD1_IOCTRL->LONG = DDR_IOCTRL_VALUE;
    DDR_CMD2_IOCTRL->LONG = DDR_IOCTRL_VALUE;
    DDR_DATA0_IOCTRL->LONG = DDR_IOCTRL_VALUE;
    DDR_DATA1_IOCTRL->LONG = DDR_IOCTRL_VALUE;
    /**/

    // IO to work for DDR3
    DDR_IO_CTRL->LONG &= ~0x10000000;

    // CKE controlled by EMIF/DDR_PHY
    DDR_CKE_CTRL->LONG = 0x00000001;

    for(i=0; i<50000; i++) asm("    nop");

    /**/
    EMIF_DDR_PHY_CTRL_1->LONG = ALLOPP_DDR3_READ_LATENCY;
    EMIF_DDR_PHY_CTRL_1_SHDW->LONG = ALLOPP_DDR3_READ_LATENCY;
    EMIF_DDR_PHY_CTRL_2->LONG = ALLOPP_DDR3_READ_LATENCY;
    /**/
    EMIF_SDRAM_TIM_1->LONG =  ALLOPP_DDR3_SDRAM_TIMING1;
    EMIF_SDRAM_TIM_1_SHDW->LONG = ALLOPP_DDR3_SDRAM_TIMING1;
    EMIF_SDRAM_TIM_2->LONG = ALLOPP_DDR3_SDRAM_TIMING2;
    EMIF_SDRAM_TIM_2_SHDW->LONG = ALLOPP_DDR3_SDRAM_TIMING2;
    EMIF_SDRAM_TIM_3->LONG = ALLOPP_DDR3_SDRAM_TIMING3;
    EMIF_SDRAM_TIM_3_SHDW->LONG = ALLOPP_DDR3_SDRAM_TIMING3;

    EMIF_SDRAM_REF_CTRL->LONG = ALLOPP_DDR3_REF_CTRL;
    EMIF_SDRAM_REF_CTRL_SHDW->LONG = ALLOPP_DDR3_REF_CTRL;

    EMIF_ZQ_CONFIG->LONG = ALLOPP_DDR3_ZQ_CONFIG;
    EMIF_SDRAM_CONFIG->LONG = ALLOPP_DDR3_SDRAM_CONFIG;

    while(!EMIF_STATUS->BIT.REG_PHY_DLL_READY);
}

volatile uint32_t *ddr = (volatile uint32_t *)0x80000000;

#define TEST_SIZE 8

void InitDDR(void) {
    InitDDRPLL();
    InitDDREMIF();
}

void DDRTest(void) {
    uint32_t expected, read;
    int i;
    
    for(i = 0; i < TEST_SIZE; i++) {
        expected = 0x01010101 << i;
        ddr[i] = expected;
    }
    for(i = 0; i < TEST_SIZE; i++) {
        expected = 0x01010101 << i;
        read = ddr[i]; (void)read; // GCC bug
        printf("[DDR0] Read %08lX from %p, expected  %08lX\n", read, (void*)&ddr[i], expected);
    }
}

