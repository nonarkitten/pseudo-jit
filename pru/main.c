#include <stdint.h>

#include "inc/pru_cfg.h"
#include "inc/pru_registers.h"

/*  GPIO access adapted from Charles Steinkuehler
    pru.main.p

    Original notes:

    PRU GPIO Write Timing Details
    The actual write instruction to a GPIO pin using SBBO takes two 
    PRU cycles (10 nS).  However, the GPIO logic can only update every 
    40 nS (8 PRU cycles).  This meas back-to-back writes to GPIO pins 
    will eventually stall the PRU, or you can execute 6 PRU instructions 
    for 'free' when burst writing to the GPIO.

    Latency from the PRU write to the actual I/O pin changing state
    (normalized to PRU direct output pins = zero latency) when the
    PRU is writing to GPIO1 and L4_PERPort1 is idle measures 
    95 nS or 105 nS (apparently depending on clock synchronization)

    PRU GPIO Posted Writes
    When L4_PERPort1 is idle, it is possible to burst-write multiple
    values to the GPIO pins without stalling the PRU, as the writes 
    are posted.  With an unrolled loop (SBBO to GPIO followed by a 
    single SET/CLR to R30), the first 20 write cycles (both 
    instructions) took 15 nS each, at which point the PRU began
    to stall and the write cycle settled in to the 40 nS maximum
    update frequency.

    PRU GPIO Read Timing Details
    Reading from a GPIO pin when L4_PERPort1 is idle require 165 nS as
    measured using direct PRU I/O updates bracking a LBBO instruction.
    Since there is no speculative execution on the PRU, it is not possible
    to execute any instructions during this time, the PRU just stalls.

    Latency from the physical I/O pin to the PRU read seeing valid data
    has not yet been measured.
*/

/*  HANDLED PINS
                                  AM335x    OSD335x   Buffee      			IO  CONF             DETAILS
                                  ------    -------   --------              --  ---------------- ------------------------------------------------------
    pr1_pru0_pru_r31_4   PRU0     B12       A3        BBB_CLK7				I	MCASP0_ACLKR     68K Clock Input
    pr1_pru0_pru_r30_6   PRU0     D13       C3        PRU_CLK7           	O	MCASP0_AXR1      68K Clock Ouput Echo
    pr1_pru0_pru_r30_7   PRU0     A14       C4        PRU_ECLK            	O	MCASP0_AHCLKX    68K Clock Divided by 10
    pr1_pru0_pru_r30_0   PRU0     A13       A1        PRU_VMA				O	MCASP0_ACLKX     Signal to logic that we're synced with the ECLK
    pr1_pru0_pru_r31_16  PRU0     D14       B4        BBB_VPA             	I	XDMA_EVENT_INTR1 Signal from logic that we're accessing 6800 peripheral

    gpio2_3              GPIO     T7        N1        GPMC_OEN              O   GPMC_OEN_REN     Signal to enforce a wait to GPMC to sync with ECLK
*/

volatile register uint32_t __R30;	/* Output */
volatile register uint32_t __R31;	/* Input */

#define CLK7M_INP   (1 << 4)		/* 7M CLK */
#define CLK_E		(1 << 7)		/* E CLKC */

#define CLK7M_OUT   (1 << 6)
#define VPA         (1 << 16)       /* Valid Peripheral Address */
#define VMA         (1 << 0)        /* Valid Peripheral Address */

#define EWAIT       (1 << 3)        /* Wait for E Clock sync */

#define wait_high()     while(!(__R31 & CLK7M_INP)) continue
#define wait_low()      while( (__R31 & CLK7M_INP)) continue
#define wait_edge()     do { \
        wait_low();  __R30 &= ~CLK7M_OUT; \
        wait_high(); __R30 |=  CLK7M_OUT; \
    } while(0)

//volatile uint32_t div = 10;
#define PRU_SHARED_MEM_ADDR 0x00010000

#define do_ewait() do { \
    if(_wait) *(uint32_t*)(GPIO2 + GPIO_SETDATAOUT)   = EWAIT; \
    else      *(uint32_t*)(GPIO2 + GPIO_CLEARDATAOUT) = EWAIT; \
    } while(0)

#define do_io() do { \
    /* Set outputs */ \
    __R30 = _e | _vma; \
    /* Get inputs */ \
    _vpa = __R31 & VPA; \
    /* This block takes care of the VMA signal */ \
    /* which is used to acknowledge to old 6800 */ \
    /* style hardware that a bus transfer has  */ \
    /* happened. Resets when the CPU AS is disasserted */ \
    if (_vpa) __R30 |= _vma = VMA; \
    /* This block delays our CPU */ \
    if (_vma == 0 && _e) _wait = 0; \
    else _wait = 1; \
    } while(0)

void main(void) {
    uint32_t _div = 0;
    uint32_t _q = 4;
    uint32_t _e = 0;
    uint32_t _vma = VMA;
    uint32_t _wait = 1;
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
        do_ewait();
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
        do_ewait();
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
        do_ewait();
        wait_edge();

        /* Q9 **************************************************/
        _div = smem[0];
        do_io();
        wait_edge();
    } // end while
}
