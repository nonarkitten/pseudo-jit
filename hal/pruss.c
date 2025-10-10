/*
 * hw_init.c
 *
 *  Created on: Feb 9, 2022
 *      Author: Renee Cousins
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "pinmux.h"
#include "pru.h"
#include "ioam3358.h"

#include "pru_data.h"

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

uint32_t* PRUGetAddress(PRU_RAM_t MemoryType) {
    static const uint32_t addr[] = {
        SOC_PRUICSS_PRU0_DRAM_OFFSET  + SOC_PRUICSS1_REGS,
        SOC_PRUICSS_PRU1_DRAM_OFFSET  + SOC_PRUICSS1_REGS,
        SOC_PRUICSS_PRU0_IRAM_OFFSET  + SOC_PRUICSS1_REGS,
        SOC_PRUICSS_PRU1_IRAM_OFFSET  + SOC_PRUICSS1_REGS,
        SOC_PRUICSS_SHARED_RAM_OFFSET + SOC_PRUICSS1_REGS
    };
    return (uint32_t*)addr[(int)MemoryType];
}

void PRUMemSet(PRU_RAM_t MemoryType, uint32_t offset, uint32_t Length, uint32_t Pattern) {
    if(Length == 0 || Length > 8192) return;
    if((offset + Length) > 8192) return;
    uint32_t* start = PRUGetAddress(MemoryType) + offset;
    Pattern = 0x01010101 * (Pattern & 0xFF);
    Length = (Length + 3) >> 2;
    for(int i=0; i<Length; i++) *start++ = Pattern;
    //memset(PRUGetAddress(MemoryType) + offset, Pattern, Length);
}

void PRUMemCpy(PRU_RAM_t MemoryType, uint32_t offset, uint32_t Length, const uint32_t *Pointer) {
    if(Length == 0 || Length > 8192) return;
    if((offset + Length) > 8192) return;
    if(Pointer < 0x402F0400 || Pointer > 0x4030FFFF) return;
    // memcpy(PRUGetAddress(MemoryType) + offset, Pointer, Length);
    uint32_t* dest = PRUGetAddress(MemoryType) + offset;
    uint32_t* src = Pointer;
    Length = (Length + 3) >> 2;
    for(int i=0; i<Length; i++) *dest++ = *src++;
}

void PRUReset(PRU_CORE_t PRUCore) {
    if(PRUCore & PRU0) PRU0_CTRL->LONG = 0;
    if(PRUCore & PRU1) PRU1_CTRL->LONG = 0;
}

void PRUEnable(PRU_CORE_t PRUCore) {
    if(PRUCore & PRU0) PRU0_CTRL->BIT.ENABLE = 1;
    if(PRUCore & PRU1) PRU1_CTRL->BIT.ENABLE = 1;
}

void PRUHalt(PRU_CORE_t PRUCore) {
    if(PRUCore & PRU0) PRU0_CTRL->BIT.ENABLE = 0;
    if(PRUCore & PRU1) PRU1_CTRL->BIT.ENABLE = 0;
}

void PRUFullReset(PRU_CORE_t PRUCore) {
    if(PRUCore & PRU0) {
        PRUHalt(PRU0);
        PRUMemSet(PRU0_DRAM, 0, 8*1024, 0);
        PRUMemSet(PRU0_IRAM, 0, 8*1024, 0);
        PRUMemCpy(PRU0_IRAM, 0, pru_text_bin_len, pru_text_bin);
        PRUMemCpy(PRU0_DRAM, 0, pru_data_bin_len, pru_data_bin);
        PRUReset(PRU0);
        PRUEnable(PRU0);
    }
}

// GreenPAK -> (13) -> GPIO_SUPE -> (B15) -> MMC0_CLK (pr1_pru0_pru_r30_12)
// GreenPAK -> (15) -> GPIO_IACK -> (B16) -> MMC0_CMD (pr1_pru0_pru_r30_13)

void InitPRU(void) {
    static const pin_muxing_t pru_pins[] = {
		/* INPUTS */
		{ PINMUX_CONF_MCASP0_ACLKR,     (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M6) }, /* CLK7    pr1_pru0_pru_r31_4   BBB_CLK7  -> M68K_CLK7   */
        { PINMUX_CONF_MMC0_CLK,         (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M6) }, /* DTACK   pr1_pru0_pru_r31_12  GPIO_SUPE -> GreenPAK 13 */
		{ PINMUX_CONF_MMC0_CMD,         (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M6) }, /* ASn     pr1_pru0_pru_r30_13  GPIO_IACK -> GreenPAK 15 */
        { PINMUX_CONF_XDMA_EVENT_INTR1, (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M5) }, /* VPAn    pr1_pru0_pru_r31_16  PRU_VPA   -> M68K_VPA    */

		/* OUTPUTS */
        { PINMUX_CONF_MCASP0_ACLKX,     (PIN_CFG_PDIS | PIN_CFG_M5) },                /* VMAn    pr1_pru0_pru_r30_0   PRU_VMA   -> M68K_VMA    */
        { PINMUX_CONF_MCASP0_AXR1,      (PIN_CFG_PDIS | PIN_CFG_M5) },                /* WAIT0n  pr1_pru0_pru_r30_6   PRU_EWAIT -> GreenPAK 2  */
        { PINMUX_CONF_MCASP0_AHCLKX,    (PIN_CFG_PDIS | PIN_CFG_M5) },                /* CLK_E   pr1_pru0_pru_r30_7   PRU_ECLK  -> M58K_ECLK   */

        { 0xFFFFFFFF, 0xFFFFFFFF },
    };    
    /* bring PRU_ICSS module out of reset */
    RM_PER_RSTCTRL->LONG  &= 0xFFFFFFFD;            

    /* Enable clocks */
    CM_PER_PRU_ICSS_CLKSTCTRL->BIT.CLKTRCTRL = 2;
    while(CM_PER_PRU_ICSS_CLKSTCTRL->BIT.CLKTRCTRL != 2);
    CM_PER_PRU_ICSS_CLKSTCTRL->BIT.CLKACTIVITY_PRUSS_OCP_GCLK = 1;

    /* Enable the module */
    CM_PER_PRU_ICSS_CLKCTRL->BIT.MODULEMODE = 2;
    /* Check for module enable status */
    while(CM_PER_PRU_ICSS_CLKCTRL->BIT.MODULEMODE != 2);

    /* Check clock activity - ungated */
    while(!CM_PER_PRU_ICSS_CLKSTCTRL->BIT.CLKACTIVITY_PRUSS_OCP_GCLK);
    /* Check idle status value - should be in functional state */
    while(CM_PER_PRU_ICSS_CLKCTRL->BIT.IDLEST);

    /* Clear out the memory of both PRU cores */
    // PRUMemSet(PRU0_DRAM, 0, 8*1024, 0);   //Data 8KB RAM0
    // PRUMemSet(PRU1_DRAM, 0, 8*1024, 0);   //Data 8KB RAM1
    // PRUMemSet(PRU0_IRAM, 0, 8*1024, 0);
    // PRUMemSet(PRU1_IRAM, 0, 8*1024, 0); 

    /*Configure pins*/
    config_mux(pru_pins);

    WaitMSDMTimer(40); 

    PRUFullReset(PRU0);
    // PRUHalt(PRU0);
    // PRUMemCpy(PRU0_IRAM, 0, pru_text_bin_len, pru_text_bin);
    // PRUMemCpy(PRU0_DRAM, 0, pru_data_bin_len, pru_data_bin);
    // PRUReset(PRU0);
    // PRUEnable(PRU0);

}

