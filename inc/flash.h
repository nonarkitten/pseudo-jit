#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifndef __HW_FLASH_H__
#define __HW_FLASH_H__

typedef enum {
    ERASE_4K,
    ERASE_32K,
    ERASE_64K,
    ERASE_ALL
} ERASE_SIZE_t;

/*Wait for last action to complete*/
extern void WaitSPI(void);
/*Perform a block/chip erase*/
extern void EraseSPI(uint32_t addr, ERASE_SIZE_t size);
/*Read one byte on SPI*/
extern uint8_t ReadSPI(uint32_t addr);
/*Read Device ID from SPI*/
extern uint16_t ReadSPIID(void);
/*Write one byte on SPI*/
extern void WriteSPI(uint32_t addr, uint8_t data);
/*Read a whole block from SPI*/
extern void ReadSPIBlock(uint32_t addr, void* buffer, uint8_t len);
/*Write a whole block to SPI*/
extern void WriteSPIBlock(uint32_t addr, const void* buffer, uint8_t len);

extern void InitSPI(int bps);
extern void SPITest(void);
extern void SPIDump(void);

#endif