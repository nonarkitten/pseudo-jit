/*
 * hw_init.c
 *
 *  Created on: Feb 9, 2022
 *      Author: Renee Cousins
 */
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "pinmux.h"
#include "ioam3358.h"

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

int UART0Ready(void) {
    return !!(UART0_LSR->BIT.RX_FIFO_E);
}

char UART0Read(void) {
    while(!(UART0_LSR->BIT.RX_FIFO_E));
    return UART0_RHR->BYTE;
}

int UART0Getkey(void) {
    if(UART0_LSR->BIT.RX_FIFO_E)
        return UART0_RHR->BYTE;
    else
        return -1;
}

