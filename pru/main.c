#include <stdint.h>

#include "inc/pru_cfg.h"
#include "inc/pru_registers.h"

/*       AM335x                                      68K BUS
   ---------------------------------.
    pr1_pru0_pru_r30_0              | -------------------------->  VMA     
    pr1_pru0_pru_r31_1              | -------------------------->  BGACK   
    pr1_pru0_pru_r31_2              | <--------------------------  RESET   
    pr1_pru0_pru_r31_3              | <--------------------------  BR
    pr1_pru0_pru_r31_4              | <--------------------------  CLK7    
    pr1_pru0_pru_r30_5              | -------------------------->  BG
    pr1_pru0_pru_r30_7              | -------------------------->  ECLK    
    pr1_pru0_pru_r31_16             | <--------------------------  VPA     
                                    |   .----------------------->  AS
                                    |   |    .-----------.    
                         GPMC_CSN2  | --o--> |20       3 | ----->  UDS    
                         GPMC_WEN   | -----> |18 Green 4 | ----->  LDS    
                         GPMC_OEN*  | -----> |19  PAK  5 | ----->  FC0     
                         GPMC_BEN1  | -----> |17       6 | ----->  R/W
                        GPMC_WAIT0  | <----- |12       7 | <-----  DTACK  
                         GPMC_BEN0  | -----> |16       10| ----->  FC1     
                                    |        |           |
    pr1_pru0_pru_r3x_6   PRU_EWAIT  | ------ | 2         | 1, 14 Vcc
    pr1_pru0_pru_r3x_13  GPIO_IACK  | ------ |15         | 8, 9  I2C   
    pr1_pru0_pru_r3x_12  GPIO_SUPE  | --o--- |13         | 11    Ground
   ---------------------------------'   |    '-----------'
                                        '----------------------->  FC2

  NOTES: - Read on PRU is 2 cycles + 1 cycle latency for GPI (stalls, so 3 cycle total)
         - Write is 1 cycle + 1 cycle latency for GPI (does not stall, so 1 cycle total)
         - By default each cycle is 5ns, but the PRU can operate up to 333MHz (3ns)
         - We don't need GPMC_OEN, so we can reuse it as a GPIO (sub GPIO_IACK)
         - This in turn frees up GPIO_IACK for other things
                                                                                                                                                                                                                                                           
*/

/* Output */
volatile register uint32_t __R30;
#define OUTP        (__R30)
#define CLK_E		(1 << 7)		/* 6800 "Enable" clock */
#define VMA         (1 << 0)        /* Valid Peripheral Address */
#define WAIT0       (1 << 6)        /* Suspend GPMC if not ready */

/* Input */
volatile register uint32_t __R31;
#define INP         (__R31)
#define CLK7M       (1 << 4)		/* 68K bus clock */
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

    // Because of the timing involved, we'll need to
    // unroll the whole thing... this is the in-phase loop
    // This is because state 5 can loop back to stage 4
    // And we have to stall on state 6
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
