#ifndef __HW_INIT_H__
#define __HW_INIT_H__

#include "main.h"

extern void InitCorePLL(int clock, int div);        /* core PLL */
extern void InitMPUPLL(int clock);                  /* CPU clock */
extern void InitPERPLL(void);                       /* peripheral clock */
extern void InitL3L4Wakeup(void);                   /* the interconnect and wake-up domain clocks */
extern void InitGPIO(void);
extern void InitDDRPLL(void);
extern void InitDDREMIF(void);
extern void InitSPI(int bus_speed);
extern void InitUART0(int baud);
extern void UART0Write(int c);
extern int  UART0Ready(void);                       /* a byte is waiting */
extern char UART0Read(void);
extern int  UART0Getkey(void);                      /* a byte, or -1 if none */
extern void UART0Flush(void);                       /* drop what was typed, clear the ESC flags */
extern volatile int uart_esc_seen;                  /* an ESC arrived: the boot stops at the menu */
extern volatile int uart_break;                     /* three ESC in a row: whatever runs stops at its next safe point */
extern volatile int uart_break_enable;              /* 0 while xmodem runs */
extern void InitDMTimer(void);                      /* the 24 MHz timer that all waits use */
extern double ReadDMTimerSeconds(void);
extern void WaitUSDMTimer(uint32_t us);
extern void WaitMSDMTimer(uint32_t ms);
extern void Reset(void);                            /* reset the SoC */
extern void InitPRU(void);
extern uint32_t rev32(uint32_t value);              /* byte swap */

extern void    I2C0Init(uint32_t bus_speed);
extern int32_t I2C0SendCmd(uint32_t slave, const uint8_t* cmd, uint32_t cmd_len, const uint8_t* data, uint32_t cntr);
extern int32_t I2C0ReadCmd(uint32_t slave, const uint8_t* cmd, uint32_t cmd_len, uint8_t* data, uint32_t cntr);
extern int32_t I2C0Probe(uint32_t slave);           /* the slave answers */

extern void GPSupplyOn(void);                       /* drive ball M1 high: the GreenPAK's supply on a v0.8 or later (nothing on a v0.6) */
extern void GPSupplyDip(uint32_t ms);               /* M1 low for ms, then high again: such a GreenPAK loads its NVM again */
extern void AssertReset(void);                      /* drive the Amiga's RESET and HALT low */
extern void ReleaseReset(void);                     /* let them go (high-Z) */
extern int  ResetHeld(void);                        /* who holds them low now: 1 RESET, 2 HALT, 3 both, 0 neither */
extern int  WaitResetReleased(uint32_t settle_ms, int (*stop)(void));   /* until the host lets both go, then settle_ms more: the ms they were held, or -1
                                                       when stop() (NULL: ESC x3) ended the wait. No time limit: a host keeps a 68000 in reset as long as it likes */
#define RESET_SETTLE_MS 50                          /* after the host's RESET: Agnus needs a moment (the CIAs take writes ~200 ms later) */

#endif
