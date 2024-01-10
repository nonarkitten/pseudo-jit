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

         AM335x                                      68K BUS
    --------------------.
    pr1_pru0_pru_r3x_4  | <--------------------  CLK7    
    pr1_pru0_pru_r3x_7  | -------------------->  ECLK    
    pr1_pru0_pru_r3x_16 | <--------------------  VPA     
    pr1_pru0_pru_r3x_0  | -------------------->  VMA     
                        |     .-----------.    
                  WAIT0 | <-- |12       7 | <--  DTACK  
       (GPIO_IACK) CSN2 | --> |20       3 | -->  UDS    
                    WEN | --> |18 Green 4 | -->  LDS    
                    OEN | --> |19  PAK  5 | -->  FC0     
                   BEN0 | --> |16       10| -->  FC1     
                   BEN1 | --> |17       6 | -->  R/W     
        (GPIO_SUPE) A24 | --> |13         |
                        |     |           | 1, 14 Vcc      
    pr1_pru0_pru_r3x_6  | --- | 2         | 8, 9  I2C          
    pr1_pru0_pru_r3x_12 | --- |15         | 11    Ground
    --------------------'     '-----------'

    With two signals we need to

    PRU_DTACK = M68K_DTACK || OEN || WEN
        - rising PRU_DTACK signifies we're in S7
        - falling PRU_DTACK signifies the bus is ready

    GPMC_WAIT0 = PRU_WAIT0
        - low WAIT0 will stall the GPMC
        - WAIT0 should be low when entering S5 and DTACK is high
        - WAIT0 should be high when entering S5 and DTACK is low
        - WAIT0 should be low if VMA is low and E is high
        - WAIT0 should be high if VMA is high or E is low
        
    68K Signals directly to GreenPAK
        M68K_DTACK
        M68K_UDS
        M68K_LDS
        M68K_FC0
        M68K_FC1
        M68K_RW

    GPMC to GreenPAK
        GPMC_WAIT0  gpmc_wait0
        GPMC_SUPE   mmc0_clk (address pin 24)

    GPIO to GreenPAK
        GPIO_IACK   mmc0_cmd (gpio1_31, was GPMC_ASN)

    68K Signals directly to PRU
        BBB_CLK7    pr1_pru0_pru_r31_4  (MCASP0_ACLKR)
        PRU_ECLK    pr1_pru0_pru_r30_7  (MCASP0_AHCLKX)
        PRU_VPA     pr1_pru0_pru_r31_16 (XDMA_EVENT_INTR1)
        PRU_VMA     pr1_pru0_pru_r30_0  (MCASP0_ACLKX)

    PRU to GreenPAK
        PRU_WAIT0   pr1_pru0_pru_r30_6  (MCASP0_AXR1, was PRU_EWAIT)
        PRU_DTACK   pr1_pru0_pru_r30_12 (MMC0_CLK, was GPIO_IACK)
*/

/* Output */
volatile register uint32_t __R30;
#define OUTP        (__R30)
#define CLK_E    	(1 << 7)		/* 6800 "Enable" clock */
#define VMA         (1 << 0)        /* Valid Peripheral Address */
#define WAIT0       (1 << 6)        /* Suspend GPMC if not ready */

/* Input */
volatile register uint32_t __R31;
#define INP         (__R31)
#define CLK7M       (1 << 4)		/* 68K bus clock */
#define VPA         (1 << 16)       /* Valid Peripheral Address */
#define DTACK       (1 << 12)       /* Monitor 68K DTACK */

#define wait_high()     while(!(INP & CLK7M)) continue
#define wait_low()      while( (INP & CLK7M)) continue
#define wait_rise()     do { wait_low(); wait_high(); } while(0)
#define wait_fall()     do { wait_high(); wait_low(); } while(0)

/*  The original E-Clock is non-symmetric.
    It is low for six clock cycles and high for four
    We will allow shortening this from 10 downto 6
    for a possible overclock of the CIA bus */
uint32_t _div = 0;
uint32_t _q = 4;
uint32_t _e = 0;
uint32_t _vma = VMA;
uint32_t _wait = 1;
uint32_t _vpa;
uint32_t _cyc;

volatile uint32_t* smem = (volatile uint32_t*)PRU_SHARED_MEM_ADDR;

#if 0
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
#endif

static void update_estate(void) {
    /* Get inputs */
    _vpa = INP & VPA;

    /* This block takes care of the VMA signal */
    /* which is used to acknowledge to old 6800 */
    /* style hardware that a bus transfer has  */
    /* happened. Resets when the CPU AS is disasserted */
    if (_vpa) _vma = VMA;

    /* This block delays our CPU */
    if (_vma == 0 && _e) _wait = 0;
    else _wait = 1;
}

// The even cycles is where most of the 6800 bus clock behaviour happens       
static void do_ewait_even(void) {
    if(_q == 0) {
        /* Q0 **************************************************/
        if(!_vpa) _vma = 0;
        _q = 1;

    } else if(_q == 2) {
        /* Q2 **************************************************/
        _q = 3;

    } else if(_q == 4) {
        /* Q4 **************************************************/
        _e = CLK_E;
        _q = 5;

    } else if(_q == 6) {
        /* Q6 **************************************************/
        _q = 7;

    } else {
        /* Q8 **************************************************/
        _e = 0;
        _q = 9;
    }
}

// On odd cycles we'll do some miscellaneous activities, such as
// read and write with the main CPU, and check if we're done one loop
static void do_ewait_odds(void) {
    if(_q == 1) {
        /* Q1 **************************************************/
        _div = smem[0]; // _div should only be even, 6, 8 or 10
        _q = 2;

    } else if(_q == 3) {
        /* Q3 **************************************************/
        smem[1] = _div;
        _q = 4;

    } else if(_q == 5) {
        /* Q5 **************************************************/
        smem[2]++;
        _q = (_div == 6) ? 0 : 6;

    } else if(_q == 7) {
        /* Q7 **************************************************/
        _q = (_div == 8) ? 0 : 8;

    } else {
        /* Q9 **************************************************/
        _q = 0;

    }
}

void main(void) {
    smem[0] = 10;
    smem[1] = 0;
    smem[2] = 0;

    while (1) {
        do_ewait_even();
        OUTP = _e | _vma; /* Set outputs */
        update_estate();
        wait_rise();
        /* S0 -- SET FC0-FC2 and R/W */
        wait_fall();
        /* S1 -- SET ADDRESS PINS */
        
        do_ewait_odds();
        OUTP = _e | _vma; /* Set outputs */
        update_estate();
        wait_rise();
        /* S2 -- SET AS, UDS and LDS*/
        wait_fall();
        /* S3 -- CPU NOP */

        do_ewait_even();
        OUTP = _e | _vma; /* Set outputs */
        update_estate();
wait_state:
        wait_rise();
        /* S4 -- CPU NOP */
        wait_fall();
        /* S5 -- READ DTACK and BERR
                 IF WAITING REPEAT S4 */
        if(INP & DTACK) {
            /* We're waiting and should assert WAIT0 */
            OUTP |= WAIT0;
            goto wait_state;
        }

        do_ewait_odds();
        OUTP = _e | _vma; /* Set outputs */
        update_estate();
        wait_rise();
        /* S6 -- EXPECT DATA */
        wait_fall();
        /* S7 -- LATCH DATA, RESET AS, UDS, and LDS */

    } // end while
}