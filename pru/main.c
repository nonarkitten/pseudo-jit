#include <stdint.h>

#include "inc/pru_cfg.h"
#include "inc/pru_registers.h"

/*
                                    PIN------------------------------------     SCHEMATIC-REFERENCES----------------------
    SIGNAL NAME             FN      AM335x  OSD335x     IO   NAME               LABEL         M68K BUS IO         GREENPAK  NOTES
    pr1_pru0_pru_r30_0      PRU0    A13     A1          O    MCASP0_ACLKX       PRU_VMA       U6.31 -> M68K_VMA   -         DNFW
    pr1_pru0_pru_r31_1      PRU0    B13     A2          I*   MCASP0_FSX         BBB_BGACK     U6.44 <- M68K_BGACK -         DNFW
    pr1_pru0_pru_r31_2      PRU0    D12     B2          I*   MCASP0_AXR0        BBB_RESET     U6.35 <- M68K_RST   -         DNFW
    pr1_pru0_pru_r31_3      PRU0    C12     B1          I*   MCASP0_AHCLKR      BBB_BR        U6.42 <- M68K_BR    -         DNFW
    pr1_pru0_pru_r31_4      PRU0    B12     A3          I    MCASP0_ACLKR       BBB_CLK7      U6.39 <- M68K_CLK7  -         DNFW
    pr1_pru0_pru_r30_5      PRU0    C13     B3          O    MCASP0_FSR         BBB_BG        U6.46 -> M68K_BG    -         DNFW
    pr1_pru0_pru_r30_6      PRU0    D13     C3          O    MCASP0_AXR1        PRU_EWAIT     -                   U3.2      Repurposable
    pr1_pru0_pru_r30_7      PRU0    A14     C4          O    MCASP0_AHCLKX      PRU_ECLK      U6.30 -> M68K_E     -         DNFW
    pr1_pru0_pru_r30_12     PRU0    G17     B15              MMC0_CLK           GPIO_SUPE     U5.9  -> M68K_FC2   U3.13     Also A24
    pr1_pru0_pru_r30_13     PRU0    G18     B16              MMC0_CMD           GPIO_IACK     -                   U3.15     Also A25
    pr1_pru0_pru_r31_16     PRU0    D14     B4          I*   XDMA_EVENT_INTR1   PRU_VPA       U6.28 <- M68K_VPA   -         DNFW

    (GreenPAK)              -       -       -           O                       M68K_UDS      Header              U3.3      No direct MCU/PRU access             
    (GreenPAK)              -       -       -           O                       M68K_LDS      Header              U3.4      No direct MCU/PRU access         
    (GreenPAK)              -       -       -           O                       M68K_FC0      Header              U3.5      No direct MCU/PRU access        
    (GreenPAK)              -       -       -           O                       M68K_RW       Header              U3.6      No direct MCU/PRU access              
    (GreenPAK)              -       -       -           O                       M68K_DTACK    Header              U3.7      No direct MCU/PRU access                 
    (GreenPAK)              -       -       -           O                       M68K_FC1      Header              U3.10     No direct MCU/PRU access

    gpmc_csn2               GPMC    V9      P1          I    GPMC_CSN2          GPMC_ASN      U4.13 -> M68K_AS    U3.20     DNFW     
    gpmc_oen_ren            GPMC    T7      N1          I    GPMC_OEN           GPMC_OEN      -                   U3.19     Repurposable ???         
    gpmc_wen                GPMC    U6      N2          I    GPMC_WEN           GPMC_WEN      -                   U3.18     Repurposable ???        
    gpmc_be1n               GPMC    U18     N14         I    GPMC_BEN1          GPMC_BEN1     -                   U3.17     DNFW          
    gpmc_be0n_cle           GPMC    T6      N3          I    GPMC_BEN0_CLE      GPMC_BEN0     -                   U3.16     DNFW          
    gpmc_wait0              GPMC    T17     P15         I    GPMC_WAIT0         GPMC_WAIT0    -                   U3.12     DNFW           

    OEn is asserted on reads
    WEn is asserted on writes
    ASn is always asserted

    So I think we can omit using OEn. This is not a PRU signal though, it's only accessible through GPIO and thus very slow.

    Recommended rework:

    Make GPMC_OEN (U3.19) GPIO_IACK since this can be a slow signal and needs to be driven by the CPU anyway.
    Reuse PRU_EWAIT (U3.2) and GPIO_IACK (U3.15) for GreenPAK <-> PRU signalling

    For our work here, this is simple renaming IACK and and EWAIT to DTACK and WAIT0 respectively and
    ensuring we're using the correct bit offsets.
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
