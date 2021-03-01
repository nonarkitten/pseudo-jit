/*
	Initialize the board.

	Adapted from bootloader for the am335x beaglebone
	Copyright (C) 2013 Paul Quevedo

	This is part of the Buffee Project Firmware (PJIT)
	Copyright (C) 2021 Renee Cousins

*/

#include "globalDefs.h"
#include "am335x.h"
#include "hardware.h"
#include "ff.h"
#include "xmodem.h"

static void delay(volatile uint32_t count) {
    while (count--) continue;
}

static void disable_watchdog(void) {
    /* Disable watchdog */
    WDT_WSPR = 0xAAAA;
    while (WDT_WWPS & WDT_WWPS_W_PEND_WSPR)
        ;
    WDT_WSPR = 0x5555;
    while (WDT_WWPS & WDT_WWPS_W_PEND_WSPR)
        ;
}

static void enable_clocks(void) {
    /* L3 & L4 Interconnect clocks */
    /* Enable module/instance level clocks */
    CM_MODULEMODE_ENABLE(CM_PER_L3_CLKCTRL);
    CM_MODULEMODE_ENABLE(CM_PER_L4LS_CLKCTRL);
    CM_MODULEMODE_ENABLE(CM_PER_L4FW_CLKCTRL);
    CM_MODULEMODE_ENABLE(CM_PER_L4HS_CLKCTRL);
    CM_MODULEMODE_ENABLE(CM_PER_L3_INSTR_CLKCTRL);
    CM_MODULEMODE_ENABLE(CM_WKUP_L4WKUP_CLKCTRL);

    /* Enable domain level clocks */
    CM_PER_L3_CLKSTCTRL   = CM_CLKTRCTRL_SW_WKUP;
    CM_PER_L4LS_CLKSTCTRL = CM_CLKTRCTRL_SW_WKUP;
    CM_WKUP_CLKSTCTRL     = CM_CLKTRCTRL_SW_WKUP;
    CM_PER_L4FW_CLKSTCTRL = CM_CLKTRCTRL_SW_WKUP;
    CM_PER_L3S_CLKSTCTRL  = CM_CLKTRCTRL_SW_WKUP;

    /* Enable control module clock */
    CM_MODULEMODE_ENABLE(CM_WKUP_CONTROL_CLKCTRL);
}

static void pll_core_init(void) {
    /* Core PLL Init as per s8.1.6.7.1 of am335x TRM */

    /* 1. Switch PLL to bypass mode */
    volatile uint32_t value;
    value  = CM_CLKMODE_DPLL_CORE   & ~CM_DPLL_EN_MASK;
    value |= CM_DPLL_EN_MN_BYP_MODE << CM_DPLL_EN_SHFT;
    CM_CLKMODE_DPLL_CORE = value;

    /* 2. Wait for PLL bypass */
    while (!(CM_IDLEST_DPLL_CORE & CM_DPLL_ST_MN_BYPASS)) continue;

    /* 3. Configure PLL mult and div */
    value  = CM_CLKSEL_DPLL_CORE & ~(CM_DPLL_MULT_MASK | CM_DPLL_DIV_MASK);
    value |= CORE_PLL_M << CM_DPLL_MULT_SHFT;
    value |= CORE_PLL_N << CM_DPLL_DIV_SHFT;
    CM_CLKSEL_DPLL_CORE = value;

    /* 4. Configure M4,5,6 Divider */
    value  = CM_DIV_M4_DPLL_CORE & ~CM_DPLL_HSDIVIDER_DIV_MASK;
    value |= CORE_PLL_M4 << CM_DPLL_HSDIVIDER_DIV_SHFT;
    CM_DIV_M4_DPLL_CORE  = value;

    value  = CM_DIV_M5_DPLL_CORE & ~CM_DPLL_HSDIVIDER_DIV_MASK;
    value |= CORE_PLL_M5 << CM_DPLL_HSDIVIDER_DIV_SHFT;
    CM_DIV_M5_DPLL_CORE  = value;

    value  = CM_DIV_M6_DPLL_CORE & ~CM_DPLL_HSDIVIDER_DIV_MASK;
    value |= CORE_PLL_M6 << CM_DPLL_HSDIVIDER_DIV_SHFT;
    CM_DIV_M6_DPLL_CORE  = value;

    /* 5. Switch PLL to lock mode */
    value  = CM_CLKMODE_DPLL_CORE & ~CM_DPLL_EN_MASK;
    value |= CM_DPLL_EN_LOCK_MODE << CM_DPLL_EN_SHFT;
    CM_CLKMODE_DPLL_CORE = value;

    /* 6. Wait for pll to lock */
    while (!(CM_IDLEST_DPLL_CORE & CM_DPLL_ST_DPLL_CLK)) continue;
}

static void pll_peripheral_init(void) {
    /* Peripheral PLL Init as per s8.1.6.8.1 of am335x TRM */

    /* 1. Switch PLL to bypass mode */
    volatile uint32_t value;
    value  = CM_CLKMODE_DPLL_PER    & ~CM_DPLL_EN_MASK;
    value |= CM_DPLL_EN_MN_BYP_MODE << CM_DPLL_EN_SHFT;
    CM_CLKMODE_DPLL_PER = value;

    /* 2. Wait for PLL bypass */
    while (!(CM_IDLEST_DPLL_PER & CM_DPLL_ST_MN_BYPASS)) continue;

    /* 3. Configure PLL mult and div */
    value  = CM_CLKSEL_DPLL_PERIPH & ~(CM_DPLL_MULT_MASK | CM_DPLL_DIV_MASK);
    value |= PER_PLL_M << CM_DPLL_MULT_SHFT;
    value |= PER_PLL_N << CM_DPLL_DIV_SHFT;
    CM_CLKSEL_DPLL_PERIPH = value;

    /* 4. Configure M2 Divider */
    value  = CM_DIV_M2_DPLL_PER & ~CM_DPLL_M2_CLKOUT_DIV_MASK;
    value |= PER_PLL_M2 << CM_DPLL_M2_CLKOUT_DIV_SHFT;
    CM_DIV_M2_DPLL_PER = value;

    /* 5. Switch PLL to lock mode */
    value  = CM_CLKMODE_DPLL_PER  & ~CM_DPLL_EN_MASK;
    value |= CM_DPLL_EN_LOCK_MODE << CM_DPLL_EN_SHFT;
    CM_CLKMODE_DPLL_PER = value;

    /* 6. Wait for pll to lock */
    while (!(CM_IDLEST_DPLL_PER & CM_DPLL_ST_DPLL_CLK)) continue;
}

static void pll_mpu_init(void) {
    /* MPU (MicroProcessor) PLL Init as per s8.1.6.9.1 of am335x TRM */

    /* 1. Switch PLL to bypass mode */
    volatile uint32_t value;
    value  = CM_CLKMODE_DPLL_MPU    & ~CM_DPLL_EN_MASK;
    value |= CM_DPLL_EN_MN_BYP_MODE << CM_DPLL_EN_SHFT;
    CM_CLKMODE_DPLL_MPU = value;

    /* 2. Wait for PLL bypass */
    while (!(CM_IDLEST_DPLL_MPU & CM_DPLL_ST_MN_BYPASS)) continue;

    /* 3. Configure PLL mult and div */
    value  = CM_CLKSEL_DPLL_MPU & ~(CM_DPLL_MULT_MASK | CM_DPLL_DIV_MASK);
    value |= MPU_PLL_M << CM_DPLL_MULT_SHFT;
    value |= MPU_PLL_N << CM_DPLL_DIV_SHFT;
    CM_CLKSEL_DPLL_MPU = value;

    /* 4. Configure M2 Divider */
    value  = CM_DIV_M2_DPLL_MPU & ~CM_DPLL_M2_CLKOUT_DIV_MASK;
    value |= MPU_PLL_M2 << CM_DPLL_M2_CLKOUT_DIV_SHFT;
    CM_DIV_M2_DPLL_MPU = value;

    /* 5. Switch PLL to lock mode */
    value  = CM_CLKMODE_DPLL_MPU  & ~CM_DPLL_EN_MASK;
    value |= CM_DPLL_EN_LOCK_MODE << CM_DPLL_EN_SHFT;
    CM_CLKMODE_DPLL_MPU = value;

    /* 6. Wait for pll to lock */
    while (!(CM_IDLEST_DPLL_MPU & CM_DPLL_ST_DPLL_CLK)) continue;
}

static void pll_ddr_init(void) {
    /* DDR PLL Init as per s8.1.6.11.1 of the am335x TRM */
    /* 1. Switch PLL to bypass mode */
    volatile uint32_t value;
    value  = CM_CLKMODE_DPLL_DDR    & ~CM_DPLL_EN_MASK;
    value |= CM_DPLL_EN_MN_BYP_MODE << CM_DPLL_EN_SHFT;
    CM_CLKMODE_DPLL_DDR = value;

    /* 2. Wait for PLL to be in bypass */
    while (!(CM_IDLEST_DPLL_DDR & CM_DPLL_ST_MN_BYPASS)) continue;

    /* 3. Configure PLL mult and div */
    value  = CM_CLKSEL_DPLL_DDR & ~(CM_DPLL_MULT_MASK | CM_DPLL_DIV_MASK);
    value |= DDR_PLL_M << CM_DPLL_MULT_SHFT;
    value |= DDR_PLL_N << CM_DPLL_DIV_SHFT;
    CM_CLKSEL_DPLL_DDR = value;

    /* 4. Configure M2 Divider */
    value  = CM_DIV_M2_DPLL_DDR & ~CM_DPLL_M2_CLKOUT_DIV_MASK;
    value |= DDR_PLL_M2 << CM_DPLL_M2_CLKOUT_DIV_SHFT;
    CM_DIV_M2_DPLL_DDR = value;

    /* 5. Switch PLL to lock mode */
    value  = CM_CLKMODE_DPLL_DDR  & ~CM_DPLL_EN_MASK;
    value |= CM_DPLL_EN_LOCK_MODE << CM_DPLL_EN_SHFT;
    CM_CLKMODE_DPLL_DDR = value;

    /* 6. Wait for PLL to lock */
    while (!(CM_IDLEST_DPLL_DDR & CM_DPLL_ST_DPLL_CLK)) continue;
}

static void emif_init(void) {
    uint32_t mask;

    CM_MODULEMODE_ENABLE(CM_PER_EMIF_FW_CLKCTRL);
    CM_MODULEMODE_ENABLE(CM_PER_EMIF_CLKCTRL);

    mask = CM_CLKSTCTRL_CLKACTIVITY_EMIF_GCLK
         | CM_CLKSTCTRL_CLKACTIVITY_L3_GCLK;

    while ((CM_PER_L3_CLKSTCTRL & mask) != mask) continue;
}

static void ddr2_init(void) {
	/* This is just taken from the DDR2Init function from TI Starterware. */
	/* http://processors.wiki.ti.com/index.php/AM335x_EMIF_Configuration_tips */

    CTRLM_VTP_CTRL |=  CTRLM_VTP_CTRL_ENABLE;
    CTRLM_VTP_CTRL &= ~CTRLM_VTP_CTRL_CLRZ;
    CTRLM_VTP_CTRL |=  CTRLM_VTP_CTRL_CLRZ;
    while (!(CTRLM_VTP_CTRL & CTRLM_VTP_CTRL_READY)) continue;

    /* DDR PHY CMD0 Register configuration */
    DDRPHY_CMD0_SLAVE_RATIO_0   = 0x80;
    DDRPHY_CMD0_SLAVE_FORCE_0   = 0x0;
    DDRPHY_CMD0_SLAVE_DELAY_0   = 0x0;
    DDRPHY_CMD0_LOCK_DIFF_0     = 0x4;
    DDRPHY_CMD0_INVERT_CLKOUT_0 = 0x0;

    /* DDR PHY CMD1 Register configuration */
    DDRPHY_CMD1_SLAVE_RATIO_0   = 0x80;
    DDRPHY_CMD1_SLAVE_FORCE_0   = 0x0;
    DDRPHY_CMD1_SLAVE_DELAY_0   = 0x0;
    DDRPHY_CMD1_LOCK_DIFF_0     = 0x4;
    DDRPHY_CMD1_INVERT_CLKOUT_0 = 0x0;

    /* DDR PHY CMD2 Register configuration */
    DDRPHY_CMD2_SLAVE_RATIO_0   = 0x80;
    DDRPHY_CMD2_SLAVE_FORCE_0   = 0x0;
    DDRPHY_CMD2_SLAVE_DELAY_0   = 0x0;
    DDRPHY_CMD2_LOCK_DIFF_0     = 0x4;
    DDRPHY_CMD2_INVERT_CLKOUT_0 = 0x0;

    /* DATA macro configuration */
    DDRPHY_DATA0_RD_DQS_SLAVE_RATIO_0  = 0x04010040;
    DDRPHY_DATA0_RD_DQS_SLAVE_RATIO_1  = 0x00000010;
    DDRPHY_DATA0_WR_DQS_SLAVE_RATIO_0  = 0x0;
    DDRPHY_DATA0_WR_DQS_SLAVE_RATIO_1  = 0x0;
    DDRPHY_DATA0_WRLVL_INIT_RATIO_0    = 0x0;
    DDRPHY_DATA0_WRLVL_INIT_RATIO_1    = 0x0;
    DDRPHY_DATA0_GATELVL_INIT_RATIO_0  = 0x0;
    DDRPHY_DATA0_GATELVL_INIT_RATIO_1  = 0x0;
    DDRPHY_DATA0_FIFO_WE_SLAVE_RATIO_0 = 0x85615856;
    DDRPHY_DATA0_FIFO_WE_SLAVE_RATIO_1 = 0x00000015;
    DDRPHY_DATA0_WR_DATA_SLAVE_RATIO_0 = 0x04010040;
    DDRPHY_DATA0_WR_DATA_SLAVE_RATIO_1 = 0x00000010;
    DDRPHY_DATA0_LOCK_DIFF_0           = 0x0;

    DDRPHY_DATA1_RD_DQS_SLAVE_RATIO_0  = 0x04010040;
    DDRPHY_DATA1_RD_DQS_SLAVE_RATIO_1  = 0x00000010;
    DDRPHY_DATA1_WR_DQS_SLAVE_RATIO_0  = 0x0;
    DDRPHY_DATA1_WR_DQS_SLAVE_RATIO_1  = 0x0;
    DDRPHY_DATA1_WRLVL_INIT_RATIO_0    = 0x0;
    DDRPHY_DATA1_WRLVL_INIT_RATIO_1    = 0x0;
    DDRPHY_DATA1_GATELVL_INIT_RATIO_0  = 0x0;
    DDRPHY_DATA1_GATELVL_INIT_RATIO_1  = 0x0;
    DDRPHY_DATA1_FIFO_WE_SLAVE_RATIO_0 = 0x85615856;
    DDRPHY_DATA1_FIFO_WE_SLAVE_RATIO_1 = 0x00000015;
    DDRPHY_DATA1_WR_DATA_SLAVE_RATIO_0 = 0x04010040;
    DDRPHY_DATA1_WR_DATA_SLAVE_RATIO_1 = 0x00000010;
    DDRPHY_DATA1_LOCK_DIFF_0           = 0x0;

    DDRPHY_DATA0_USE_RANK0_DELAYS_0    = 1;
    DDRPHY_DATA1_USE_RANK0_DELAYS_0    = 1;
}

static void uart_init(void) {
	/* Enable UART */
	static const uartCfg_t uartCfg = {
		.baud = BAUD_115200,
		.fifo = { 
			.enable = TRUE,
			.rxTrig = 1,
			.txTrig = 1
		}
	};
    uartConfig(UART_CONSOLE, &uartCfg);
}

void board_init(void) {
	/* Initialize everything we need before we can print anything */
	//disable_watchdog();
	enable_clocks();
	pll_core_init();
    pll_peripheral_init();
    pll_mpu_init();
    pll_ddr_init();
    emif_init();
    ddr2_init();
	uart_init();
}
