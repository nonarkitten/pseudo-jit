/*
 * console_uart.c — PL011 UART driver for QEMU's "virt" machine, wired up
 * as the pjit_console_putc() backend that platform/pjit_libc.c's printf()
 * calls into.
 *
 * QEMU's `-nographic` flag multiplexes this UART to the host terminal, so
 * anything printed here is what tests/run.sh captures and greps.
 */
#include <stdint.h>

#define UART0_BASE 0x09000000u
#define UARTDR  (*(volatile uint32_t *)(UART0_BASE + 0x000))
#define UARTFR  (*(volatile uint32_t *)(UART0_BASE + 0x018))
#define UARTFR_TXFF (1u << 5)  /* transmit FIFO full */

void pjit_console_putc(char c)
{
    if (c == '\n') pjit_console_putc('\r');
    while (UARTFR & UARTFR_TXFF) { }
    UARTDR = (uint32_t)(uint8_t)c;
}
