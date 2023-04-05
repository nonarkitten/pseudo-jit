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

/*************************************************************************
 * Function Name: InitCorePLL
 * Parameters: None
 *
 * Return: None
 *
 * Description: Inits Core DPLL. CLKDCOLDO      2000MHZ
 *                               CORE_CLKOUTM6  500MHz
 *                               CORE_CLKOUTM4  200MHz
 *                               CORE_CLKOUTM5  250MHz
 *
 *************************************************************************/
void InitCorePLL(int clock)
{
    /*CORE_DPLL in baypass*/
    CM_CLKMODE_DPLL_CORE->BIT.DPLL_EN = 4;
    /*Wait DPLL enter bypass mode*/
    while((1 == CM_IDLEST_DPLL_CORE->BIT.ST_DPLL_CLK) ||
          (0 == CM_IDLEST_DPLL_CORE->BIT.ST_MN_BYPASS));
    /*disable Spread Spectrum Clocking*/
    CM_CLKMODE_DPLL_CORE->BIT.DPLL_SSC_EN = 0;
    /*Set DPLL multiplier factor *1000*/
    CM_CLKSEL_DPLL_CORE->BIT.DPLL_MULT = clock;
    /*Set DPLL divider factor*/
    CM_CLKSEL_DPLL_CORE->BIT.DPLL_DIV = 24-1;
    /*Set DPLL post-divider factor M4*/
    CM_DIV_M4_DPLL_CORE->BIT.HSDIVIDER_CLKOUT1_DIV = 10;
    /*Set DPLL post-divider factor M5*/
    CM_DIV_M5_DPLL_CORE->BIT.HSDIVIDER_CLKOUT2_DIV = 8;
    /*Set DPLL post-divider factor M6*/
    CM_DIV_M6_DPLL_CORE->BIT.HSDIVIDER_CLKOUT3_DIV = 4;
    /*CORE_DPLL in baypass*/
    CM_CLKMODE_DPLL_CORE->BIT.DPLL_EN = 7;
    /*Wait DPLL locks*/
    while((0 == CM_IDLEST_DPLL_CORE->BIT.ST_DPLL_CLK) ||
          (1 == CM_IDLEST_DPLL_CORE->BIT.ST_MN_BYPASS));
}

/*************************************************************************
 * Function Name: InitMPUPLL
 * Parameters: None
 *
 * Return: None
 *
 * Description: Inits MPU DPLL.  MPU Clock 1000MHZ
 *
 *
 *************************************************************************/
void InitMPUPLL(int clock)
{
    /*MPU_DPLL in baypass*/
    CM_CLKMODE_DPLL_MPU->BIT.DPLL_EN = 4;
    /*Wait DPLL enter bypass mode*/
    while((1 == CM_IDLEST_DPLL_MPU->BIT.ST_DPLL_CLK) ||
          (0 == CM_IDLEST_DPLL_MPU->BIT.ST_MN_BYPASS));
    /*disable Spread Spectrum Clocking*/
    CM_CLKMODE_DPLL_MPU->BIT.DPLL_SSC_EN = 0;
    /*Set DPLL multiplier factor*/
    CM_CLKSEL_DPLL_MPU->BIT.DPLL_MULT = clock;
    /*Set DPLL divider factor*/
    CM_CLKSEL_DPLL_MPU->BIT.DPLL_DIV = 24-1;
    /*Set DPLL post-divider factor M2*/
    CM_DIV_M2_DPLL_MPU->BIT.DPLL_CLKOUT_DIV = 1;
    /*MPU_DPLL in baypass*/
    CM_CLKMODE_DPLL_MPU->BIT.DPLL_EN = 7;
    /*Wait DPLL locks*/
    while((0 == CM_IDLEST_DPLL_MPU->BIT.ST_DPLL_CLK) ||
          (1 == CM_IDLEST_DPLL_MPU->BIT.ST_MN_BYPASS));
}

/*************************************************************************
 * Function Name: InitPERPLL
 * Parameters: None
 *
 * Return: None
 *
 * Description: Inits PER DPLL.  CLKDCOLDO 960MHZ
 *                               CLKOUT    192MHZ
 *
 *************************************************************************/
void InitPERPLL(void)
{
    /*MPU_DPLL in baypass*/
    CM_CLKMODE_DPLL_PER->BIT.DPLL_EN = 4;
    /*Wait DPLL enter bypass mode*/
    while((1 == CM_IDLEST_DPLL_PER->BIT.ST_DPLL_CLK) ||
          (0 == CM_IDLEST_DPLL_PER->BIT.ST_MN_BYPASS));
    /*disable Spread Spectrum Clocking*/
    CM_CLKMODE_DPLL_PER->BIT.DPLL_SSC_EN = 0;
    /*Set DPLL multiplier factor *960*/
    CM_CLKSEL_DPLL_PERIPH->BIT.DPLL_MULT = 960;
    /*Set DPLL divider factor*/
    CM_CLKSEL_DPLL_PERIPH->BIT.DPLL_DIV = 24-1;
    /*Set DPLL Sigma-Delta divider*/
    CM_CLKSEL_DPLL_PERIPH->BIT.DPLL_SD_DIV = 4;
    /*Set DPLL post-divider factor M2*/
    CM_DIV_M2_DPLL_PER->BIT.DPLL_CLKOUT_DIV = 5;
    /*MPU_DPLL in baypass*/
    CM_CLKMODE_DPLL_PER->BIT.DPLL_EN = 7;
    /*Wait DPLL locks*/
    while((0 == CM_IDLEST_DPLL_PER->BIT.ST_DPLL_CLK) ||
          (1 == CM_IDLEST_DPLL_PER->BIT.ST_MN_BYPASS));
}

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

uint32_t ReadDMTimer(void) {
    return (DMTIMER1_1MS_TCRR->LONG);
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
extern void DDR_PLL_Config(uint32_t CLKIN, uint32_t N, uint32_t M, uint32_t M2);

void InitDDRPLL(void)
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

// OSD Recommended
//#define  DATA_PHY_RD_DQS_SLAVE_RATIO    0x3A
//#define  DATA_PHY_WR_DQS_SLAVE_RATIO    0x45
//#define  DATA_PHY_FIFO_WE_SLAVE_RATIO   0x95
//#define  DATA_PHY_WR_DATA_SLAVE_RATIO   0x7F

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

// OSD Recommended
//#define ALLOPP_DDR3_READ_LATENCY    0x00100007
//#define ALLOPP_DDR3_SDRAM_TIMING1   0x0AAAD4DB
//#define ALLOPP_DDR3_SDRAM_TIMING2   0x266B7FDA
//#define ALLOPP_DDR3_SDRAM_TIMING3   0x501F867F
//#define ALLOPP_DDR3_SDRAM_CONFIG    0x61C05332
//#define ALLOPP_DDR3_REF_CTRL        0x00000C30
//#define ALLOPP_DDR3_ZQ_CONFIG       0x50074BE4


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
extern void DDR3_EMIF_Config(void);
void InitDDR(void)
{
    uint32_t i;

    /*Enable Clocks*/
    CM_PER_L3_CLKSTCTRL->BIT.CLKTRCTRL = 2;
    CM_PER_L3_CLKCTRL->BIT.MODULEMODE = 2;
    /*Enable Func Clock*/
    CM_PER_EMIF_CLKCTRL->BIT.MODULEMODE = 2;
    /*Wait clock to get active*/
    while(CM_PER_EMIF_CLKCTRL->BIT.IDLEST);

    /*VTT Enable*/
    /*Enable Control module clocks*/
    CM_WKUP_CLKSTCTRL->BIT.CLKTRCTRL = 2;
    CM_WKUP_CONTROL_CLKCTRL->BIT.MODULEMODE = 2;
    /*Wait clock to get active*/
    while(CM_WKUP_CONTROL_CLKCTRL->BIT.IDLEST);
    /*CONF_ECAP0_IN_PWM0_OUT (GPIO0_7) Pin configure*/
    /*pull disabled*/
    CONF_ECAP0_IN_PWM0_OUT->BIT.PUDEN = 1;
    /*Select GPIO mode*/
    CONF_ECAP0_IN_PWM0_OUT->BIT.MMODE = 7;
    /*Enable GPIO Interface Clock*/
    CM_WKUP_CLKSTCTRL->BIT.CLKTRCTRL = 2;
    CM_WKUP_L4WKUP_CLKCTRL->BIT.MODULEMODE = 2;
    CM_WKUP_GPIO0_CLKCTRL->BIT.MODULEMODE = 2;
    /*Wait clocks to get active*/
    while(CM_WKUP_GPIO0_CLKCTRL->BIT.IDLEST ||
          CM_WKUP_L4WKUP_CLKCTRL->BIT.IDLEST);
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
//    DDR_IO_CTRL->BIT.DDR3_RST_DEF_VAL = 0;
//    DDR_IO_CTRL->BIT.DDR_WUCLK_DISABLE = 0;
//    DDR_IO_CTRL->BIT.MDDR_SEL = 0;
//    DDR_CKE_CTRL->BIT.DDR_CKE_CTRL = 1;


    // IO to work for DDR3
    // WR_MEM_32(DDR_IO_CTRL, RD_MEM_32(DDR_IO_CTRL) & ~0x10000000);
    //printf("[DDR0] DDR_IO_CTRL: %08x\n", DDR_IO_CTRL->LONG);
    DDR_IO_CTRL->LONG &= ~0x10000000;
    //printf("[DDR0] DDR_IO_CTRL: %08x\n", DDR_IO_CTRL->LONG);

    //for(i=0; i<50000; i++) asm("    nop");

    // CKE controlled by EMIF/DDR_PHY
    // WR_MEM_32(DDR_CKE_CTRL, RD_MEM_32(DDR_CKE_CTRL) | 0x00000001);
    //printf("[DDR0] DDR_CKE_CTRL: %08x\n", DDR_CKE_CTRL->LONG);
    DDR_CKE_CTRL->LONG = 0x00000001;
    //*((volatile uint32_t*)0x44E1131C) = 1;
    //printf("[DDR0] DDR_CKE_CTRL: %08x\n", DDR_CKE_CTRL->LONG);


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

#define SYSTEM_CLOCK 48000000 // TODO: Verify!

uint8_t TransferSPI(uint8_t *io_buffer, uint8_t len) {
    MCSPI0_CH0CTRL->BIT.EN = 1;
    MCSPI0_CH0CONF->BIT.FORCE = 1;
    while(len--) {
        while(!MCSPI0_CH0STAT->BIT.TXS);
        MCSPI0_TX0->LONG = *io_buffer;
        while(!MCSPI0_CH0STAT->BIT.RXS);
        *io_buffer++ = MCSPI0_RX0->LONG;
    }
    MCSPI0_CH0CONF->BIT.FORCE = 0;
    MCSPI0_CH0CTRL->BIT.EN = 0;
    return 0; // TODO: add error?
}

void InitSPI(int bus_speed) {
    /*Enable Clocks*/
    CM_PER_SPI0_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_SPI0_CLKCTRL->BIT.IDLEST);

    /*Enable SPI*/
    MCSPI0_SYSCONFIG->BIT.SOFTRESET = 1;
    while(!MCSPI0_SYSSTATUS->BIT.RESETDONE);

    /*Configure clock activity and idle mode*/
    MCSPI0_SYSCONFIG->BIT.SIDLEMODE = 0; // no-idle
    MCSPI0_SYSCONFIG->BIT.CLOCKACTIVITY = 3; // both

    /*Configure module contoller*/
    MCSPI0_MODULCTRL->LONG = (
          (0 << 8)               /* fifo managed with ctrl register */
        | (0 << 7)               /* multiword disabled */
        | (1 << 4)               /* initial spi delay = 4 spi clock */
        | (0 << 3)               /* functional mode */
        | (0 << 2)               /* master mode */
        | (0 << 1)               /* use SPIEN as chip select */
        | (1 << 0)               /* multi channel mode */
        );
    /*Set fifo level to 0*/
    MCSPI0_XFERLEVEL->LONG = 0;

    /*Compute frequency flags*/
    uint32_t clkg   = 0;
    uint32_t clkd   = 0;
    uint32_t extclk = 0;

    /*calculate the frequency ratio*/
    uint32_t ratio = SYSTEM_CLOCK / bus_speed;

    /*If ratio is not a power of 2, set granularity of 1 clock cycle*/
    if ((ratio & (ratio - 1)) != 0) {
        clkg   = 1;
        clkd   = (ratio - 1) & 0xf;
        extclk = (ratio - 1) >> 4;
    } else {
        /* Compute log2 (ratio) */
        while (ratio != 1) {
            ratio /= 2;
            clkd++;
        }
    }

    MCSPI0_CH0CONF->LONG = (
          (clkg << 29)           /* clock granularity */
        | (3 << 25)              /* chip select time control 2.5 cycles */
        | (0 << 21)              /* spienslv = 0 */
        | (0 << 20)              /* force = 0 */
        | (0 << 19)              /* turbo = 0 */
        | (1 << 16)              /* TX on D1, RX on D0 */
        | (0 << 14)              /* DMA transfer disabled */
        | (0 << 12)              /* TX + RX mode */
        | ((8 - 1) << 7)         /* spi word len */
        | (1 << 6)               /* spien polarity = low */
        | (clkd << 2)            /* frequency diviver */
        | (0 << 1)               /* spiclk polarity = high */
        | (0 << 0)               /* spiclk phase = odd */
        );

    /*Configure clock ratio extender*/
    MCSPI0_CH0CTRL->BIT.EXTCLK = extclk;

    /*Configure pins*/
    CONF_SPI0_SCLK->BIT.MMODE = 0;
    CONF_SPI0_SCLK->BIT.RXACTIVE = 1;

    CONF_SPI0_D0->BIT.MMODE = 0;
    CONF_SPI0_D0->BIT.RXACTIVE = 1;
    CONF_SPI0_D0->BIT.PUTYPESEL = 1;

    CONF_SPI0_D1->BIT.MMODE = 0;
    CONF_SPI0_D1->BIT.RXACTIVE = 1;
    CONF_SPI0_D1->BIT.PUTYPESEL = 1;

    CONF_SPI0_CS0->BIT.MMODE = 0;
    CONF_SPI0_CS0->BIT.RXACTIVE = 1;
}

// I2C CON register bit definition
#define CON_I2C_EN (1 << 15)

#define CON_STB  (1 << 11)
#define CON_MST  (1 << 10)
#define CON_TRX  (1 << 9)
#define CON_XSA  (1 << 8)
#define CON_XOA0 (1 << 7)
#define CON_XOA1 (1 << 6)
#define CON_XOA2 (1 << 5)
#define CON_XOA3 (1 << 4)
#define CON_STP  (1 << 1)
#define CON_STT  (1 << 0)

/** public functions **/
/*************************************************************************
 * Function Name: I2C0Init
 * Parameters: None
 *
 * Return: None
 *
 * Description: Inits I2C0 Module.
 *
 *************************************************************************/
#define INTERNAL_CLOCK 12000000

void I2C0Init(uint32_t bus_speed) {
    uint32_t divider = (INTERNAL_CLOCK / 2) / bus_speed;

    /*Enable Interface Clock*/
    CM_WKUP_CLKSTCTRL->BIT.CLKTRCTRL = 2;
    CM_WKUP_L4WKUP_CLKCTRL->BIT.MODULEMODE = 2;
    /*Enable Func Clock*/
    CM_WKUP_I2C0_CLKCTRL->BIT.MODULEMODE = 2;
    /*Wait clocks to get active*/
    while(CM_WKUP_I2C0_CLKCTRL->BIT.IDLEST ||
        CM_WKUP_L4WKUP_CLKCTRL->BIT.IDLEST);

    /*Reset I2C0 module*/
    I2C0_SYSC->BIT.SRST  = 1;
    while(I2C0_SYSC->BIT.SRST);
    /*Disable auto idle*/
    I2C0_SYSC->BIT.AUTOIDLE  = 0;
    /*Prescaler set to 2. ICLK is 24MHz*/
    I2C0_PSC->LONG = (SYSTEM_CLOCK / INTERNAL_CLOCK) - 1;
    /*Set SCLL */
    I2C0_SCLL->LONG = divider - 7;
    /*Set SCLH */
    I2C0_SCLH->LONG = divider - 5;

    /*Pin Config*/

    /*Enable Control module clocks*/
    CM_WKUP_CLKSTCTRL->BIT.CLKTRCTRL = 2;
    CM_WKUP_CONTROL_CLKCTRL->BIT.MODULEMODE = 2;
    /*Wait clock to get active*/
    while(CM_WKUP_CONTROL_CLKCTRL->BIT.IDLEST);
    
    /*I2C0 Enable*/
    I2C0_CON->BIT.I2C_EN = 1;

    I2C0_OA->BIT.OA = 1;

    /*Disable interrupts*/
    I2C0_IRQENABLE_CLR->LONG = I2C_IRQ_ARDY | I2C_IRQ_NACK | I2C_IRQ_AL;
}

static int wait_for_status(void) {
    int status = 0;
    int timeout = 20000;
    while (1) {
        // check for malfunction...
        status = -1;
        if (I2C0_IRQSTATUS_RAW->BIT.AL) break;
        if (I2C0_IRQSTATUS_RAW->BIT.AERR) break;
        if (timeout == 0) break; else timeout--;

        // check for valid bit...
        status = 0;
        if (I2C0_IRQSTATUS_RAW->BIT.BF) break;
    }

    // clear error bits
    I2C0_IRQSTATUS->BIT.AL = 1;
    I2C0_IRQSTATUS->BIT.AERR = 1;

    // return status information
    return status;
}

/*************************************************************************
 * Function Name: I2C0Send
 * Parameters: uint32_t slave - slave address
 *             uint8_t * data - data pointer
 *             uint32_t cntr  - data size
 *             uint32_t stop  - 0 do not send stop condition
 *                              1 send stop condition
 *
 * Return: uint32_t   0 - send o.k.
 *                    1 - send error.
 *
 * Description: Sends data to slave
 *
 *************************************************************************/
int32_t  I2C0SendCmd(uint32_t slave, uint8_t cmd, uint8_t * data, uint32_t cntr)
{
    /*Enable peripheral*/
    I2C0_CON->BIT.I2C_EN = 1;
    /*Check for bus busy*/
    while(I2C0_IRQSTATUS_RAW->BIT.BB);
    /*Set data count*/
    I2C0_CNT->BIT.DCOUNT = cntr + 1;
    /*Set Slave Addr*/
    I2C0_SA->BIT.SA = slave;
    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = 0x7FFF;
    /*Start command*/
    I2C0_CON->LONG = CON_I2C_EN | CON_TRX | CON_MST | CON_STT | CON_STP;

    while(!I2C0_IRQSTATUS_RAW->BIT.XRDY);
    I2C0_DATA->BIT.DATA = cmd;
    I2C0_IRQSTATUS->BIT.XRDY = 1;

    while(cntr) {
        while(!I2C0_IRQSTATUS_RAW->BIT.XRDY);
        I2C0_DATA->BIT.DATA = *data++;
        I2C0_IRQSTATUS->BIT.XRDY = 1;
        cntr--;
    }

    return 0;
}

/*************************************************************************
 * Function Name: I2C0Read
 * Parameters: uint32_t slave - slave address
 *             uint8_t * data - data pointer
 *             uint32_t cntr  - data size
 *
 * Return: uint32_t   0 - read o.k.
 *                    1 - read error.
 *
 * Description:  Reads data from slave
 *
 *************************************************************************/
int32_t  I2C0ReadCmd(uint32_t slave, uint8_t cmd, uint8_t * data, uint32_t cntr)
{
    /*Enable peripheral*/
    I2C0_CON->BIT.I2C_EN = 1;
    /*Flush buffer*/
    (void)I2C0_DATA->BIT.DATA;
    /*Check for bus busy*/
    while(I2C0_IRQSTATUS_RAW->BIT.BB);
    /*Set data count*/
    I2C0_CNT->BIT.DCOUNT = 1;
    /*Set Slave Addr*/
    I2C0_SA->BIT.SA = slave;
    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = 0x7FFF;
    /*Start command*/
    I2C0_CON->LONG = CON_I2C_EN | CON_TRX | CON_MST | CON_STT;

    while(!I2C0_IRQSTATUS_RAW->BIT.XRDY);
    I2C0_DATA->BIT.DATA = cmd;
    I2C0_IRQSTATUS->BIT.XRDY = 1;

    /* TODO check NACK */
    while(!I2C0_IRQSTATUS_RAW->BIT.ARDY);

    I2C0_CNT->BIT.DCOUNT = cntr;
    I2C0_CON->LONG = CON_I2C_EN | CON_MST | CON_STT | CON_STP;

    while(cntr) {
        while(!I2C0_IRQSTATUS_RAW->BIT.RRDY);
        *data++ = I2C0_DATA->BIT.DATA;
        I2C0_IRQSTATUS->BIT.RRDY = 1;
        cntr--;
    }

    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = 0x7FFF;

    return 0;
}

/*************************************************************************
 * Function Name: I2C0Probe
 * Parameters: uint32_t slave - slave address
 *
 * Return: uint32_t   0 - device absent
 *                    1 - device present
 *
 * Description:  Checks if slave is present
 *
 *************************************************************************/
int32_t  I2C0Probe(uint32_t slave) {
    /*Enable peripheral*/
    I2C0_CON->BIT.I2C_EN = 1;
    /*Flush buffer*/
    (void)I2C0_DATA->BIT.DATA;
    /*Check for bus busy*/
    while(I2C0_IRQSTATUS_RAW->BIT.BB);
    /*Set data count*/
    I2C0_CNT->BIT.DCOUNT = 1;
    /*Set Slave Addr*/
    I2C0_SA->BIT.SA = slave;
    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = 0x7FFF;
    /*Start immediate RX*/
    I2C0_CON->LONG = CON_I2C_EN | CON_MST | CON_STT | CON_STP;
    /*Wait until transfer complete and check if done correctly*/
    if ((wait_for_status() == 0)) {
        if(I2C0_IRQSTATUS_RAW->BIT.NACK) return 0;
        //if(I2C0_IRQSTATUS_RAW->BIT.AERR) return 0;
    } 
    //else return 0;
    /*Found, receive dummy byte*/
    while(!I2C0_IRQSTATUS_RAW->BIT.RRDY);
    (void)I2C0_DATA->BIT.DATA;
    /*Clear IRQ Status*/
    I2C0_IRQSTATUS->LONG = I2C0_IRQSTATUS_RAW->LONG;

    return 1;
}


void UART0Init(int baud) {
    /*Enable Func Clock*/
    CM_WKUP_UART0_CLKCTRL->BIT.MODULEMODE = 2;
    /*Wait clocks to get active*/
    while(CM_WKUP_UART0_CLKCTRL->BIT.IDLEST);

    // PAD_CONTROL_RXACTIVE | PAD_CONTROL_PULLUP
    CONF_UART0_RXD->LONG = 0x70;
//    CONF_UART0_RXD->BIT.MMODE = 0;   // Enable GPIO
//    CONF_UART0_RXD->BIT.PUDEN = 0;
//    CONF_UART0_RXD->BIT.PUTYPESEL = 1;
//    CONF_UART0_RXD->BIT.RXACTIVE = 1;
//    CONF_UART0_RXD->BIT.SLEWCTRL = 0;
//
//    // PAD_CONTROL_PULLUP | PAD_CONTROL_SLEWCTRL
    CONF_UART0_TXD->LONG = 0x58;
//    CONF_UART0_TXD->BIT.MMODE = 0;   // Enable GPIO
//    CONF_UART0_TXD->BIT.PUDEN = 0;
//    CONF_UART0_TXD->BIT.PUTYPESEL = 1;
//    CONF_UART0_TXD->BIT.RXACTIVE = 0;
//    CONF_UART0_TXD->BIT.SLEWCTRL = 1;

    /*Reset UART*/
    UART0_SYSC->BIT.SOFTRESET = 1;
    while(UART0_SYSS->BIT.RESETDONE == 0);
    /*Disable all interrupts and line drivers*/
    //UART0_MDR1->BIT.MODE_SELECT = 7;

    float div = 48000000.0f/(16.0f*(float)baud);
    unsigned int intdiv = (unsigned int) div;

    UART0_IER->BYTE =  0x00;         // Disable interrupts
    UART0_MDR1->BYTE = 0x07;         // Disable mode select

    UART0_LCR->BYTE =  ~(0x7C);      // Divisor latch enable, access DLL DHL, set UART as 8bit
    UART0_DLL->BYTE =  0x00;         // DLL = 0
    UART0_DLH->BYTE =  0x00;         // DLH = 0
    UART0_LCR->BYTE =  0x03;         // Set UART 8-bit

    UART0_MCR->BYTE =  0x03;         // Force /rts & /drt to active (low)
    UART0_IIR->BYTE =  0x07;         // Clear FIFOs and enable them

    UART0_LCR->BYTE =  ~(0x7C);      // Divisor latch enable, access DLL DHL, set UART as 8bit
    UART0_DLL->BYTE =  (intdiv & 0xFF);         // DLL = 0
    UART0_DLH->BYTE =  ((intdiv >> 8) & 0x3F);         // DLH = 0
    UART0_LCR->BYTE =  0x03;         // Set UART 8-bit

    UART0_MDR1->BYTE = 0;            // UART 16x oversampleing
}

void UART0Write(int c) {
    while(UART0_SSR->BIT.TX_FIFO_FULL);
    UART0_THR->BYTE = (uint8_t)c;
}

int UART0Ready(void) {
    return !!(UART0_LSR->BIT.RX_FIFO_E);
}

char UART0Read(void) {
    while(!(UART0_LSR->BIT.RX_FIFO_E));
    return UART0_RHR->BYTE;
}

int UART0Getkey(void) {
    if(UART0_LSR->BIT.RX_FIFO_E)
        return UART0_RHR->BYTE;
    else
        return -1;
}

/*  BBB_RESET and BBB_HALT are open drain
    BBB_HALT  is on GPIO0_13
    BBB_RESET is on GPIO3_16
*/

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


void GPMCInit(void) {
    // CM_DIV_M4_DPLL_CORE->BIT.HSDIVIDER_CLKOUT1_DIV = clk_div;
    CM_PER_GPMC_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_GPMC_CLKCTRL->BIT.IDLEST);
    CM_PER_ELM_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_ELM_CLKCTRL->BIT.IDLEST);

    /* Reset GPMC */
    GPMC_SYSCONFIG->BIT.SOFTRESET = 1;
    while(!GPMC_SYSSTATUS->BIT.RESETDONE);

    /* isr's sources masked */
    GPMC_IRQENABLE->LONG = 0;
    /* timeout disable */
    GPMC_TIMEOUT_CONTROL->LONG = 0;
    /* wait polarity */
    GPMC_CONFIG->BIT.WAIT0PINPOLARITY = 1;

    /*
     * Disable the GPMC0 config set by ROM code
     * It conflicts with our MPDB (both at 0x08000000)
     */
    GPMC_CONFIG7_0->LONG = 0;
}

#define GPMC_SIZE_256M  0x0
#define GPMC_SIZE_128M  0x8
#define GPMC_SIZE_64M   0xC
#define GPMC_SIZE_32M   0xE
#define GPMC_SIZE_16M   0xF

void GPMCConfig(const uint32_t config[6], uint32_t cs, uint32_t base, uint32_t size) {
    __gpmc_config7_bits config7;

    // Take an actual size and convert it to an address mask
    config7.MASKADDRESS = (size > 0x08000000) ? GPMC_SIZE_256M :
                         ((size > 0x04000000) ? GPMC_SIZE_128M :
                         ((size > 0x02000000) ? GPMC_SIZE_64M  :
                         ((size > 0x01000000) ? GPMC_SIZE_32M  :
                                                GPMC_SIZE_16M)));

    // Ensure base aligns to the boundary
    config7.BASEADDRESS = (base >> 24) & (0x30 | config7.MASKADDRESS);

    // Prepare for enablement
    config7.CSVALID = 1;

    if(cs == 2) {
        GPMC_CONFIG7_2->LONG = 0;

        WaitUSDMTimer(100);

        GPMC_CONFIG1_2->LONG = config[0];
        GPMC_CONFIG2_2->LONG = config[1];
        GPMC_CONFIG3_2->LONG = config[2];
        GPMC_CONFIG4_2->LONG = config[3];
        GPMC_CONFIG5_2->LONG = config[4];
        GPMC_CONFIG6_2->LONG = config[5];
        GPMC_CONFIG7_2->BIT = config7;
    }

    GPMC_CONFIG->BIT.WAIT0PINPOLARITY = 0;

    GPMC_TIMEOUT_CONTROL->BIT.TIMEOUTENABLE = 1;
    GPMC_TIMEOUT_CONTROL->BIT.TIMEOUTSTARTVALUE = 200;


}


#include "hw_pru.h"

#define SOC_PRUICSS1_REGS               (0x4A300000)

#define SOC_PRUICSS_PRU0_DRAM_OFFSET    (0x00000000)
#define SOC_PRUICSS_PRU1_DRAM_OFFSET    (0x00002000)
#define SOC_PRUICSS_SHARED_RAM_OFFSET   (0x00010000)
#define SOC_PRUICSS_INTC_OFFSET         (0x00020000)
#define SOC_PRUICSS_PRU0_CTRL_OFFSET    (0x00022000)
#define SOC_PRUICSS_PRU0_DBG_OFFSET     (0x00022400)
#define SOC_PRUICSS_PRU1_CTRL_OFFSET    (0x00024000)
#define SOC_PRUICSS_PRU1_DBG_OFFSET     (0x00024400)
#define SOC_PRUICSS_CFG_OFFSET          (0x00026000)
#define SOC_PRUICSS_UART_OFFSET         (0x00028000)
#define SOC_PRUICSS_IEP_OFFSET          (0x0002E000)
#define SOC_PRUICSS_ECAP_OFFSET         (0x00030000)
#define SOC_PRUICSS_PRU0_IRAM_OFFSET    (0x00034000)
#define SOC_PRUICSS_PRU1_IRAM_OFFSET    (0x00038000)

void PRUReset(void) {
}

void PRUMemSet(uint32_t MemoryType, uint32_t offset, uint32_t Length, uint32_t Pattern) {
    uint32_t StartAddress;

    if (MemoryType == PRU0_DRAM)
        StartAddress = SOC_PRUICSS1_REGS + SOC_PRUICSS_PRU0_DRAM_OFFSET;
    else if (MemoryType == PRU1_DRAM)
        StartAddress = SOC_PRUICSS1_REGS + SOC_PRUICSS_PRU1_DRAM_OFFSET;
    else if (MemoryType == PRU0_IRAM)
        StartAddress = SOC_PRUICSS1_REGS + SOC_PRUICSS_PRU0_IRAM_OFFSET;
    else if (MemoryType == PRU1_IRAM)
        StartAddress = SOC_PRUICSS1_REGS + SOC_PRUICSS_PRU1_IRAM_OFFSET;
    else if (MemoryType == PRU_SHARED_RAM)
        StartAddress = SOC_PRUICSS1_REGS + SOC_PRUICSS_SHARED_RAM_OFFSET;
    else return;

    uint8_t *destaddr = (uint8_t*)(StartAddress + offset);
    memset(destaddr, Pattern, Length);
}

void PRUMemCpy(uint32_t MemoryType, uint32_t offset, uint32_t Length, const uint32_t *Pointer) {
    //static const uint32_t BaseAddress = SOC_PRUICSS1_REGS;
    if(Length == 0 || Length > 8192) 
        return;
    if((offset + Length) > 8192) 
        return;
    if(Pointer < 0x402F0400 || Pointer > 0x4030FFFF) 
        return;

    uint32_t StartAddress;
    uint8_t *srcaddr;
    uint8_t *destaddr;

    if (MemoryType == PRU0_DRAM)
        StartAddress = SOC_PRUICSS1_REGS + SOC_PRUICSS_PRU0_DRAM_OFFSET;
    else if (MemoryType == PRU1_DRAM)
        StartAddress = SOC_PRUICSS1_REGS + SOC_PRUICSS_PRU1_DRAM_OFFSET;
    else if (MemoryType == PRU0_IRAM)
        StartAddress = SOC_PRUICSS1_REGS + SOC_PRUICSS_PRU0_IRAM_OFFSET;
    else if (MemoryType == PRU1_IRAM)
        StartAddress = SOC_PRUICSS1_REGS + SOC_PRUICSS_PRU1_IRAM_OFFSET;
    else if (MemoryType == PRU_SHARED_RAM)
        StartAddress = SOC_PRUICSS1_REGS + SOC_PRUICSS_SHARED_RAM_OFFSET;
    else return;

    srcaddr = (uint8_t*)Pointer;
    destaddr = (uint8_t*)(StartAddress + offset);

    memcpy(destaddr, srcaddr, Length);
}

void PRUInit(void) {
    /* Enable PRU Clocks */
    CM_PER_L4LS_CLKSTCTRL->BIT.CLKTRCTRL = 2;
    while(!CM_PER_L4LS_CLKSTCTRL->BIT.CLKTRCTRL);

    /*PRU-ICSS PRCM Enable Step 1*/
    RM_PER_RSTCTRL->BIT.PRU_ICSS_LRST = 2;

    /*PRU-ICSS PRCM Enable Step 2*/
    CM_PER_PRU_ICSS_CLKCTRL->BIT.MODULEMODE = 2;
    while(CM_PER_PRU_ICSS_CLKCTRL->BIT.IDLEST);

    /*PRU-ICSS PRCM Reset*/
    RM_PER_RSTCTRL->BIT.PRU_ICSS_LRST = 2;

    WaitUSDMTimer(100);

    PRUSS_CFG_SYSCFG->LONG = 0x00000005;
    while(PRUSS_CFG_SYSCFG->BIT.SUB_MWAIT);

    /* Clear out the memory of both PRU cores */
    PRUMemSet(PRU0_DRAM, 0, 8*1024, 0);   //Data 8KB RAM0
    PRUMemSet(PRU1_DRAM, 0, 8*1024, 0);   //Data 8KB RAM1
    PRUMemSet(PRU0_IRAM, 0, 8*1024, 0);
    PRUMemSet(PRU1_IRAM, 0, 8*1024, 0);
}

void PRUEnable(PRU_CORE_t PRUCore) {
    if(PRUCore & PRU0) PRU0_CTRL->LONG = 0xB;
    if(PRUCore & PRU1) PRU1_CTRL->LONG = 0xB;
}

void PRUHalt(PRU_CORE_t PRUCore) {
    if(PRUCore & PRU0) PRU0_CTRL->LONG = 0;
    if(PRUCore & PRU1) PRU1_CTRL->LONG = 0;
}

/*

0x231E0 SPI Initialize
0x23230 SPI ReadSectors

 *
 *
 */











