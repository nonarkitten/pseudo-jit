#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "support.h"

#include "pinmux.h"
#include "hw_init.h"
#include "hw_mmu.h"
#include "hw_ddr.h"
#include "hw_flash.h"
#include "hw_gpmc.h"

static uint32_t WriteChunk(uint32_t addr, uint32_t data) {
    data = LE32(data);
    WriteSPIBlock(addr, (uint8_t*)&data, sizeof(data));
    am335x_dmtimer1_wait_us(600);
    return addr + 4;
}

void WriteLoaderImage(uint32_t address, uint32_t len) {
    uint32_t * src = (uint32_t*)address;
    uint32_t data;
    uint32_t pos = 0;

    //len += 4; // add room for zee header

    printf("[SPI0] Programming %u bytes\n", len);
    printf("[SPI0] Erasing block %p\n", (pos >> 12));
    EraseSPI(pos, ERASE_4K);
    am335x_dmtimer1_wait_ms(400);

    // Write length
    pos = WriteChunk(pos, len);

    // Write load address
    pos = WriteChunk(pos, 0x402F0400);

    while(pos < len) {
        if((pos & 0xFFF) == 0) {
            printf("[SPI0] Erasing block %p\n", (pos >> 12));
            EraseSPI(pos, ERASE_4K);
            am335x_dmtimer1_wait_ms(400);
        }
        pos = WriteChunk(pos, *src++); // TODO CRC check?
    }

    printf("[SPI0] Verifying %d bytes\n", len);
    src = (uint32_t*)address;
    pos = 0;

    ReadSPIBlock(pos, (uint8_t*)&data, sizeof(data));
    data = LE32(data);
    printf("[SPI0] Read length %u bytes\n", data);
    if(data != len) goto verify_failed;
    pos += 4;

    ReadSPIBlock(pos, (uint8_t*)&data, sizeof(data));
    data = LE32(data);
    printf("[SPI0] Read load address %p\n", data);
    if(data != 0x402F0400) goto verify_failed;
    pos += 4;

    while(pos < len) {
        ReadSPIBlock(pos, (uint8_t*)&data, sizeof(data));
        data = LE32(data);
        if(data != *src++) goto verify_failed;
        pos += 4;
    }
    printf("[SPI0] Verify passed!\n");
    return;

verify_failed:
    printf("[SPI0] Verify failed at %p!\n", pos);
    return;
}