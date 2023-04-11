#include "main.h"

#include "pru_data.h"
#include "pru_text.h" 

// extern unsigned char pru_data_bin[];
// extern unsigned int pru_data_bin_len;
// extern unsigned char pru_text_bin[];
// extern unsigned int pru_text_bin_len;
static pin_muxing_t pru_pins[] = {
        { CONF_MCASP0_ACLKR,     (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M6) }, /* CLK7  pr1_pru0_pru_r31_4  */
        { CONF_MCASP0_AHCLKX,    (PIN_CFG_PDIS | PIN_CFG_M5) },                /* E-CLK pr1_pru0_pru_r30_7  */
        { CONF_MCASP0_AXR1,      (PIN_CFG_PDIS | PIN_CFG_M5) },                /* EWAIT pr1_pru0_pru_r30_6  */
        { CONF_XDMA_EVENT_INTR1, (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M5) }, /* VPA   pr1_pru0_pru_r31_16 */
        { CONF_MCASP0_ACLKX,     (PIN_CFG_PDIS | PIN_CFG_M5) },                /* VMA   pr1_pru0_pru_r30_0  */
        { 0xFFFFFFFF, 0xFFFFFFFF },
};

void InitPRU(void) {
    PRUInit();
 
    /*Configure pins*/
    config_mux(pru_pins);

    WaitMSDMTimer(40); 

    PRUHalt(PRU0);

    PRUMemCpy(PRU0_IRAM, 0, pru_text_bin_len, pru_text_bin);
    PRUMemCpy(PRU0_DRAM, 0, pru_data_bin_len, pru_data_bin);

    // WaitMSDMTimer(2000);
    PRUReset(PRU0);
    PRUEnable(PRU0);

    // uint32_t* x = (uint32_t*)0x4A322000;
    // for(int i=0; i<11; i++) printf("[PRU0] %p = %08x\n", &x[i], x[i]);

}

