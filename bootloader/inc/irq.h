#ifndef __IRQ_H__
#define __IRQ_H__
#include <stdint.h>

/* Buffee's 68K interrupt inputs:
 *   IPL2..0  GPIO2_6 / 7 / 8, active low (the level is the bitwise complement)      both edges   INTC 32 (GPIOINT2A)
 *   RESET    GPIO3_16, open drain, shared with the board: our own pulses are masked  falling edge INTC 62 (GPIOINT3A)
 *   HALT     GPIO0_13, open drain                                                    falling edge INTC 96 (GPIOINT0A)
 * The handlers only count and latch; the work belongs to the hooks, which run in IRQ mode (keep them short, no bus accesses) or to whoever polls the counters. */
extern void InitIRQ(void);
extern unsigned GetIPL(void);                       /* the level now, 0-7 (two identical reads in a row) */
extern void IRQPoll(void);                          /* call from idle loops (the UART wait): runs the work an interrupt only posted, at a point with no bus access of ours in flight */
extern void (*irq_reset_cb)(void);                  /* run by IRQPoll after an external RESET (once the line is released): the ASn clock phase is calibrated again */
extern void IRQResetMask(int on);                   /* 1: we are driving RESET / HALT, ignore their edges; 0: listen again (clears the stale edge) */

extern volatile uint32_t irq_ipl_events;            /* IPL changes seen */
extern volatile uint32_t irq_ipl_level;             /* the level at the last change */
extern volatile uint32_t irq_reset_events;          /* external RESET assertions seen */
extern volatile uint32_t irq_halt_events;           /* external HALT assertions seen */
extern void (*irq_ipl_hook)(unsigned level);        /* called in IRQ mode after every IPL change */
extern void (*irq_reset_hook)(void);                /* called in IRQ mode after every external RESET */
extern void (*irq_halt_hook)(void);

#endif
