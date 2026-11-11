/// @file shim.h
/// @brief Prototypes for the externs golden/mcl68k.c calls without
///        declaring itself (GetIPLx/UART0Ready/UART0Read/UART0Write) --
///        force-included via -include so a modern C standard (which
///        treats an implicit declaration as a hard error, not just a
///        warning) can compile it unmodified. golden/oracle.c defines
///        all four as no-ops; no test case in this harness's v1 scope
///        touches interrupts or the UART.
#ifndef GOLDEN_SHIM_H
#define GOLDEN_SHIM_H
#include <stdint.h>
extern uint16_t GetIPLx(void);
extern int UART0Ready(void);
extern uint8_t UART0Read(void);
extern void UART0Write(uint8_t ch);
#endif
