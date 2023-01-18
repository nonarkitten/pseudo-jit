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

#ifndef PINMUX_H_
#define PINMUX_H_

#include "main.h"

/*
 * IEN  - Input Enable
 * IDIS - Input Disable
 * PTD  - Pull type Down
 * PTU  - Pull type Up
 * DIS  - Pull type selection is inactive
 * EN   - Pull type selection is active
 * M0   - Mode 0
 * The commented string gives the final mux configuration for that pin
 */
#define PIN_CFG_IEN     (1 << 5)
#define PIN_CFG_IDIS    (0 << 5)
#define PIN_CFG_PTU     (1 << 4)
#define PIN_CFG_PTD     (0 << 4)
#define PIN_CFG_EN      (1 << 3)
#define PIN_CFG_DIS     (0 << 3)

#define PIN_CFG_M0      (0 << 0)
#define PIN_CFG_M1      (1 << 0)
#define PIN_CFG_M2      (2 << 0)
#define PIN_CFG_M3      (3 << 0)
#define PIN_CFG_M4      (4 << 0)
#define PIN_CFG_M5      (5 << 0)
#define PIN_CFG_M6      (6 << 0)
#define PIN_CFG_M7      (7 << 0)

#define CFG_MOD_BASE       (0x44E10000)      //REVISIT

#define CONF_GPMC_AD0          (0x800)
#define CONF_GPMC_AD1          (0x804)
#define CONF_GPMC_AD2          (0x808)
#define CONF_GPMC_AD3          (0x80C)
#define CONF_GPMC_AD4          (0x810)
#define CONF_GPMC_AD5          (0x814)
#define CONF_GPMC_AD6          (0x818)
#define CONF_GPMC_AD7          (0x81C)
#define CONF_GPMC_AD8          (0x820)
#define CONF_GPMC_AD9          (0x824)
#define CONF_GPMC_AD10         (0x828)
#define CONF_GPMC_AD11         (0x82C)
#define CONF_GPMC_AD12         (0x830)
#define CONF_GPMC_AD13         (0x834)
#define CONF_GPMC_AD14         (0x838)
#define CONF_GPMC_AD15         (0x83C)

#define CONF_GPMC_A0           (0x840)
#define CONF_GPMC_A1           (0x844)
#define CONF_GPMC_A2           (0x848)
#define CONF_GPMC_A3           (0x84C)
#define CONF_GPMC_A4           (0x850)
#define CONF_GPMC_A5           (0x854)
#define CONF_GPMC_A6           (0x858)
#define CONF_GPMC_A7           (0x85C)
#define CONF_GPMC_A8           (0x860)
#define CONF_GPMC_A9           (0x864)
#define CONF_GPMC_A10          (0x868)
#define CONF_GPMC_A11          (0x86C)

#define CONF_GPMC_WAIT0        (0x870)
#define CONF_GPMC_WPN          (0x874)
#define CONF_GPMC_BEN1         (0x878)
#define CONF_GPMC_CSN0         (0x87C)
#define CONF_GPMC_CSN1         (0x880)
#define CONF_GPMC_CSN2         (0x884)
#define CONF_GPMC_CSN3         (0x888)
#define CONF_GPMC_CLK          (0x88C)
#define CONF_GPMC_ADVN_ALE     (0x890)
#define CONF_GPMC_OEN_REN      (0x894)
#define CONF_GPMC_WEN          (0x898)
#define CONF_GPMC_BEN0_CLE     (0x89C)

#define CONF_LCD_DATA0         (0x8A0)
#define CONF_LCD_DATA1         (0x8A4)
#define CONF_LCD_DATA2         (0x8A8)
#define CONF_LCD_DATA3         (0x8AC)
#define CONF_LCD_DATA4         (0x8B0)
#define CONF_LCD_DATA5         (0x8B4)
#define CONF_LCD_DATA6         (0x8B8)
#define CONF_LCD_DATA7         (0x8BC)
#define CONF_LCD_DATA8         (0x8C0)
#define CONF_LCD_DATA9         (0x8C4)
#define CONF_LCD_DATA10        (0x8C8)
#define CONF_LCD_DATA11        (0x8CC)
#define CONF_LCD_DATA12        (0x8D0)
#define CONF_LCD_DATA13        (0x8D4)
#define CONF_LCD_DATA14        (0x8D8)
#define CONF_LCD_DATA15        (0x8DC)

#define CONF_LCD_VSYNC         (0x8E0)
#define CONF_LCD_HSYNC         (0x8E4)
#define CONF_LCD_PCLK          (0x8E8)
#define CONF_LCD_AC_BIAS_EN    (0x8EC)

#define CONF_MMC0_DAT3         (0x8F0)
#define CONF_MMC0_DAT2         (0x8F4)
#define CONF_MMC0_DAT1         (0x8F8)
#define CONF_MMC0_DAT0         (0x8FC)
#define CONF_MMC0_CLK          (0x900)
#define CONF_MMC0_CMD          (0x904)
#define CONF_MII1_COL          (0x908)
#define CONF_MII1_CRS          (0x90C)
#define CONF_MII1_RX_ER        (0x910)
#define CONF_MII1_TX_EN        (0x914)
#define CONF_MII1_RX_DV        (0x918)
#define CONF_MII1_TXD3         (0x91C)
#define CONF_MII1_TXD2         (0x920)
#define CONF_MII1_TXD1         (0x924)
#define CONF_MII1_TXD0         (0x928)
#define CONF_MII1_TX_CLK       (0x92C)
#define CONF_MII1_RX_CLK       (0x930)
#define CONF_MII1_RXD3         (0x934)
#define CONF_MII1_RXD2         (0x938)
#define CONF_MII1_RXD1         (0x93C)
#define CONF_MII1_RXD0         (0x940)
#define CONF_RMII1_REF_CLK     (0x944)
#define CONF_MDIO              (0x948)
#define CONF_MDC               (0x94C)

#define CONF_SPI0_SCLK         (0x950)
#define CONF_SPI0_D0           (0x954)
#define CONF_SPI0_D1           (0x958)
#define CONF_SPI0_CS0          (0x95C)
#define CONF_SPI0_CS1          (0x960)
#define CONF_ECAP0_IN_PWM0_OUT (0x964)
#define CONF_UART0_CTSN        (0x968)
#define CONF_UART0_RTSN        (0x96C)
#define CONF_UART0_RXD         (0x970)
#define CONF_UART0_TXD         (0x974)
#define CONF_UART1_CTSN        (0x978)
#define CONF_UART1_RTSN        (0x97C)
#define CONF_UART1_RXD         (0x980)
#define CONF_UART1_TXD         (0x984)
#define CONF_I2C0_SDA          (0x988)
#define CONF_I2C0_SCL          (0x98C)

#define CONF_MCASP0_ACLKX      (0x990)
#define CONF_MCASP0_FSX        (0x994)
#define CONF_MCASP0_AXR0       (0x998)
#define CONF_MCASP0_AHCLKR     (0x99C)
#define CONF_MCASP0_ACLKR      (0x9A0)
#define CONF_MCASP0_FSR        (0x9A4)
#define CONF_MCASP0_AXR1       (0x9A8)
#define CONF_MCASP0_AHCLKX     (0x9AC)
#define CONF_XDMA_EVENT_INTR0  (0x9B0)
#define CONF_XDMA_EVENT_INTR1  (0x9B4)
#define CONF_WARMRSTN          (0x9B8)
#define CONF_NNMI              (0x9C0)
#define CONF_TMS               (0x9D0)
#define CONF_TDI               (0x9D4)
#define CONF_TDO               (0x9D8)
#define CONF_TCK               (0x9DC)
#define CONF_TRSTN             (0x9E0)
#define CONF_EMU0              (0x9E4)
#define CONF_EMU1              (0x9E8)
#define CONF_RTC_PWRONRSTN     (0x9F8)
#define CONF_PMIC_POWER_EN     (0x9FC)
#define CONF_EXT_WAKEUP        (0xA00)
#define CONF_USB0_DRVVBUS      (0xA1C)
#define CONF_USB1_DRVVBUS      (0xA34)

typedef struct {
    uint32_t offset;
    uint32_t val;
} pin_muxing_t;

extern void config_mux(const pin_muxing_t* pin_mux);

#endif /* PINMUX_H_ */
