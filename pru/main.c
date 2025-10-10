#include <stdint.h>
#include "pru_cfg.h"

#define PRU_SHARED_MEM_ADDR 0x00010000

// INPUTS
#define CLK7M		(1 << 4)		/* 7M CLK */
#define DTACKn      (1 << 12)       /* Passthru of DTACK */
#define ASn         (1 << 13)       /* Passthru of OEn & WEn */
#define VPAn        (1 << 16)       /* Valid Peripheral Address */

// OUTPUTS
#define VMAn        (1 << 0)        /* Valid Memory Address */
#define EWAITn      (1 << 6)        /* GPMC WAIT0 */
#define CLK_E		(1 << 7)		/* E CLKC */

volatile register uint32_t __R30;	/* Output */
volatile register uint32_t __R31;	/* Input */

volatile uint32_t* smem = (volatile uint32_t*)PRU_SHARED_MEM_ADDR;   

#define wait_low()  do { _in = __R31; } while ( (_in & CLK7M))
#define wait_high() do { _in = __R31; } while (!(_in & CLK7M))

// The phase of this in relation to the CPU clock does not matter
#define do_e_bus() do { \
    if (e_counter == 9) { \
        __R30 &= ~CLK_E; \
        e_counter = 0; \
    } else { \
        if (e_counter == 5) __R30 |= CLK_E; \
        e_counter += 1; \
    }} while(0)

void main(void) {
    register uint8_t e_counter = 0;
    register uint32_t _in;

    /* Configure GPI and GPO as Mode 0 (Direct Connect) */
    CT_CFG.GPCFG0 = 0x0000;

    volatile uint32_t* smem = (volatile uint32_t*)PRU_SHARED_MEM_ADDR;
    smem[0] = 0;
    smem[1] = 0;
    smem[2] = 0;

    // Count edges until main tells us to stop
    while (smem[0] == 0) {
        wait_high();
        wait_low();
        smem[2]++;
    }

    if (smem[0] > 10) smem[0] = 10; else if (smem[0] < 7) smem[0] = 7;
    smem[1] = smem[0];

    __R30 = VMAn;

state0:
    wait_low();
    do_e_bus();
    // should be asserted by GPMC here
    // M68K_RW_n <= 1'b1;


state1:
    wait_high();
    // ***PROBLEM, AS_n gets read here about 35+ns *after* GPMC asserts
    // the + here is whatever our PRU latency & jitter is (??)
    // we should delay AS_n as much as possible and assert OE and WE
    // early and echo the logical AND of these
    // e.g.,
    //   _   _   _   _   _   _
    // _| |_| |_| |_| |_| |_|   CLK
    // ______           ______
    //       \\________//       "Real" ASn (35ns pD)
    // _____           _______
    //      \_________/         OEn/WEn (35ns early)
    // ______           ______
    //       \_________/        "Fake" ASn (35ns pD)
    
    // if (!(_in & ASn)) __R30 &= ~EWAITn;

state2:
    wait_low();
    do_e_bus();
    // Note: the 68020 asserts DS at the same time as AS
    // regardless of R/W and this is never corrected on
    // accelerator hardware; however, if we want a delay
    // in DS that's in the GreenPAK on falling edge of CLK

    // should be asserted by GPMC here
    // M68K_RW based on r/w
    // M68K_UDS_n read;
    // M68K_LDS_n read;

    if (_in & ASn) goto state1;

state3:
    wait_high();

    // We proceed when either VMA or DTACK are asserted
    if (!(_in & DTACKn) || (!(__R30 & VMAn) && (e_counter == 8))) {
        goto state4;
    } else if(!(_in & VPAn) && (e_counter == 2)) {
        __R30 &= ~VMAn;
    }
    // Add wait states
    wait_low();
    do_e_bus();
    goto state3;

state4:
    wait_low();
    do_e_bus();
    // free the GPMC from waiting to read/write
    __R30 |= EWAITn;

state5:
    wait_high();
    // GPMC should have read by now

state6:
    wait_low();
    do_e_bus();
    __R30 |= VMAn;
    __R30 &= ~EWAITn;

state7:
    wait_high();
    // should be asserted by GPMC here
    // M68K_AS_n <= 1'b1;
    // M68K_UDS_n <= 1'b1;
    // M68K_LDS_n <= 1'b1;

    goto state0;
}

// OE/WE 6, 24 aligns with AS 8, 26 so OE/WE needs to be 2 GPMC clocks early (at  100MHz)
// Purple AS
// Yellow DS
// AS pulse is ~200ns

