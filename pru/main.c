#include <stdint.h>
#include "pru_cfg.h"

/*
                            AM335x    OSD335x   Buffee      			IO
pr1_pru0_pru_r30_0   PRU0     A13       A1        PRU_VMA				O	MCASP0_ACLKX
pr1_pru0_pru_r31_1   PRU0     B13       A2        BBB_BGACK				I*	MCASP0_FSX
pr1_pru0_pru_r31_2   PRU0     D12       B2        BBB_RESET				I*	MCASP0_AXR0
pr1_pru0_pru_r31_3   PRU0     C12       B1        BBB_BR				I*	MCASP0_AHCLKR
pr1_pru0_pru_r31_4   PRU0     B12       A3        BBB_CLK7				I	MCASP0_ACLKR
pr1_pru0_pru_r30_5   PRU0     C13       B3        BBB_BG				O	MCASP0_FSR
pr1_pru0_pru_r30_6   PRU0     D13       C3        PRU_EWAIT     Y ---.	O	MCASP0_AXR1
pr1_pru0_pru_r30_7   PRU0     A14       C4        PRU_ECLK           |	O	MCASP0_AHCLKX
                                                                     |
pr1_pru0_pru_r30_12  PRU0     G17       B15       GPIO_SUPE     Y -. |		MMC0_CLK
pr1_pru0_pru_r30_13  PRU0     G18       B16       GPIO_IACK     Y -| |		MMC0_CMD
                                                                   | |
pr1_pru0_pru_r31_16  PRU0     D14       B4        BBB_VPA          | |	I*	XDMA_EVENT_INTR1
                                                                   | |                                                                   | |
GreenPAK Code:                                                     | |
                                  _                                | |
M68K_FC0 = ~GPIO_SUPE | GPIO_IACK  \_______________________________| |
M68K_FC1 =  GPIO_SUPE | GPIO_IACK _/                                 |
                                                                     |
GPMC_WAIT0 = (M68K_DTACK & PRU_EWAIT) & !(OE & WE)                   |
                           \_______/                                 |
                                   \_________________________________|

 */

volatile register uint32_t __R30;	/* Output */
volatile register uint32_t __R31;	/* Input */

// typedef struct {
// 	uint32_t inputEcho;
// } bufferData;
// bufferData dmemBuf;

#define CLK7M		(1 << 4)		/* 7M CLK */
#define CLK_E		(1 << 7)		/* E CLKC */

#define EWAIT       (1 << 6)
#define VPA         (1 << 16)       /* Valid Peripheral Address */
#define VMA         (1 << 0)       /* Valid Peripheral Address */

#define wait_high()     while(!(__R31 & CLK7M)) continue
#define wait_low()      while( (__R31 & CLK7M)) continue
#define wait_edge()     do { wait_low(); wait_high(); } while(0)

//volatile uint32_t div = 10;
#define PRU_SHARED_MEM_ADDR 0x00010000

#define do_io() do { \
    /* Set outputs */ \
    __R30 = _e | _vma | _wait; \
    /* Get inputs */ \
    _vpa = __R31 & VPA; \
    /* This block takes care of the VMA signal */ \
    /* which is used to acknowledge to old 6800 */ \
    /* style hardware that a bus transfer has  */ \
    /* happened. Resets when the CPU AS is disasserted */ \
    if (_vpa) __R30 |= _vma = VMA; \
    /* This block delays our CPU */ \
    if (_vma == 0 && _e) _wait = 0; \
    else _wait = EWAIT; \
    } while(0)

void main(void) {
    uint32_t _div = 0;
    uint32_t _q = 4;
    uint32_t _e = 0;
    uint32_t _vma = VMA;
    uint32_t _wait = EWAIT;
    uint32_t _vpa;
    uint32_t _cyc;

    /* Configure GPI and GPO as Mode 0 (Direct Connect) */
    CT_CFG.GPCFG0 = 0x0000;

    volatile uint32_t* smem = (volatile uint32_t*)PRU_SHARED_MEM_ADDR;
    smem[0] = 10;
    smem[1] = 0;
    smem[2] = 0;

    while (1) {
        /* Q0 **************************************************/
        if(!_vpa) _vma = 0;
        /*  The original E-Clock is non-symmetric.
            It is low for six clock cycles and high for four
            We will allow shortening this from 10 downto 6
            for a possible overclock of the CIA bus */
        if (_div > 10) _div = 10; else if (_div < 6) _div = 6;
        do_io();
        wait_edge();

        /* Q1 **************************************************/
        do_io();
        if(_div >= 8) wait_edge();

        /* Q2 **************************************************/
        do_io();
        if(_div >= 10) wait_edge();

        /* Q3 **************************************************/
        smem[1] = _div;
        do_io();
        wait_edge();

        /* Q4 **************************************************/
        _e = CLK_E;
        do_io();
        wait_edge();

        /* Q5 **************************************************/
        do_io();
        if(_div >= 7) wait_edge();

        /* Q6 **************************************************/
        do_io();
        if(_div >= 9) wait_edge();

        /* Q7 **************************************************/
        do_io();
        smem[2]++;
        wait_edge();

        /* Q8 **************************************************/
        _e = 0;
        do_io();
        wait_edge();

        /* Q9 **************************************************/
        _div = smem[0];
        do_io();
        wait_edge();
    } // end while
}
