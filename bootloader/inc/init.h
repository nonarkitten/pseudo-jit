#ifndef __HW_INIT_H__
#define __HW_INIT_H__

#include "main.h"

extern void InitCorePLL(int clock, int div);        /* core PLL */
extern void InitMPUPLL(int clock);                  /* CPU clock */
extern void InitPERPLL(void);                       /* peripheral clock */
extern void InitDDRPLL(void);
extern void InitDDREMIF(void);
extern void InitSPI(int bus_speed);
extern void InitUART0(int baud);
extern void UART0Write(int c);
extern int  UART0Ready(void);                       /* a byte is waiting */
extern char UART0Read(void);
extern int  UART0Getkey(void);                      /* a byte, or -1 if none */
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

extern void AssertReset(void);                      /* drive the Amiga's RESET and HALT low */
extern void ReleaseReset(void);                     /* let them go (high-Z) */

#endif
