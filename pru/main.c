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

#define B_WAIT      (1)             /* Bus Wait */
#define D_WAIT      (2)             /* DTACK Wait */
#define E_WAIT      (4)             /* E Clock Wait */

#define wait_high()     while(!(INP & CLK7M)) continue
#define wait_low()      while( (INP & CLK7M)) continue
// #define wait_high()     do { wait_low(); wait_high(); } while(0)
// #define wait_fall()     do { wait_high(); wait_low(); } while(0)
// #define wait_edge()     do { wait_high(); wait_low();  } while(0);

#define PRU_SHARED_MEM_ADDR 0x00010000

/*  The original E-Clock is non-symmetric.
    It is low for six clock cycles and high for four
    We will allow shortening this from 10 downto 6
    for a possible overclock of the CIA bus */
static uint32_t _div = 0;
static uint32_t _cnt = 0;
static uint32_t _state = 0;

static uint32_t _q = 4;
static uint32_t _e = 0;
static uint32_t _vma = VMA;
static uint32_t _wait = 1;
static uint32_t _vpa;
static uint32_t _cyc;

volatile uint32_t* smem = (volatile uint32_t*)PRU_SHARED_MEM_ADDR;

static void update_io(void) {
    /* Set outputs */
    OUTP = _e | _vma | (_wait ? WAIT0 : 0); 

    /* Get inputs */
    _vpa = INP & VPA;

    /* This block takes care of the VMA signal */
    /* which is used to acknowledge to old 6800 */
    /* style hardware that a bus transfer has  */
    /* happened. Resets when the CPU AS is disasserted */
    if (_vpa) _vma = VMA;

    /* This block delays our CPU */
    if (_vma == 0 && _e) _wait |= E_WAIT;
    else _wait &= ~E_WAIT;
}

// 68K CPU States
static inline void callS0(void) { /* S0 -- SET FC0-FC2 and R/W */ }
static inline void callS1(void) { /* S1 -- SET ADDRESS PINS */ }
static inline void callS2(void) { /* S2 -- SET AS, UDS and LDS*/ }
static inline void callS3(void) { /* S3 -- CPU NOP */ }
static inline void callS4(void) { /* S4 -- CPU NOP */ }
static inline int  callS5(void) { /* S5 -- READ DTACK and BERR
                                   IF WAITING REPEAT S4 */
    // if (INP & DTACK) {
    //     /* We're waiting and should assert WAIT0 */
    //     _wait |= D_WAIT;
    //     return 1;
    // } else {
    //     _wait &= ~D_WAIT; // here or S6?
    //     return 0;
    // }
    return 0;
}
static inline void callS6(void) { /* S6 -- EXPECT DATA */ }
static inline int  callS7(void) { /* S7 -- LATCH DATA, RESET AS, UDS, and LDS */
    // return (!INP & DTACK);
    return 0;
}

// 6800 E States
static inline void callQ0(void) { if(!_vpa) _vma = 0; }
static inline void callQ1(void) { }
static inline void callQ2(void) { smem[2]++; }
static inline void callQ3(void) { }
static inline void callQ4(void) { _e = CLK_E; }
static inline void callQ5(void) { }
static inline void callQ6(void) { smem[2]++; }
static inline void callQ7(void) { }
static inline void callQ8(void) { _e = 0; }
static inline void callQ9(void) { }

void main(void) {        
    CT_CFG.GPCFG0 = 0x0000;
    

    // Clear inital state
    smem[0] = 10;
    smem[1] = 0;
    smem[2] = 0;

    // Because of the timing involved, we'll need to
    // unroll the whole thing... this is the in-phase loop
    // This is because state 5 can loop back to stage 4
    // And we have to stall on state 6
    while (1) {
s0q0:  callQ0();    wait_high(); callS0();
s1q0:  update_io(); wait_low();  callS1();
s2q1:  callQ1();    wait_high(); callS2();
s3q1:  update_io(); wait_low();  callS3();
s4q2:  callQ2();    wait_high(); callS4();
s5q2:  update_io(); wait_low();  if(callS5()) goto s4q3;
s6q3:  callQ3();    wait_high(); callS6();
s7q3:  update_io(); wait_low();  if(callS7()) goto s6q4;
s0q4:  callQ4();    wait_high(); callS0();
s1q4:  update_io(); wait_low();  callS1();
s2q5:  callQ5();    wait_high(); callS2();
s3q5:  update_io(); wait_low();  callS3();
s4q6:  callQ6();    wait_high(); callS4();
s5q6:  update_io(); wait_low();  if(callS5()) goto s4q7;
s6q7:  callQ7();    wait_high(); callS6();
s7q7:  update_io(); wait_low();  if(callS7()) goto s6q8;
s0q8:  callQ8();    wait_high(); callS0();
s1q8:  update_io(); wait_low();  callS1();
s2q9:  callQ9();    wait_high(); callS2();
s3q9:  update_io(); wait_low();  callS3();

s4q0:  callQ0();    wait_high(); callS4();
s5q0:  update_io(); wait_low();  if(callS5()) goto s4q1;
s6q1:  callQ1();    wait_high(); callS6();
s7q1:  update_io(); wait_low();  if(callS7()) goto s6q2;
s0q2:  callQ2();    wait_high(); callS0();
s1q2:  update_io(); wait_low();  callS1();
s2q3:  callQ3();    wait_high(); callS2();
s3q3:  update_io(); wait_low();  callS3();
s4q4:  callQ4();    wait_high(); callS4();
s5q4:  update_io(); wait_low();  if(callS5()) goto s4q5;
s6q5:  callQ5();    wait_high(); callS6();
s7q5:  update_io(); wait_low();  if(callS7()) goto s6q6;
s0q6:  callQ6();    wait_high(); callS0();
s1q6:  update_io(); wait_low();  callS1();
s2q7:  callQ7();    wait_high(); callS2();
s3q7:  update_io(); wait_low();  callS3();
s4q8:  callQ8();    wait_high(); callS4();
s5q8:  update_io(); wait_low();  if(callS5()) goto s4q9;
s6q9:  callQ9();    wait_high(); callS6();
s7q9:  update_io(); wait_low();  if(callS7()) goto s6q0;
    }

    // And this is the off-phase loop
    while (1) {
s0q1:  callQ1();    wait_high(); callS0();
s1q1:  update_io(); wait_low();  callS1();
s2q2:  callQ2();    wait_high(); callS2();
s3q2:  update_io(); wait_low();  callS3();
s4q3:  callQ3();    wait_high(); callS4();
s5q3:  update_io(); wait_low();  if(callS5()) goto s4q4;
s6q4:  callQ4();    wait_high(); callS6();
s7q4:  update_io(); wait_low();  if(callS7()) goto s6q5;
s0q5:  callQ5();    wait_high(); callS0();
s1q5:  update_io(); wait_low();  callS1();
s2q6:  callQ6();    wait_high(); callS2();
s3q6:  update_io(); wait_low();  callS3();
s4q7:  callQ7();    wait_high(); callS4();
s5q7:  update_io(); wait_low();  if(callS5()) goto s4q8;
s6q8:  callQ8();    wait_high(); callS6();
s7q8:  update_io(); wait_low();  if(callS7()) goto s6q9;
s0q9:  callQ9();    wait_high(); callS0();
s1q9:  update_io(); wait_low();  callS1();
s2q0:  callQ0();    wait_high(); callS2();
s3q0:  update_io(); wait_low();  callS3();

s4q1:  callQ1();    wait_high(); callS4();
s5q1:  update_io(); wait_low();  if(callS5()) goto s4q2;
s6q2:  callQ2();    wait_high(); callS6();
s7q2:  update_io(); wait_low();  if(callS7()) goto s6q3;
s0q3:  callQ3();    wait_high(); callS0();
s1q3:  update_io(); wait_low();  callS1();
s2q4:  callQ4();    wait_high(); callS2();
s3q4:  update_io(); wait_low();  callS3();
s4q5:  callQ5();    wait_high(); callS4();
s5q5:  update_io(); wait_low();  if(callS5()) goto s4q6;
s6q6:  callQ6();    wait_high(); callS6();
s7q6:  update_io(); wait_low();  if(callS7()) goto s6q7;
s0q7:  callQ7();    wait_high(); callS0();
s1q7:  update_io(); wait_low();  callS1();
s2q8:  callQ8();    wait_high(); callS2();
s3q8:  update_io(); wait_low();  callS3();
s4q9:  callQ9();    wait_high(); callS4();
s5q9:  update_io(); wait_low();  if(callS5()) goto s4q0;
s6q0:  callQ0();    wait_high(); callS6();
s7q0:  update_io(); wait_low();  if(callS7()) goto s6q1;
    }

}
