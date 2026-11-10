/*
 * uart.c -- UART0, the console: polled output, a receive ring filled by the interrupt, and the break on three ESC presses.
 *
 * Copyright (c) 2021-2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdio.h>
#include <stdint.h>
#include "irq.h"
#include <string.h>

#include "pinmux.h"
#include "am335x_regs.h"

void InitUART0(int baud) {
    static const pin_muxing_t uart_pins[] = {
        { PINMUX_CONF_UART0_RXD, (PIN_CFG_INEN | PIN_CFG_PDIS | PIN_CFG_M0) },
        { PINMUX_CONF_UART0_TXD, (PIN_CFG_PDIS | PIN_CFG_M0) },
        { 0xFFFFFFFF, 0xFFFFFFFF },
    };
    /*Enable Func Clock*/
    CM_WKUP_UART0_CLKCTRL->BIT.MODULEMODE = 2;
    /*Wait clocks to get active*/
    while(CM_WKUP_UART0_CLKCTRL->BIT.IDLEST);

    CONF_UART0_RXD->LONG = 0x70;
    CONF_UART0_TXD->LONG = 0x58;

    /*Reset UART*/
    UART0_SYSC->BIT.SOFTRESET = 1;
    while(UART0_SYSS->BIT.RESETDONE == 0);

    float div = 48000000.0f/(16.0f*(float)baud);
    unsigned int intdiv = (unsigned int) div;

    UART0_IER->BYTE =  0x00;         // Disable interrupts
    UART0_MDR1->BYTE = 0x07;         // Disable mode select

    UART0_LCR->BYTE =  ~(0x7C);      // Divisor latch enable, access DLL DHL, set UART as 8bit
    UART0_DLL->BYTE =  0x00;         // DLL = 0
    UART0_DLH->BYTE =  0x00;         // DLH = 0
    UART0_LCR->BYTE =  0x03;         // Set UART 8-bit

    UART0_MCR->BYTE =  0x03;         // Force /rts & /drt to active (low)
    UART0_IIR->BYTE =  0x07;         // Clear FIFOs and enable them

    UART0_LCR->BYTE =  ~(0x7C);      // Divisor latch enable, access DLL DHL, set UART as 8bit
    UART0_DLL->BYTE =  (intdiv & 0xFF);         // DLL = 0
    UART0_DLH->BYTE =  ((intdiv >> 8) & 0x3F);         // DLH = 0
    UART0_LCR->BYTE =  0x03;         // Set UART 8-bit

    UART0_MDR1->BYTE = 0;            // UART 16x oversampleing

    config_mux(uart_pins);
}

void UART0Write(int c) {
    while(UART0_SSR->BIT.TX_FIFO_FULL);
    UART0_THR->BYTE = (uint8_t)c;
}

/* Receive side: the FIFO is drained into a ring by the UART interrupt (INTC 72) and, when interrupts are not on yet, from every read. Three ESC in a row
 * (nothing between them, no more than 1 s apart) set `uart_break`: whatever is running looks at it at its safe points and returns to the menu. The check is off
 * while an xmodem transfer runs (three 0x1B in a row are possible in binary data). */
#define RING 512
static volatile uint8_t ring[RING];
static volatile uint16_t rhead, rtail;
volatile int uart_break;
volatile int uart_break_enable = 1;
static uint8_t esc_run;

static void rx_drain(void) {
    while(UART0_LSR->BIT.RX_FIFO_E) {
        uint8_t c = UART0_RHR->BYTE;
        if(c == 0x1B) { if(++esc_run >= 3 && uart_break_enable) { uart_break = 1; esc_run = 0; rhead = rtail = 0; continue; } }
        else esc_run = 0;
        uint16_t n = (rhead + 1) % RING;
        if(n != rtail) { ring[rhead] = c; rhead = n; }
    }
}

void UART0IRQ(void) { rx_drain(); }

void InitUART0IRQ(void) {
    UART0_IER->BYTE = 0x01;                 /* RHR interrupt (the receive time-out interrupt rides on it) */
}

static void drain_masked(void) {
    uint32_t cpsr; asm volatile("mrs %0, cpsr; cpsid i" : "=r"(cpsr) :: "memory");
    rx_drain();
    if(!(cpsr & 0x80)) asm volatile("cpsie i" ::: "memory");
}

int UART0Ready(void) {
    drain_masked();
    return rhead != rtail;
}

int UART0Getkey(void) {
    drain_masked();
    if(rhead == rtail) return -1;
    uint8_t c = ring[rtail]; rtail = (rtail + 1) % RING;
    return c;
}

char UART0Read(void) {
    int c;
    while((c = UART0Getkey()) < 0) {
        IRQPoll();                          /* the idle loop of the menu: a safe point */
        if(uart_break) { uart_break = 0; return '\r'; }        /* ESC ESC ESC while waiting for a line: end the line, the menu shows */
    }
    return c;
}
