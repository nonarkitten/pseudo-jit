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
#include "pru_text.h" 

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

void InitPRU(void) {
    static const pin_muxing_t pru_pins[] = {
        { PINMUX_CONF_MCASP0_ACLKR,     (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M6) }, /* CLK7_INP  pr1_pru0_pru_r31_4  */
        { PINMUX_CONF_MCASP0_AXR1,      (PIN_CFG_PDIS | PIN_CFG_M5) },                /* CLK7_OUT  pr1_pru0_pru_r30_6  */
        { PINMUX_CONF_MCASP0_AHCLKX,    (PIN_CFG_PDIS | PIN_CFG_M5) },                /* ECLK      pr1_pru0_pru_r30_7  */
        { PINMUX_CONF_MCASP0_ACLKX,     (PIN_CFG_PDIS | PIN_CFG_M5) },                /* VMA       pr1_pru0_pru_r30_0  */
        { PINMUX_CONF_XDMA_EVENT_INTR1, (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M5) }, /* VPA       pr1_pru0_pru_r31_16 */
        
		{ PINMUX_CONF_GPMC_OEN_REN,     (PIN_CFG_PDIS | PIN_CFG_M7) },                /* GPMC_OEN  gpio2_3   */

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
    PRUMemSet(PRU0_DRAM, 0, 8*1024, 0);   //Data 8KB RAM0
    PRUMemSet(PRU1_DRAM, 0, 8*1024, 0);   //Data 8KB RAM1
    PRUMemSet(PRU0_IRAM, 0, 8*1024, 0);
    PRUMemSet(PRU1_IRAM, 0, 8*1024, 0);

    /*Configure pins*/
    config_mux(pru_pins);

    WaitMSDMTimer(40); 

    PRUHalt(PRU0);
    PRUMemCpy(PRU0_IRAM, 0, pru_text_bin_len, pru_text_bin);
    PRUMemCpy(PRU0_DRAM, 0, pru_data_bin_len, pru_data_bin);
    PRUReset(PRU0);
    PRUEnable(PRU0);

}

