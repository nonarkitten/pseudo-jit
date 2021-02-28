/**
 * Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * This module is based on the software library developped by Texas Instruments
 * Incorporated - http://www.ti.com/ for its AM335x starter kit.
 *
 * Project: HEIA-FR†/ Embedded Systems 1+2 Laboratory
 *
 * Abstract: AM335x Clocking Driver
 *
 * Purpose: This module implements basic services to drive the AM335x
 *          clocking module.
 *
 * Author:  Daniel Gachet
 * Date:    03.08.2016
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "clock.h"

#define printf(...)

// define am335x clock module PER registers
static volatile struct am335x_cm_per_ctrl {
    uint32_t l4ls_clkstctrl;       // 00
    uint32_t l3s_clkstctrl;        // 04
    uint32_t l4fw_clkstctrl;       // 08
    uint32_t l3_clkstctrl;         // 0c
    uint32_t res1[1];              // 10-10
    uint32_t cpgmac0_clkctrl;      // 14
    uint32_t lcdc_clkctrl;         // 18
    uint32_t usb0_clkctrl;         // 1c
    uint32_t mlb_clkctrl;          // 20
    uint32_t tptc0_clkctrl;        // 24
    uint32_t emif_clkctrl;         // 28
    uint32_t ocmcram_clkctrl;      // 2c
    uint32_t gpmc_clkctrl;         // 30
    uint32_t mcasp0_clkctrl;       // 34
    uint32_t uart5_clkctrl;        // 38
    uint32_t mmc0_clkctrl;         // 3c
    uint32_t elm_clkctrl;          // 40
    uint32_t i2c2_clkctrl;         // 44
    uint32_t i2c1_clkctrl;         // 48
    uint32_t spi0_clkctrl;         // 4c
    uint32_t spi1_clkctrl;         // 50
    uint32_t res2[3];              // 54-5c
    uint32_t l4ls_clkctrl;         // 60
    uint32_t l4fw_clkctrl;         // 64
    uint32_t mcasp1_clkctrl;       // 68
    uint32_t uart1_clkctrl;        // 6c
    uint32_t uart2_clkctrl;        // 70
    uint32_t uart3_clkctrl;        // 74
    uint32_t uart4_clkctrl;        // 78
    uint32_t timer7_clkctrl;       // 7c
    uint32_t timer2_clkctrl;       // 80
    uint32_t timer3_clkctrl;       // 84
    uint32_t timer4_clkctrl;       // 88
    uint32_t res3[1];              // 8c-8c
    uint32_t rng_clkctrl;          // 90
    uint32_t aes0_clkctrl;         // 94
    uint32_t res4[2];              //†98-9c
    uint32_t sha0_clkctrl;         // a0
    uint32_t pka_clkctrl;          // a4
    uint32_t gpio6_clkctrl;        // a8
    uint32_t gpio1_clkctrl;        // ac
    uint32_t gpio2_clkctrl;        // b0
    uint32_t gpio3_clkctrl;        // b4
    uint32_t res5[1];              // b8-b8
    uint32_t tpcc_clkctrl;         // bc
    uint32_t dcan0_clkctrl;        // c0
    uint32_t dcan1_clkctrl;        // c4
    uint32_t res6[1];              // c8-c8
    uint32_t epwmss1_clkctrl;      // cc
    uint32_t emif_fw_clkctrl;      // d0
    uint32_t epwmss0_clkctrl;      // d4
    uint32_t epwmss2_clkctrl;      // d8
    uint32_t l3_instr_clkctrl;     // dc
    uint32_t l3_clkctrl;           // e0
    uint32_t ieee5000_clkctrl;     // e4
    uint32_t icss_clkctrl;         // e8
    uint32_t timer5_clkctrl;       // ec
    uint32_t timer6_clkctrl;       // f0
    uint32_t mmc1_clkctrl;         // f4
    uint32_t mmc2_clkctrl;         // f8
    uint32_t tptc1_clkctrl;        // fc
    uint32_t tptc2_clkctrl;        // 100
    uint32_t res7[2];              // 104-108
    uint32_t spinlock_clkctrl;     // 10c
    uint32_t mailbox0_clkctrl;     // 110
    uint32_t res8[2];              // 114-118
    uint32_t l4hs_clkstctrl;       // 11c
    uint32_t l4hs_clkctrl;         // 120
    uint32_t mstr_exps_clkctrl;    // 124
    uint32_t slv_exps_clkctrl;     // 128
    uint32_t ocpwp_l3_clkstctrl;   // 12c
    uint32_t ocpwp_clkctrl;        // 130
    uint32_t res9[3];              // 134-13c
    uint32_t icss_clkstctrl;       // 140
    uint32_t cpsw_clkstctrl;       // 144
    uint32_t lcdc_clkstctrl;       // 148
    uint32_t clkdiv32k_clkctrl;    // 14c
    uint32_t clk_24mhz_clkstctrl;  // 150
}* per = (struct am335x_cm_per_ctrl*)0x44e00000;

// CM PER L3_CLKSTCTRL register bit definition
#define L3_CLKSTCTRL_CLKACTIVITY_MCASP_GCLK (1 << 7)
#define L3_CLKSTCTRL_CLKACTIVITY_CPTS_RFT_GCLK (1 << 6)
#define L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK (1 << 4)
#define L3_CLKSTCTRL_CLKACTIVITY_MMC_FCLK (1 << 3)
#define L3_CLKSTCTRL_CLKACTIVITY_EMIF_GCLK (1 << 2)

// CM†PER OCPWP_L3_CLKSTCTRL register bit definition
#define OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L4_GCLK (1 << 5)
#define OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK (1 << 4)

// CM†PER†OCPWP_L3_CLKSTCTRL register bit definition
#define L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK (1 << 3)

static volatile struct am335x_cm_wkup_ctrl {
    uint32_t wkup_clkstctrl;             // 00
    uint32_t wkup_control_clkctrl;       // 04
    uint32_t wkup_gpio0_clkctrl;         // 08
    uint32_t wkup_l4wkup_clkctrl;        // 0c
    uint32_t wkup_timer0_clkctrl;        // 10
    uint32_t wkup_debugss_clkctrl;       // 14
    uint32_t l3_aon_clkstctrl;           // 18
    uint32_t autoidle_dpll_mpu;          // 1c
    uint32_t idlest_dpll_mpu;            // 20
    uint32_t ssc_deltamstep_dpll_mpu;    // 24
    uint32_t ssc_modfreqdiv_dpll_mpu;    // 28
    uint32_t clksel_dpll_mpu;            // 2c
    uint32_t autoidle_dpll_ddr;          // 30
    uint32_t idlest_dpll_ddr;            // 34
    uint32_t ssc_deltamstep_dpll_ddr;    // 38
    uint32_t ssc_modfreqdiv_dpll_ddr;    // 3c
    uint32_t clksel_dpll_ddr;            // 40
    uint32_t autoidle_dpll_disp;         // 44
    uint32_t idlest_dpll_disp;           // 48
    uint32_t ssc_deltamstep_dpll_disp;   // 4c
    uint32_t ssc_modfreqdiv_dpll_disp;   // 50
    uint32_t clksel_dpll_disp;           // 54
    uint32_t autoidle_dpll_core;         // 58
    uint32_t idlest_dpll_core;           // 5c
    uint32_t ssc_deltamstep_dpll_core;   // 60
    uint32_t ssc_modfreqdiv_dpll_core;   // 64
    uint32_t clksel_dpll_core;           // 68
    uint32_t autoidle_dpll_per;          // 6c
    uint32_t idlest_dpll_per;            // 70
    uint32_t ssc_deltamstep_dpll_per;    // 74
    uint32_t ssc_modfreqdiv_dpll_per;    // 78
    uint32_t clkdcoldo_dpll_per;         // 7c
    uint32_t div_m4_dpll_core;           // 80
    uint32_t div_m5_dpll_core;           // 84
    uint32_t clkmode_dpll_mpu;           // 88
    uint32_t clkmode_dpll_per;           // 8c
    uint32_t clkmode_dpll_core;          // 90
    uint32_t clkmode_dpll_ddr;           // 94
    uint32_t clkmode_dpll_disp;          // 98
    uint32_t clksel_dpll_periph;         // 9c
    uint32_t div_m2_dpll_ddr;            // a0
    uint32_t div_m2_dpll_disp;           // a4
    uint32_t div_m2_dpll_mpu;            // a8
    uint32_t div_m2_dpll_per;            // ac
    uint32_t wkup_m3_clkctrl;            // b0
    uint32_t wkup_uart0_clkctrl;         // b4
    uint32_t wkup_i2c0_clkctrl;          // b8
    uint32_t wkup_adc_tsc_clkctrl;       // bc
    uint32_t wkup_smartreflex0_clkctrl;  // c0
    uint32_t wkup_timer1_clkctrl;        // c4
    uint32_t wkup_smartreflex1_clkctrl;  // c8
    uint32_t l4_wkup_aon_clkstctrl;      // cc
    uint32_t wkup_wdt0_clkctrl;          // d0
    uint32_t wkup_wdt1_clkctrl;          // d4
    uint32_t div_m6_dpll_core;           // d8
}* wkup = (struct am335x_cm_wkup_ctrl*)0x44e00400;

// CM WKUP CLKSTCTRL register bit definition
#define WKUP_CLKSTCTRL_CLKACTIVITY_ADC_FCLK (1 << 14)
#define WKUP_CLKSTCTRL_CLKACTIVITY_TIMER1_GFCLK (1 << 13)
#define WKUP_CLKSTCTRL_CLKACTIVITY_UART0_GFCLK (1 << 12)
#define WKUP_CLKSTCTRL_CLKACTIVITY_I2C0_GFCLK (1 << 11)
#define WKUP_CLKSTCTRL_CLKACTIVITY_TIMER0_GFCLK (1 << 10)
#define WKUP_CLKSTCTRL_CLKACTIVITY_WDT0_GCL (1 << 9)
#define WKUP_CLKSTCTRL_CLKACTIVITY_GPIO0_GDBCLK (1 << 8)
#define WKUP_CLKSTCTRL_CLKACTIVITY_WDT1_GCLK (1 << 4)
#define WKUP_CLKSTCTRL_CLKACTIVITY_SR_SYSCLK (1 << 3)
#define WKUP_CLKSTCTRL_CLKACTIVITY_L4_WKUP_GCLK (1 << 2)

// L3_AON_CLKSTCTRL register bit definition
#define L3_AON_CLKSTCTRL_CLKACTIVITY_DEBUG_CLKA (1 << 4)
#define L3_AON_CLKSTCTRL_CLKACTIVITY_L3_AON_GCLK (1 << 3)
#define L3_AON_CLKSTCTRL_CLKACTIVITY_DBGSYSCLK (1 << 2)

// L4_WKUP_AON_CLKSTCTRL register bit definition
#define L4_WKUP_AON_CLKSTCTRL_CLKACTIVITY_L4_WKUP_AON_GCLK (1 << 2)

// GPIOx_CLKCTRL register bit definition
#define OPTFCLKEN_GPIOX_GDBCLK (1 << 18)

#define CM_PER_TPCC_CLKCTRL_MODULEMODE_ENABLE (0x00000002u)
#define CM_PER_TPTC0_CLKCTRL_MODULEMODE_ENABLE (0x00000002u)
#define CM_PER_TPTC1_CLKCTRL_MODULEMODE_ENABLE (0x00000002u)
#define CM_PER_TPTC2_CLKCTRL_MODULEMODE_ENABLE (0x00000002u)

#define CM_PER_TPCC_CLKCTRL_IDLEST (0x3 << 16)
#define CM_PER_TPTC0_CLKCTRL_IDLEST (0x3 << 16)
#define CM_PER_TPTC1_CLKCTRL_IDLEST (0x3 << 16)
#define CM_PER_TPTC2_CLKCTRL_IDLEST (0x3 << 16)
#define CM_PER_CPGMAC0_CLKCTRL_IDLEST (0x3 << 16)

#define CM_PER_TPCC_CLKCTRL_IDLEST_FUNC (0x0)
#define CM_PER_TPTC0_CLKCTRL_IDLEST_FUNC (0x0)
#define CM_PER_TPTC1_CLKCTRL_IDLEST_FUNC (0x0)
#define CM_PER_TPTC2_CLKCTRL_IDLEST_FUNC (0x0)
#define CM_PER_CPGMAC0_CLKCTRL_IDLEST_FUNC (0x0)

#define CM_PER_TPCC_CLKCTRL_STBYST (0x1 << 18)
#define CM_PER_TPTC0_CLKCTRL_STBYST (0x1 << 18)
#define CM_PER_TPTC1_CLKCTRL_STBYST (0x1 << 18)
#define CM_PER_TPTC2_CLKCTRL_STBYST (0x1 << 18)

#define CM_PER_TPCC_CLKCTRL_STBYST_FUNC (0x0)
#define CM_PER_TPTC0_CLKCTRL_STBYST_FUNC (0x0)
#define CM_PER_TPTC1_CLKCTRL_STBYST_FUNC (0x0)
#define CM_PER_TPTC2_CLKCTRL_STBYsST_FUNC (0x0)

#define CM_PER_CPSW_CLKSTCTRL_CLKACTIVITY_CPSW_125MHZ_GCLK (1 << 4)

// CM DPLL Registers // see SPRUH73L chap. 8.1.2.3 / p. 1283
static volatile struct am335x_cm_dpll_ctrl {
    uint32_t res0[1];                  // 00
    uint32_t clksel_timer7_clk;        //†04
    uint32_t clksel_timer2_clk;        //†08
    uint32_t clksel_timer3_clk;        //†0c
    uint32_t clksel_timer4_clk;        //†10
    uint32_t cm_mac_clksel;            // 14
    uint32_t clksel_timer5_clk;        //†18
    uint32_t clksel_timer6_clk;        //†1c
    uint32_t cm_cpts_rft_clksel;       // 20
    uint32_t res1[1];                  // 24
    uint32_t clksel_timer1ms_clk;      // 28
    uint32_t clksel_gfx_fclk;          // 2c
    uint32_t clksel_pru_icss_ocp_clk;  // 30
    uint32_t clksel_lcdc_pixel_clk;    // 34
    uint32_t clksel_wdt1_clk;          // 38
    uint32_t clksel_gpio0_dbclk;       // 3c
}* dpll = (struct am335x_cm_dpll_ctrl*)0x44e00500;

/* --------------------------------------------------------------------------
 * implementation of the internal methods
 * -------------------------------------------------------------------------- */

/**
 * method to enable module clock and wait until performed
 * 
 *@param ctrl address of the controller register
 */
static inline void enable_module(volatile uint32_t* ctrl)
{
    *ctrl = 0x02000000;
    while ((*ctrl & 0x03030000) != 0x02000000);
}

/* -------------------------------------------------------------------------- */

/**
 * method to wake-up a module and wait until performed
 *
 *@param ctrl address of the controller register
 */
static inline void wkup_module(volatile uint32_t* ctrl)
{
    *ctrl = 0x02000000;
    while ((*ctrl & 0x03000000) != 0x02000000)
        ;
}

/* -------------------------------------------------------------------------- */

/**
 * method to wait until a flag is set
 *
 *@param ctrl address of the controller register
 *@param bit bit to wait for
 */
static inline void wait4bit(volatile uint32_t* ctrl, uint32_t bit)
{
    while ((*ctrl & __builtin_bswap32(bit)) == 0)
        ;
}

/* -------------------------------------------------------------------------- */
  
/**
 * method to wait until module enabled
 *
 *@param ctrl address of the controller register
 */
static inline void wait4mode(volatile uint32_t* ctrl)
{
    while ((*ctrl & 0x03000000) != 0x02000000)
        ;
}

/* -------------------------------------------------------------------------- */

/**
 * method to set a specific bit and wait until performed
 *
 *@param ctrl address of the controller register
 *@param bit bit mask to set
 */
static inline void setbit(volatile uint32_t* ctrl, uint32_t bit)
{
    bit = __builtin_bswap32(bit);
    *ctrl |= bit;
    while ((*ctrl & bit) == 0)
        ;
}

/* --------------------------------------------------------------------------
 * implementation of the public methods
 * -------------------------------------------------------------------------- */

void am335x_clock_enable_l3_l4wkup(void)
{
    printf("...Entering %s\n", __func__);
    printf("...Enable L3 module clock\n");
    enable_module(&per->l3_clkctrl);
    printf("...Enable L3 instruction clock\n");
    enable_module(&per->l3_instr_clkctrl);

    wkup_module(&per->l3_clkstctrl);
    wkup_module(&per->ocpwp_l3_clkstctrl);
    wkup_module(&per->l3s_clkstctrl);

    printf("...Wait until modules are active\n");
    wait4bit(&per->l3_clkstctrl, L3_CLKSTCTRL_CLKACTIVITY_L3_GCLK);
    ///†GAC wait4bit (&per->ocpwp_l3_clkstctrl,
    ///OCPWP_L3_CLKSTCTRL_CLKACTIVITY_OCPWP_L3_GCLK);
    wait4bit(&per->l3s_clkstctrl, L3S_CLKSTCTRL_CLKACTIVITY_L3S_GCLK);

    printf("...Wake-up regions\n");
    enable_module(&wkup->wkup_control_clkctrl);

    wkup_module(&wkup->wkup_clkstctrl);
    wkup_module(&wkup->l3_aon_clkstctrl);

    printf("...Wait until region are waked-up\n");
    wait4bit(&wkup->l3_aon_clkstctrl, L3_AON_CLKSTCTRL_CLKACTIVITY_L3_AON_GCLK);
    wait4mode(&wkup->wkup_l4wkup_clkctrl);
    wait4bit(&wkup->wkup_clkstctrl, WKUP_CLKSTCTRL_CLKACTIVITY_L4_WKUP_GCLK);
    wait4bit(&wkup->l4_wkup_aon_clkstctrl,
             L4_WKUP_AON_CLKSTCTRL_CLKACTIVITY_L4_WKUP_AON_GCLK);
    printf("...Exiting %s\n", __func__);
}

/* -------------------------------------------------------------------------- */

void am335x_clock_enable_uart_module(enum am335x_clock_uart_modules module)
{
    switch (module) {
        case AM335X_CLOCK_UART0:
            // enable uart0 module clock & wait until performed
            enable_module(&wkup->wkup_uart0_clkctrl);
            wait4bit(&wkup->wkup_clkstctrl,
                     WKUP_CLKSTCTRL_CLKACTIVITY_UART0_GFCLK);
            break;

        case AM335X_CLOCK_UART1:
            while (1)
                ;
            break;
        case AM335X_CLOCK_UART2:
            while (1)
                ;
            break;
        case AM335X_CLOCK_UART3:
            while (1)
                ;
            break;
        case AM335X_CLOCK_UART4:
            enable_module(&per->uart4_clkctrl);
            break;
        case AM335X_CLOCK_UART5:
            enable_module(&per->uart5_clkctrl);
            break;
    }
}

/* -------------------------------------------------------------------------- */

void am335x_clock_enable_gpio_module(enum am335x_clock_gpio_modules module)
{
    switch (module) {
        case AM335X_CLOCK_GPIO0:
            enable_module(&wkup->wkup_gpio0_clkctrl);
            wait4bit(&wkup->wkup_clkstctrl,
                     WKUP_CLKSTCTRL_CLKACTIVITY_GPIO0_GDBCLK);
            setbit(&wkup->wkup_gpio0_clkctrl, OPTFCLKEN_GPIOX_GDBCLK);
            break;

        case AM335X_CLOCK_GPIO1:
            enable_module(&per->gpio1_clkctrl);
            setbit(&per->gpio1_clkctrl, OPTFCLKEN_GPIOX_GDBCLK);
            break;

        case AM335X_CLOCK_GPIO2:
            enable_module(&per->gpio2_clkctrl);
            setbit(&per->gpio2_clkctrl, OPTFCLKEN_GPIOX_GDBCLK);
            break;

        case AM335X_CLOCK_GPIO3:
            enable_module(&per->gpio3_clkctrl);
            setbit(&per->gpio3_clkctrl, OPTFCLKEN_GPIOX_GDBCLK);
            break;
    }
}

/* -------------------------------------------------------------------------- */

void am335x_clock_enable_i2c_module(enum am335x_clock_i2c_modules module)
{
    switch (module) {
        case AM335X_CLOCK_I2C0:
            enable_module(&wkup->wkup_i2c0_clkctrl);
            wait4bit(&wkup->wkup_clkstctrl,
                     WKUP_CLKSTCTRL_CLKACTIVITY_I2C0_GFCLK);
            break;

        case AM335X_CLOCK_I2C1:
            enable_module(&per->i2c1_clkctrl);
            break;

        case AM335X_CLOCK_I2C2:
            enable_module(&per->i2c2_clkctrl);
            break;
    }
}

/* -------------------------------------------------------------------------- */

void am335x_clock_enable_spi_module(enum am335x_clock_spi_modules module)
{
    switch (module) {
        case AM335X_CLOCK_SPI0:
            enable_module(&per->spi0_clkctrl);
            break;

        case AM335X_CLOCK_SPI1:
            enable_module(&per->spi1_clkctrl);
            break;
    }
}

/* -------------------------------------------------------------------------- */

void am335x_clock_enable_timer_module(enum am335x_clock_timer_modules module)
{
    switch (module) {
        case AM335X_CLOCK_TIMER0:
            enable_module(&wkup->wkup_timer0_clkctrl);
            wait4bit(&wkup->wkup_clkstctrl,
                     WKUP_CLKSTCTRL_CLKACTIVITY_TIMER0_GFCLK);
            break;

        case AM335X_CLOCK_TIMER1:
            enable_module(&wkup->wkup_timer1_clkctrl);
            wait4bit(&wkup->wkup_clkstctrl,
                     WKUP_CLKSTCTRL_CLKACTIVITY_TIMER1_GFCLK);
            dpll->clksel_timer1ms_clk = 0x0;
            break;

        case AM335X_CLOCK_TIMER2:
            enable_module(&per->timer2_clkctrl);
            dpll->clksel_timer2_clk = 0x1;
            break;

        case AM335X_CLOCK_TIMER3:
            enable_module(&per->timer3_clkctrl);
            dpll->clksel_timer3_clk = 0x1;
            break;

        case AM335X_CLOCK_TIMER4:
            enable_module(&per->timer4_clkctrl);
            dpll->clksel_timer4_clk = 0x1;
            break;

        case AM335X_CLOCK_TIMER5:
            enable_module(&per->timer5_clkctrl);
            dpll->clksel_timer5_clk = 0x1;
            break;

        case AM335X_CLOCK_TIMER6:
            enable_module(&per->timer6_clkctrl);
            dpll->clksel_timer6_clk = 0x1;
            break;

        case AM335X_CLOCK_TIMER7:
            enable_module(&per->timer7_clkctrl);
            dpll->clksel_timer7_clk = 0x1;
            break;
    }
}

/* -----------------------------------------------------------------------------------
 */

void am335x_clock_enable_mmc_module(enum am335x_clock_mmc_modules module)
{
    switch (module) {
        case AM335X_CLOCK_MMC0:
            // enable clock for the given module
            enable_module(&per->mmc0_clkctrl);
            // This field indicates the state of the MMC_GCLK clock in the
            // domain
            wait4bit(&per->l3_clkstctrl, L3_CLKSTCTRL_CLKACTIVITY_MMC_FCLK);
            break;

        case AM335X_CLOCK_MMC1:
            enable_module(&per->mmc1_clkctrl);
            wait4bit(&per->l3_clkstctrl, L3_CLKSTCTRL_CLKACTIVITY_MMC_FCLK);
            break;

        case AM335X_CLOCK_MMC2:
            enable_module(&per->mmc2_clkctrl);
            wait4bit(&per->l3_clkstctrl, L3_CLKSTCTRL_CLKACTIVITY_MMC_FCLK);
            break;
    }
}

/* -----------------------------------------------------------------------------------
 */

void am335x_clock_enable_epwm_module(enum am335x_clock_epwm_modules module)
{
    switch (module) {
        case AM335X_CLOCK_EPWM0:
            // enable clock for the given module
            enable_module(&per->epwmss0_clkctrl);
            *(volatile uint32_t*)(0x44e10000 + 0x664) |= (1 << 0);
            break;

        case AM335X_CLOCK_EPWM1:
            enable_module(&per->epwmss1_clkctrl);
            *(volatile uint32_t*)(0x44e10000 + 0x664) |= (1 << 1);
            break;

        case AM335X_CLOCK_EPWM2:
            enable_module(&per->epwmss2_clkctrl);
            *(volatile uint32_t*)(0x44e10000 + 0x664) |= (1 << 2);
            break;
    }
}

/* -----------------------------------------------------------------------------------
 */

void am335x_clock_enable_edma_module()
{
    uint32_t* addr = NULL;

    // enable module tpcc
    enable_module(&per->tpcc_clkctrl);
    wait4bit(&per->tpcc_clkctrl, CM_PER_TPCC_CLKCTRL_MODULEMODE_ENABLE);

    // enable module tptc0
    enable_module(&per->tptc0_clkctrl);
    wait4bit(&per->tptc0_clkctrl, CM_PER_TPTC0_CLKCTRL_MODULEMODE_ENABLE);

    // enable module tptc1
    enable_module(&per->tptc1_clkctrl);
    wait4bit(&per->tptc1_clkctrl, CM_PER_TPTC1_CLKCTRL_MODULEMODE_ENABLE);

    // enable module tptc2
    enable_module(&per->tptc2_clkctrl);
    wait4bit(&per->tptc2_clkctrl, CM_PER_TPTC2_CLKCTRL_MODULEMODE_ENABLE);

    // DMA in non-idle mode
    addr  = (uint32_t*)0x49800010;
    *addr = (uint32_t)0x00000028;
    addr  = (uint32_t*)0x49900010;
    *addr = (uint32_t)0x00000028;
    addr  = (uint32_t*)0x49a00010;
    *addr = (uint32_t)0x00000028;

    // wait for functionnal mode ACK
    while ((per->tpcc_clkctrl & CM_PER_TPCC_CLKCTRL_IDLEST) !=
           CM_PER_TPCC_CLKCTRL_IDLEST_FUNC)
        ;

    while ((per->tptc0_clkctrl & CM_PER_TPCC_CLKCTRL_IDLEST) !=
           CM_PER_TPCC_CLKCTRL_IDLEST_FUNC)
        ;
    while ((per->tptc0_clkctrl & CM_PER_TPCC_CLKCTRL_STBYST) !=
           CM_PER_TPCC_CLKCTRL_STBYST_FUNC)
        ;

    while ((per->tptc1_clkctrl & CM_PER_TPCC_CLKCTRL_IDLEST) !=
           CM_PER_TPCC_CLKCTRL_IDLEST_FUNC)
        ;
    while ((per->tptc1_clkctrl & CM_PER_TPCC_CLKCTRL_STBYST) !=
           CM_PER_TPCC_CLKCTRL_STBYST_FUNC)
        ;

    while ((per->tptc2_clkctrl & CM_PER_TPCC_CLKCTRL_IDLEST) !=
           CM_PER_TPCC_CLKCTRL_IDLEST_FUNC)
        ;
    while ((per->tptc2_clkctrl & CM_PER_TPCC_CLKCTRL_STBYST) !=
           CM_PER_TPCC_CLKCTRL_STBYST_FUNC)
        ;
}

/* -----------------------------------------------------------------------------------
 */

void am335x_clock_enable_cpsw_module(void)
{
    wkup_module(&per->cpgmac0_clkctrl);
    while ((per->cpgmac0_clkctrl & CM_PER_CPGMAC0_CLKCTRL_IDLEST) !=
           CM_PER_CPGMAC0_CLKCTRL_IDLEST_FUNC)
        ;

    wkup_module(&per->cpsw_clkstctrl);
    wait4bit(&per->cpsw_clkstctrl,
             CM_PER_CPSW_CLKSTCTRL_CLKACTIVITY_CPSW_125MHZ_GCLK);
}
