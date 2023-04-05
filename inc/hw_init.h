#ifndef __HW_INIT_H__
#define __HW_INIT_H__

#include "main.h"

/*Core PLL init*/
extern void InitCorePLL(int clock);
/*MPU Clock init*/
extern void InitMPUPLL(int clock);
/*Peripheral Clock init*/
extern void InitPERPLL(void);
/*DDR PLL Init*/
extern void InitDDRPLL(void);
/*DDR Init*/
extern void InitDDR(void);
/*SPI Init*/
extern void InitSPI(int bus_speed);
/*SPI Transfer*/
extern uint8_t TransferSPI(uint8_t *io_buffer, uint8_t len);
/*Init UART*/
extern void UART0Init(int baud);
/*Write byte to UART*/
extern void UART0Write(int c);
/*Check if UART has data*/
extern int UART0Ready(void);
/*Read one byte from UART*/
extern char UART0Read(void);
/*Get any key or return -1 if none*/
extern int UART0Getkey(void);
/*Initialize the timer*/
extern void InitDMTimer(void);
/*Read current count*/
extern uint32_t ReadDMTimer(void);
/*Read current count in seconds*/
extern double ReadDMTimerSeconds(void);
/*Wait specified microseconds*/
extern void WaitUSDMTimer(uint32_t us);
/*Wait specified milliseconds*/
extern void WaitMSDMTimer(uint32_t us);
/*Reset MCU*/
extern void Reset(void);
/*Initialize GPMC*/
extern void GPMCInit(void);
/*Configure GPMC*/
extern void GPMCConfig(const uint32_t config[6], uint32_t cs, uint32_t base, uint32_t size);
/*Unconfigure GPMC*/
extern void GPMCDeInit(uint32_t cs);
/*Initialize PRU*/
extern void InitPRU(void);
/*Stub for byte swap */
extern uint32_t rev32(uint32_t value);
/*Inits I2C0 Module. 400KHz*/
extern void I2C0Init(uint32_t bus_speed);
/*Sends data to slave*/
extern int32_t I2C0SendCmd(uint32_t slave, uint8_t cmd, uint8_t * data, uint32_t cntr);
/*Reads data from slave*/
extern int32_t I2C0ReadCmd(uint32_t slave, uint8_t cmd, uint8_t * data, uint32_t cntr);
/*Checks if slave is present*/
extern int32_t I2C0Probe(uint32_t slave);
/*Assert Reset and Halt by driving low*/
extern void AssertReset(void);
/*Release Reset and Halt by setting to high-Z*/
extern void ReleaseReset(void);
#endif