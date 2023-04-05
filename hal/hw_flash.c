/*
 * test_flash.c
 *
 *  Created on: Apr. 17, 2021
 *      Author: renee.cousins
 */

#include "hw_init.h"
#include "hw_flash.h"
#include "pinmux.h"

void SPIInit(int bps) {
	InitSPI(bps);
}

static uint8_t spi_xfer[256];
extern uint8_t TransferSPI(uint8_t *io_buffer, uint8_t len);

void WaitSPI(void) {
    while(1) {
        spi_xfer[0] = 0x05;
        spi_xfer[1] = 0x00;
        TransferSPI( spi_xfer, 2 );
        if(spi_xfer[1] & 1) {
            int i=10000;
            while(i--) asm("    nop");
        } else {
            break;
        }
    }
}

void WriteAllowSPI(void) {
    uint8_t unlock = 0x06;
    TransferSPI(&unlock, 1);
    WaitSPI();
}

void EraseSPI(uint32_t addr, ERASE_SIZE_t size) {
    uint8_t cmd, len;

    switch(size) {
    default: return;
    case ERASE_4K:  cmd = 0x20; addr &= 0xFFFFF000; len = 4; break;
    case ERASE_32K: cmd = 0x52; addr &= 0xFFFF8000; len = 4; break;
    case ERASE_64K: cmd = 0xD8; addr &= 0xFFFF0000; len = 4; break;
    case ERASE_ALL: cmd = 0x60; addr = 0; len = 1; break;
    }

    WriteAllowSPI();

    spi_xfer[0] = cmd;
    spi_xfer[1] = (addr >> 16) & 0xFF;
    spi_xfer[2] = (addr >>  8) & 0xFF;
    spi_xfer[3] = (addr >>  0) & 0xFF;
    TransferSPI( spi_xfer, len );
    WaitSPI();
}

uint16_t ReadSPIID(void) {
    spi_xfer[0] = 0x90;
    spi_xfer[1] = 0;
    spi_xfer[2] = 0;
    spi_xfer[3] = 0;
    spi_xfer[4] = 0;
    TransferSPI( spi_xfer, 6 );
    return (spi_xfer[4] << 8) | spi_xfer[5];
}

uint8_t ReadSPI(uint32_t addr) {
    spi_xfer[0] = 0x03;
    spi_xfer[1] = (addr >> 16) & 0xFF;
    spi_xfer[2] = (addr >>  8) & 0xFF;
    spi_xfer[3] = (addr >>  0) & 0xFF;
    spi_xfer[4] = 0;
    TransferSPI( spi_xfer, 5 );
    return spi_xfer[4];
}

void ReadSPIBlock(uint32_t addr, void* buffer, uint8_t len) {
    spi_xfer[0] = 0x03;
    spi_xfer[1] = (addr >> 16) & 0xFF;
    spi_xfer[2] = (addr >>  8) & 0xFF;
    spi_xfer[3] = (addr >>  0) & 0xFF;
    memset(&spi_xfer[4], 0, len);
    TransferSPI( spi_xfer, len + 4 );
    memcpy(buffer, &spi_xfer[4], len);
    return;
}

void WriteSPI(uint32_t addr, uint8_t data) {
    WriteAllowSPI();

    spi_xfer[0] = 0x02;
    spi_xfer[1] = (addr >> 16) & 0xFF;
    spi_xfer[2] = (addr >>  8) & 0xFF;
    spi_xfer[3] = (addr >>  0) & 0xFF;
    spi_xfer[4] = data;
    TransferSPI( spi_xfer, 5 );
    WaitSPI();
    return;
}

void WriteSPIBlock(uint32_t addr, const void* buffer, uint8_t len) {
    WriteAllowSPI();

    spi_xfer[0] = 0x02;
    spi_xfer[1] = (addr >> 16) & 0xFF;
    spi_xfer[2] = (addr >>  8) & 0xFF;
    spi_xfer[3] = (addr >>  0) & 0xFF;
    memcpy(&spi_xfer[4], buffer, len);
    TransferSPI( spi_xfer, len + 4 );
    WaitSPI();
    return;
}

void SPITest(void) {
    const uint32_t write = 0xAA55AA55;
    const uint32_t clear = 0x00000000;
    uint32_t read = 0;
	bool all_good = true;

	uint16_t devid = ReadSPIID();
	all_good &= devid == 0x1F16;
	printf("[FLASH] Flash Device ID: %04x\n", devid);
	printf("[FLASH] Erasing page 0\n");
	EraseSPI(0, ERASE_4K);

    printf("[FLASH] Writing pattern block\n");
    for(int i = 0; i < 4; i++) WriteSPIBlock(i * 4, &write, sizeof(write));
    printf("[FLASH] Read:");
    for(int i = 0; i < 4; i++) {
        ReadSPIBlock(i * 4, &read, sizeof(read));
		all_good &= read == write;
		printf(" %08lX", read);
	}
    printf("\n");

    printf("[FLASH] Writing zero block\n");
    for(int i = 0; i < 4; i++) WriteSPIBlock(i * 4, &clear, sizeof(clear));
    printf("[FLASH] Read:");
    for(int i = 0; i < 4; i++) {
        ReadSPIBlock(i * 4, &read, sizeof(read));
		all_good &= read == clear;
		printf(" %08lX", read);
	}
    printf("\n");
	printf("[FLASH] Tests %s\n", (all_good ? "passed" : "failed"));
}

void SPIDump(void) {
    const uint32_t len = 4096;
    uint32_t data;
    uint32_t pos = 0;

    printf("[FLASH] Reading first 4KB:\n");
    while(pos < len) {
        ReadSPIBlock(pos, &data, sizeof(data));
        if((pos & 15) == 0) printf("\n%08lX:", pos);
        printf(" %08lX", data);
        pos += 4;
    }
    printf("\n");
}

