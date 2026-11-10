/*
 * irq.c - the AM335x INTC and the GPIO interrupts of Buffee's 68K inputs (IPL2..0, RESET, HALT).
 */
#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "pinmux.h"
#include "init.h"
#include "irq.h"
#include "mem_be.h"

#define R32(a) (*(volatile uint32_t*)(a))

#define INTC             0x48200000u
#define INTC_SYSCONFIG   (INTC + 0x10)
#define INTC_SYSSTATUS   (INTC + 0x14)
#define INTC_SIR_IRQ     (INTC + 0x40)
#define INTC_CONTROL     (INTC + 0x48)
#define INTC_THRESHOLD   (INTC + 0x68)
#define INTC_MIR_CLEAR(n) (INTC + 0x88 + 0x20 * (n))
#define INTC_MIR_SET(n)   (INTC + 0x8C + 0x20 * (n))
#define INTC_ILR(n)      (INTC + 0x100 + 4 * (n))

#define GPIO0 0x44E07000u
#define GPIO2 0x481AC000u
#define GPIO3 0x481AE000u
#define G_IRQSTATUS0     0x2C
#define G_IRQSTATUS_SET0 0x34
#define G_IRQSTATUS_CLR0 0x3C
#define G_DATAIN         0x138
#define G_RISINGDETECT   0x148
#define G_FALLINGDETECT  0x14C

#define IRQ_GPIO2A 32
#define IRQ_GPIO3A 62
#define IRQ_GPIO0A 96
#define IRQ_UART0  72
extern void UART0IRQ(void);
extern void InitUART0IRQ(void);

#define IPL_MASK    (7u << 6)           /* GPIO2_6 = IPL2, 7 = IPL1, 8 = IPL0 */
#define RESET_BIT   (1u << 16)          /* GPIO3_16 */
#define HALT_BIT    (1u << 13)          /* GPIO0_13 */

volatile uint32_t irq_ipl_events, irq_ipl_level, irq_reset_events, irq_halt_events;
void (*irq_ipl_hook)(unsigned level);
void (*irq_reset_hook)(void);
void (*irq_halt_hook)(void);
static volatile int reset_masked, reset_pending;
void (*irq_reset_cb)(void);

unsigned GetIPL(void) {
    uint32_t a, b;
    b = R32(GPIO2 + G_DATAIN);
    do { a = b; b = R32(GPIO2 + G_DATAIN); } while (((a ^ b) & IPL_MASK) != 0);       /* the three lines settle within a few ns of each other */
    b = ~b;                                                                            /* active low; GPIO2_6 = IPL2, 7 = IPL1, 8 = IPL0 */
    return ((b >> 6) & 1) << 2 | ((b >> 7) & 1) << 1 | ((b >> 8) & 1);
}

void irq_dispatch(void) {
    uint32_t n = R32(INTC_SIR_IRQ) & 0x7F;
    if (n == IRQ_GPIO2A) {
        uint32_t st = R32(GPIO2 + G_IRQSTATUS0) & IPL_MASK;
        R32(GPIO2 + G_IRQSTATUS0) = st;
        if (st) {
            unsigned lvl = GetIPL();
            irq_ipl_level = lvl; irq_ipl_events++;
            if (irq_ipl_hook) irq_ipl_hook(lvl);
        }
    } else if (n == IRQ_GPIO3A) {
        uint32_t st = R32(GPIO3 + G_IRQSTATUS0) & RESET_BIT;
        R32(GPIO3 + G_IRQSTATUS0) = st;
        if (st && !reset_masked) {
            irq_reset_events++; reset_pending = 1;
            if (irq_reset_hook) irq_reset_hook();
        }
    } else if (n == IRQ_UART0) {
        UART0IRQ();
    } else if (n == IRQ_GPIO0A) {
        uint32_t st = R32(GPIO0 + G_IRQSTATUS0) & HALT_BIT;
        R32(GPIO0 + G_IRQSTATUS0) = st;
        if (st && !reset_masked) {
            irq_halt_events++;
            if (irq_halt_hook) irq_halt_hook();
        }
    }
    R32(INTC_CONTROL) = 1;                                  /* new IRQ agreement: the INTC may present the next one */
    asm volatile("dsb");
}

void IRQPoll(void) {
    if (!reset_pending) return;
    reset_pending = 0;
    double t0 = ReadDMTimerSeconds();
    while (!(R32(GPIO3 + G_DATAIN) & RESET_BIT) && ReadDMTimerSeconds() - t0 < 2.0) ;       /* the Amiga holds RESET for a while */
    WaitMSDMTimer(50);                                      /* and Agnus needs a moment after it */
    R32(GPIO3 + G_IRQSTATUS0) = RESET_BIT; R32(GPIO0 + G_IRQSTATUS0) = HALT_BIT;
    printf("\nAmiga reset (#%lu)\n", (unsigned long)irq_reset_events);
    if (irq_reset_cb) irq_reset_cb();
}

void IRQResetMask(int on) {
    if (on) { reset_masked = 1; return; }
    WaitMSDMTimer(2);                                       /* the line rises slowly (open drain, pull-up) */
    R32(GPIO3 + G_IRQSTATUS0) = RESET_BIT;                  /* forget our own edge */
    R32(GPIO0 + G_IRQSTATUS0) = HALT_BIT;
    reset_masked = 0;
}

static void intc_unmask(uint32_t n) { R32(INTC_MIR_CLEAR(n >> 5)) = 1u << (n & 31); }

void InitIRQ(void) {
    static const pin_muxing_t ipl_pins[] = {
        { PINMUX_CONF_LCD_DATA0, (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M7) },   /* IPL2 gpio2_6 */
        { PINMUX_CONF_LCD_DATA1, (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M7) },   /* IPL1 gpio2_7 */
        { PINMUX_CONF_LCD_DATA2, (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M7) },   /* IPL0 gpio2_8 */
        { 0xFFFFFFFF, 0xFFFFFFFF },
    };
    config_mux(ipl_pins);

    R32(INTC_SYSCONFIG) = 2;                                /* soft reset: everything masked */
    while (!(R32(INTC_SYSSTATUS) & 1)) ;
    R32(INTC_THRESHOLD) = 0xFF;

    R32(GPIO2 + G_RISINGDETECT)  |= IPL_MASK;               /* the level is an encoding: every change matters */
    R32(GPIO2 + G_FALLINGDETECT) |= IPL_MASK;
    R32(GPIO2 + G_IRQSTATUS0) = IPL_MASK;
    R32(GPIO2 + G_IRQSTATUS_SET0) = IPL_MASK;

    R32(GPIO3 + G_FALLINGDETECT) |= RESET_BIT;
    R32(GPIO3 + G_RISINGDETECT)  &= ~RESET_BIT;
    R32(GPIO3 + G_IRQSTATUS0) = RESET_BIT;
    R32(GPIO3 + G_IRQSTATUS_SET0) = RESET_BIT;

    R32(GPIO0 + G_FALLINGDETECT) |= HALT_BIT;
    R32(GPIO0 + G_RISINGDETECT)  &= ~HALT_BIT;
    R32(GPIO0 + G_IRQSTATUS0) = HALT_BIT;
    R32(GPIO0 + G_IRQSTATUS_SET0) = HALT_BIT;

    irq_ipl_level = GetIPL();
    intc_unmask(IRQ_UART0); InitUART0IRQ(); intc_unmask(IRQ_GPIO2A); intc_unmask(IRQ_GPIO3A); intc_unmask(IRQ_GPIO0A);
    asm volatile("cpsie i" ::: "memory");
}

