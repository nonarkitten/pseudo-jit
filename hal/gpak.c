#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "pinmux.h"
#include "init.h"
#include "mmu.h"
#include "ddr.h"
#include "flash.h"
#include "gpmc.h"
#include "gpak.h"
#include "pjit.h"
#include "gpak.h"

#define PAGE_LIMIT 0x0E
#define PAGE_DELAY 25

static uint8_t _bus;
static uint8_t _slave_address;

static void gpak_init(uint8_t bus, uint8_t slave_address) {
    _bus = bus;
    _slave_address = slave_address;
}

static uint8_t gpak_i2caddr(uint8_t area) {
    if(!_slave_address) return 0x88;
    else return (_slave_address << 3) + area;
}

static void gpak_read(uint8_t area, uint8_t offset, uint8_t* data, uint16_t length) {
    uint8_t addr = gpak_i2caddr(area);
    while(length > 0) {
        uint8_t xferlen = (length > 16) ? 16 : length;
        I2C0ReadCmd(addr, &offset, 1, data, xferlen);
        offset += xferlen;
        length -= xferlen;
        data += xferlen;
    }
}

static void gpak_write(uint8_t area, uint8_t offset, uint8_t* data, uint16_t length) {
    uint8_t addr = gpak_i2caddr(area);
    //if((offset + length) >= (PAGE_LIMIT << 4)) return;
    while(length > 0) {
        uint8_t xferlen = (length > 16) ? 16 : length;
        I2C0SendCmd(addr, &offset, 1, data, xferlen);
        WaitMSDMTimer(PAGE_DELAY);
        offset += xferlen;
        length -= xferlen;
        data += xferlen;
    }
}

static void gpak_write_reg(uint8_t reg, uint8_t value) {
    uint8_t addr = gpak_i2caddr(REG_CONFIG);
    uint8_t data[1] = { value };
    I2C0SendCmd(addr, &reg, 1, data, 1);
}

static uint8_t gpak_read_reg(uint8_t reg) {
    uint8_t addr = gpak_i2caddr(REG_CONFIG);
    uint8_t data[1] = { 0 };
    I2C0ReadCmd(addr, &reg, 1, data, 1);
    return data[0];
}

static void gpak_erase(uint8_t area) {
    static const uint8_t cmd = 0xE3;
    uint8_t addr = gpak_i2caddr(area);
    uint8_t data[1] = { 0 };
    for(int i=0; i<16; i++) {
        //if(i >= PAGE_LIMIT) break;
        if(area == NVM_CONFIG) data[0] = 0x80 + i;
        else if(area == EEPROM) data[0] = 0x90 + i;
        I2C0SendCmd(addr, &cmd, 1, data, 1);
        WaitMSDMTimer(PAGE_DELAY);
    }
}

// Mask determines all the settable (not reserved) bits
//                      ↓↓ 0 1 2 3 4 5 6 7 8 9 a b c d e f
static const char nvmMask0[] =  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
static const char nvmMask1[] =  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
static const char nvmMask2[] =  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
static const char nvmMask3[] =  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
static const char nvmMask4[] =  "FFFFFFFFFFFFFFFF0000000000000000";
static const char nvmMask5[] =  "00000000000000000000000000000000";
static const char nvmMask6[] =  "03FF7F00FFFF7F7F1E1EFEFE7F007F7F";
static const char nvmMask7[] =  "7F7F7F7FFFFFFFFFFFFFFFFFFFFFFFFF";
static const char nvmMask8[] =  "FFFFFF7E7F00000C130000FFFF380800";
static const char nvmMask9[] =  "FFFFFFFFFFFFFFFFFFFFFF7FFF7F0000";
static const char nvmMask10[] = "FFFFFFFFE7FFFFFFFFFFFFFFFFFF6FFF";
static const char nvmMask11[] = "FEFFFFFFFFFFFF6BFFFEFFFFFFFFFFFF";
static const char nvmMask12[] = "6BFFFFFFFF03FFFF06FFFFFF00000000";
static const char nvmMask13[] = "00000000000000000000000000000000";
static const char nvmMask14[] = "00000000000000000000000000000000";
static const char nvmMask15[] = "00000000000000000000000000000000";
//                      ↑↑ 0 1 2 3 4 5 6 7 8 9 a b c d e f

static const char* const nvmMasks[16] = {
    nvmMask0,  nvmMask1,  nvmMask2,  nvmMask3,
    nvmMask4,  nvmMask5,  nvmMask6,  nvmMask7,
    nvmMask8,  nvmMask9,  nvmMask10, nvmMask11,
    nvmMask12, nvmMask13, nvmMask14, nvmMask15
};

// Slave Address for GreenPAK
#define SLAVE_ADDRESS 1
#define NVM_CONFIG 0x02
#define NVM_BYTES  240

// Store nvmData in to save on RAM
static const char nvmString0[] = "1D702007C200CAC2240B030000000000";
static const char nvmString1[] = "000000000000D0384600000D00000000";
static const char nvmString2[] = "00000000000000000000000000000000";
static const char nvmString3[] = "000000D23F3DD00F00D1090000000000";
static const char nvmString4[] = "00000000000000000000000000000000";
static const char nvmString5[] = "00000000000000000000000000000000";
static const char nvmString6[] = "00010000808000010000808000000000";
static const char nvmString7[] = "00000001000000000000000000000000";
static const char nvmString8[] = "00010000001422300C00000000000000";
static const char nvmString9[] = "DAE50000EAEAAAAA0000000000000000";
static const char nvmString10[] = "00FFEF20000100020000010000020001";
static const char nvmString11[] = "00000201000002000100000201000002";
static const char nvmString12[] = "00010000020001000000020100000000";
static const char nvmString13[] = "00000000000000000000000000000000";
////                               ↓↓ 0 1 2 3 4 5 6 7 8 9 a b c d e f
static const char nvmString14[] = "00000000000000000000000000000000";
////                               ↑↑ 0 1 2 3 4 5 6 7 8 9 a b c d e f
static const char nvmString15[] = "000000000000000000000000000000A5";

static const char* const nvmString[16] = {
    nvmString0,  nvmString1,  nvmString2,  nvmString3,
    nvmString4,  nvmString5,  nvmString6,  nvmString7,
    nvmString8,  nvmString9,  nvmString10, nvmString11,
    nvmString12, nvmString13, nvmString14, nvmString15
};


static uint8_t nvmData[256] = { 0 };
static uint8_t nvmMask[256] = { 0 };

static uint8_t toHex2(char* n) {
    static const char TOHEX[32] = {
        0, 10, 11, 12, 13, 14, 15,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
        0,  0,  0,  0,  0,  0,
    };
    return (TOHEX[n[0] & 0x1F] << 4) | TOHEX[n[1] & 0x1F];
}

static void InitNvm(void) {
    for (int i = 0; i < 16; i++) {
        for (int b = 0; b < 16; b++) {
            uint8_t D = toHex2(nvmString[i] + (b << 1));
            uint8_t M = toHex2(nvmMasks[i] + (b << 1));
            nvmData[(i << 4) + b] = D;
            nvmMask[(i << 4) + b] = M;
        }
    }

    nvmData[0xCA] = SLAVE_ADDRESS;
    gpak_init(0, SLAVE_ADDRESS);  
}

////////////////////////////////////////////////////////////////////////////////
// ReadChip 
////////////////////////////////////////////////////////////////////////////////
static int ReadChip(int dump) {
    uint8_t data[256];

    int same = 1;
    if (dump) printf("[I2C0] Dumping GreenPAK...\n");
    gpak_read(NVM_CONFIG, 0, data, 256);
    for (int page = 0; page < 16; page++) {
        if (dump) printf("       %0X0: ", page);
        for (int b = 0; b < 16; b++) {
            uint8_t m = nvmMask[(page << 4) | b];
            uint32_t c =
                (nvmData[(page << 4) | b] & m)
                ==
                (data[(page << 4) | b] & m);

            if (dump) printf(" %02X%c", data[(page << 4) | b], c ? ' ' : 'x');
            same &= c;
        }
        if (dump) printf("\n");
    }
    return same;
}

static void ProgramChip(int dump) {
    printf("[I2C0] Erasing GreenPAK...\n");
    gpak_erase(NVM_CONFIG);
    printf("[I2C0] Resetting GreenPAK...\n");
    gpak_write_reg(0xC8, 0x02);
    WaitUSDMTimer(300);
    printf("[I2C0] Programming GreenPAK...\n");
    gpak_write(NVM_CONFIG, 0, nvmData, NVM_BYTES);
    printf("[I2C0] Resetting GreenPAK...\n");
    gpak_write_reg(0xC8, 0x02);
    WaitUSDMTimer(300);
    printf("[I2C0] GreenPAK %s\n", ReadChip(dump) ? "ok." : "bad!");
}

static void SetGreenPAKAddr(void) {
    char option[4];
    uint8_t addr = 0xFF;
    printf("Enter address (0-F): ");
    gets(option);
    /**/ if (option[0] >= '0' && option[0] <= '9') addr = option[0] - '0';
    else if (option[0] >= 'A' && option[0] <= 'F') addr = option[0] - 'A' + 10;
    else if (option[0] >= 'a' && option[0] <= 'f') addr = option[0] - 'a' + 10;
    else return;
    printf("Setting address to %X, ", addr);
    if (confirm()) {
        gpak_init(0, addr);
        nvmData[0xCA] = addr;
        printf("Done\n");
    }
}

void ManageGP(void) {
    char option[4] = { 0 };
    while(option[0] != 'x' && option[0] != 'X') {
        printf("%s",
            "1. Set GreenPAK Address\n"
            "2. Dump & Verify GreenPAK\n"
            "3. Erase & Program GreenPAK\n"
            "X. Exit to main menu\n"
            "] "
        );
        gets(option);
        switch(option[0]) {
        case '1': if(confirm()) SetGreenPAKAddr(); break;
        case '2':  break;
        case '3': if(confirm()) ProgramChip(1); break;
        }
    }
}

int DetectGP(void) {
    if (I2C0Probe(0x8)) {
        printf("[I2C0] 000_10xx ($8~$A) GreenPAK Detected\n");
        InitNvm();
        if(cpu_state.config.post_enable_gpack_ok) {
            printf("[I2C0] GreenPAK Protection Bits: $%02X $%02X $%02X\n", 
            gpak_read_reg(0xE0), // Register Read/Write Protection Bits
            gpak_read_reg(0xE1), // NVM Configuration Protection Bits
            gpak_read_reg(0xE4)  // Protection Lock Bit
            );  
            printf("[I2C0] GreenPAK %s\n", ReadChip(0) ? "ok" : "BAD");
        }
    }
}