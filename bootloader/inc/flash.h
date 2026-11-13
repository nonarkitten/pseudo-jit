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
/*Read Device ID from SPI*/
extern uint16_t ReadSPIID(void);
/*Read a whole block from SPI*/
extern void ReadSPIBlock(uint32_t addr, void* buffer, uint8_t len);
extern void ReadSPIBulk(uint32_t addr, void* buffer, uint32_t len);   /* len a multiple of 4: 32-bit words, fast read, 48 MHz (spi_bulk_div) */
extern int spi_bulk_div, spi_bulk_mode, spi_bulk_err;
/*Write a whole block to SPI*/
extern void WriteSPIBlock(uint32_t addr, const void* buffer, uint8_t len);

extern void InitSPI(int bps);

#endif