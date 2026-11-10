#include <stdint.h>
#include <pru/io.h>        /* __R30 / __R31 as regio symbols (every access is a real read / write) */
/* PRU firmware for the GreenPAK design 4 (greenpak/buffee4.hex, 'buffee 4.gp6'): pru-gcc build, the hot loop in assembler.
 *
 * What the GreenPAK does: it holds WAIT0 low from ASn falling until a two-flop chain (falling then rising edge of the echoed clock) has captured 'a DTACK-equivalent
 * (DTACKn low or PRU_TERMn low) AND armed (one clock after ASn)'; R/W = WEn; ASn is a GreenPAK flop clocked by the echoed clock (everything on the bus follows the echo).
 * What the PRU does:
 *   - echoes CLK7 INVERTED (low while CLK7 is high): the echo comes ~20 ns after the edge, and the inversion puts every GreenPAK edge half a period later, where a 68000 asserts AS (S2).
 *     Echoes CLK7 to the GreenPAK (r30 bit 6, 'PRU_CLK7E') with as little and as constant a delay as it can: the echo is the first instruction after the edge is seen, and the wait for
 *     an edge is a one-instruction loop (QBBC / QBBS on r31 bit 4), so the delay from the CLK7 pin to the echo pad is the PRU's input synchroniser + that loop's granularity + one instruction;
 *   - generates E (r30 bit 7, a ten clock divide, high for four);
 *   - 6800 cycles (VPA low: the CIA, autovector): VMA low (r30 bit 0) when the E counter is 2 at a rising edge, then PRU_TERMn low (r30 bit 13) at E counter TERM_AT; both are released as soon as VPAn is high.
 * Why the loop is split in two copies: while no 6800 cycle is running (VMA and TERMn high, nearly always) the waits are the bare one-instruction loops, so the echo is as fast as it
 * gets; while one is running (VMA / TERMn low) the waits also poll VPAn for the release, a two-instruction loop (the echo of those few cycles is up to one instruction later).
 * Every SET / CLR of an r30 bit is one instruction, no read-modify-write sequence: the budget is 14 PRU cycles (200 MHz) per CLK7 half period.
 * Inputs: CLK7 (r31 bit 4), VPAn (r31 bit 16). Outputs as above. */

#define PRU_SHARED_MEM_ADDR 0x00010000

#define CLK7M		(1 << 4)
#define VMAn        (1 << 0)
#define TERMn       (1 << 13)       /* PRU_TERMn */
#define MEAS_WINDOWS 8              /* 8 x 5 ms */
#define MEAS_CYCLES  1000000        /* 5 ms of PRU cycles */
#define TERM_AT     9               /* E counter at which TERMn is asserted: a constant (a load from the shared RAM in the loop would make a pass as long as a CLK7 half period); 5-9 read the CIAs reliably, only 8 and 9 also land the writes */

void main(void) {
    volatile uint32_t* smem = (volatile uint32_t*)PRU_SHARED_MEM_ADDR;
    register uint32_t in;

    *(volatile uint32_t*)0x00026008 = 0x0000;        /* PRU_CFG GPCFG0: direct connect */
    smem[0] = 0; smem[1] = 0; smem[2] = 0; smem[3] = TERM_AT;

    /* Measure CLK7 here, against the PRU's own 200 MHz cycle counter (PRU_CTRL CYCLE): count the rising edges from a first edge; at the end of each 5 ms window i (1,000,000 cycles)
     * publish the cumulative edges and cycles since that first edge (smem[8 + 2i], smem[9 + 2i]) and the number of windows done (smem[6], written last). The ARM computes
     * kHz = edges * 200000 / cycles (one edge in 35,000 per window: 29 ppm), any number of windows 1..MEAS_WINDOWS, no timestamps taken at other moments (the ARM-side timer read
     * used before was off by 0.9%). The work per edge is a counter increment, one load of the cycle register and a compare; the publish costs a few more cycles once per window.
     * An ARM request to start (smem[0] != 0, e.g. a PRU restart from the menu) ends the measurement at the next window. */
    {
        volatile uint32_t* ctl = (volatile uint32_t*)0x00022000;       /* PRU0 CTRL: CONTROL (+0), CYCLE (+0xC) */
        uint32_t edges = 0, c0, w = 0, target = MEAS_CYCLES;
        for (int i = 4; i < 8 + 2 * MEAS_WINDOWS; i++) smem[i] = 0;
        ctl[0] |= (1u << 3);                                           /* COUNTER_ENABLE */
        ctl[3] = 0;
        do { in = __R31; } while (!(in & CLK7M));
        c0 = ctl[3];
        do { in = __R31; } while (in & CLK7M);
        while (w < MEAS_WINDOWS) {
            do { in = __R31; } while (!(in & CLK7M));
            edges++;
            uint32_t now = ctl[3] - c0;
            if (now >= target) {
                smem[8 + 2 * w] = edges; smem[9 + 2 * w] = now; w++; smem[6] = w; target += MEAS_CYCLES;
                if (smem[0] != 0) break;
            }
            do { in = __R31; } while (in & CLK7M);
        }
        while (smem[0] == 0) ;
    }
    if (smem[0] > 10) smem[0] = 10; else if (smem[0] < 7) smem[0] = 7;
    smem[1] = smem[0];

    __R30 = VMAn | TERMn;                            /* VMAn and TERMn inactive (high), echo and E low */

    register uint32_t e = 0;                         /* the E counter: counts falling edges, 0..9 */
    __asm__ volatile (
        /* ---- idle: VMA and TERMn are high ---- */
        "1:  qbbc  1b, r31, 4          \n"           /* wait for CLK7 high */
        "    clr   r30, r30, 6         \n"           /* the echo (inverted: low while CLK7 is high) */
        "    qbbs  2f, r31, 16         \n"           /* VPAn high: nothing to do (the usual case) */
        "    qbne  2f, %0, 2           \n"           /* a 6800 cycle: VMA goes low at E counter 2 ... */
        "    clr   r30, r30, 0         \n"
        "    jmp   21f                 \n"           /* ... and the active copy takes over (we are just after the rising edge) */
        "2:  qbbs  2b, r31, 4          \n"           /* wait for CLK7 low */
        "    set   r30, r30, 6         \n"           /* the echo (inverted: high while CLK7 is low) */
        "3:  qbne  4f, %0, 9           \n"           /* E: high at 6, low at 0 (ten falling edges per period) */
        "    clr   r30, r30, 7         \n"
        "    ldi   %0, 0               \n"
        "    jmp   1b                  \n"
        "4:  qbne  5f, %0, 5           \n"
        "    set   r30, r30, 7         \n"
        "5:  add   %0, %0, 1           \n"
        "    jmp   1b                  \n"
        /* ---- active: VMA (and later TERMn) low until VPAn is high ---- */
        "21: qbbs  30f, r31, 16        \n"           /* wait for CLK7 low, watching VPAn */
        "    qbbs  21b, r31, 4         \n"
        "    set   r30, r30, 6         \n"           /* the echo (inverted) */
        "    qbne  22f, %0, 9          \n"           /* E, as above */
        "    clr   r30, r30, 7         \n"
        "    ldi   %0, 0               \n"
        "    jmp   23f                 \n"
        "22: qbne  24f, %0, 5          \n"
        "    set   r30, r30, 7         \n"
        "24: add   %0, %0, 1           \n"
        "23: qbbs  31f, r31, 16        \n"           /* wait for CLK7 high, watching VPAn */
        "    qbbc  23b, r31, 4         \n"
        "    clr   r30, r30, 6         \n"           /* the echo (inverted) */
        "    qbbs  30f, r31, 16        \n"           /* VPAn high: release (we are after a rising edge: wait for the falling edge) */
        "    qbne  21b, %0, %1         \n"           /* TERMn low at E counter TERM_AT */
        "    clr   r30, r30, 13        \n"
        "    jmp   21b                 \n"
        "30: set   r30, r30, 0         \n"           /* release VMA and TERMn; back to idle at the falling-edge wait */
        "    set   r30, r30, 13        \n"
        "    jmp   2b                  \n"
        "31: set   r30, r30, 0         \n"           /* release while waiting for the rising edge: back to idle at the rising-edge wait */
        "    set   r30, r30, 13        \n"
        "    jmp   1b                  \n"
        : "+r"(e) : "i"(TERM_AT));
    __builtin_unreachable();
}
