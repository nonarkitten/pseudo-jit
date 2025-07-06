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
#define DTACK       (1 << 12)       /* Monitor 68K DTACK */

#define wait_high()     while(!(INP & CLK7M)) continue
#define wait_low()      while( (INP & CLK7M)) continue
#define wait_rise()     do { wait_low(); wait_high(); } while(0)
#define wait_fall()     do { wait_high(); wait_low(); } while(0)

/*  The original E-Clock is non Symmetric.
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

// volatile uint32_t* smem = (volatile uint32_t*)PRU_SHARED_MEM_ADDR;

void main(void) {
    // smem[0] = 10;
    // smem[1] = 0;
    // smem[2] = 0;

    // Because of the timing involved, we'll need to
    // unroll the whole thing... this is the in-phase loop
    // This is because state 5 can loop back to stage 4
    // And we have to stall on state 6
    while (1) {
        // E high for 4 CLK cycles
        OUTP |= CLK_E;

        wait_high();
        OUTP |= WAIT0;
        wait_low();
        OUTP &= ~WAIT0;

        wait_high();
        OUTP |= WAIT0;
        wait_low();
        OUTP &= ~WAIT0;

        wait_high();
        OUTP |= WAIT0;
        wait_low();
        OUTP &= ~WAIT0;

        wait_high();
        OUTP |= WAIT0;
        wait_low();
        OUTP &= ~WAIT0;

        // E low for 6 clock cycles
        OUTP &= ~CLK_E;

        wait_high();
        OUTP |= WAIT0;
        wait_low();
        OUTP &= ~WAIT0;

        wait_high();
        OUTP |= WAIT0;
        wait_low();
        OUTP &= ~WAIT0;

        wait_high();
        OUTP |= WAIT0;
        wait_low();
        OUTP &= ~WAIT0;

        wait_high();
        OUTP |= WAIT0;
        wait_low();
        OUTP &= ~WAIT0;

        wait_high();
        OUTP |= WAIT0;
        wait_low();
        OUTP &= ~WAIT0;

        wait_high();
        OUTP |= WAIT0;
        wait_low();
        OUTP &= ~WAIT0;

    } // end while
}
