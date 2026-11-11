/// @file console_uart.c
/// @brief PL011 console of the realview-pb-a8 board (QEMU), at 0x10009000.
#include <stdint.h>
#define UART0_BASE 0x10009000u
#define UARTDR  (*(volatile uint32_t *)(UART0_BASE + 0x000))
#define UARTFR  (*(volatile uint32_t *)(UART0_BASE + 0x018))
void pjit_console_putc(char c) { if (c == '\n') pjit_console_putc('\r'); while (UARTFR & (1u << 5)) { } UARTDR = (uint32_t)(uint8_t)c; }
