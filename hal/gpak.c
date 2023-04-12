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
#define PAGE_DELAY 100

// static uint8_t _bus;
// static uint8_t slave_address;

// static void gpak_init(uint8_t bus, uint8_t slave_address) {
//     // _bus = bus;
//     _slave_address = slave_address;
// }

static void gpak_read(uint8_t addr, uint8_t offset, uint8_t* data, uint16_t length) {
    I2C0ReadCmd((addr << 3) | 0x02, &offset, 1, data, length);
}

static int gpack_poll_ack(uint8_t addressForAckPolling) {
    int nack_count = 0;

    while(1) {
        if(I2C0Probe(addressForAckPolling)) return 0;
        if(nack_count >= 1000) {
            printf("Geez! Something went wrong while programming!\n");
            return -1;
        }
        nack_count++;
        WaitMSDMTimer(1);
    }
}

static int gpak_write(uint8_t offset, uint8_t* data) {
    // Set the control code to 0x00 since the chip has just been erased
    uint8_t control_code = 0x02;
    uint8_t addressForAckPolling = 0x00;

    if(!I2C0SendCmd(control_code, &offset, 1, data, 16)) {
        printf("Oh No! Something went wrong while programming!\n");
        return 1;
    }
    if(gpack_poll_ack(addressForAckPolling) == -1) 
        return 1;
    WaitMSDMTimer(PAGE_DELAY);
    return 0;
}

static int gpak_erase(uint8_t addr, uint8_t page) {
    static const uint8_t cmd = 0xE3;
    uint8_t control_code = addr << 3;
    uint8_t addressForAckPolling = control_code;
    uint8_t data = 0x80 | page;

    I2C0SendCmd_GPFix(control_code, &cmd, 1, &data, 1);
    if(gpack_poll_ack(addressForAckPolling) == -1)
        return 1;
    WaitMSDMTimer(PAGE_DELAY);
    return 0;
    // }
}

static void gpak_write_reg(uint8_t addr, uint8_t reg, uint8_t value) {
    // uint8_t addr = gpak_i2caddr(REG_CONFIG);
    uint8_t control_code = addr << 3;
    uint8_t data[1] = { value };
    I2C0SendCmd(control_code, &reg, 1, data, 1);
}

static uint8_t gpak_read_reg(uint8_t addr, uint8_t reg) {
    // uint8_t addr = gpak_i2caddr(REG_CONFIG);
    uint8_t control_code = addr << 3;
    uint8_t data[1] = { 0 };
    I2C0ReadCmd(control_code, &reg, 1, data, 1);
    return data[0];
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

uint32_t toHex2(char* n, int len) {
    static const char TOHEX[32] = {
        0, 10, 11, 12, 13, 14, 15,
        0,  0,  0,  0,  0,  0,  0,  0,  0,
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9,
        0,  0,  0,  0,  0,  0,
    };
    uint32_t value = 0;
    while(len--) value = (value << 4) | TOHEX[(*n++) & 0x1f];
    return value; //(TOHEX[n[0] & 0x1F] << 4) | TOHEX[n[1] & 0x1F];
}

static void InitNvm(void) {
    for (int i = 0; i < 16; i++) {
        for (int b = 0; b < 16; b++) {
            uint8_t D = toHex2(nvmString[i] + (b << 1), 2);
            uint8_t M = toHex2(nvmMasks[i] + (b << 1), 2);
            nvmData[(i << 4) + b] = D;
            nvmMask[(i << 4) + b] = M;
        }
    }

    // nvmData[0xCA] = SLAVE_ADDRESS;
    // gpak_init(0, SLAVE_ADDRESS);  
}

typedef enum {
    GP_BLANK,
    GP_GOOD,
    GP_BAD,
} GreenPAK_State_t;

const char* GP_State[] = { "blank", "good", "bad" };

////////////////////////////////////////////////////////////////////////////////
// ReadChip 
////////////////////////////////////////////////////////////////////////////////
static GreenPAK_State_t ReadChip(int dump, uint32_t addr) {
    uint8_t data[16] = { 0 };
    int blank = 1;
    int same = 1;

    //if(addr > 0xF) return 0;
    if(!I2C0Probe(addr << 3)) return 0;

    if (dump) printf("[I2C0] Dumping GreenPAK...\n");
    for (int page = 0; page < 16; page++) {
        if (dump) printf("       %0X0: ", page);
        gpak_read(addr, page << 4, data, 16);
        for (int b = 0; b < 16; b++) {
            uint8_t m = nvmMask[(page << 4) | b];
            uint32_t c = (nvmData[(page << 4) | b] & m) == (data[b] & m);
            blank &= data[b] == 0;
            if (dump) printf(" %02X%c", data[b], c ? ' ' : 'x');
            same &= c;
        }
        if (dump) printf("\n");
    }
    return blank ? GP_BLANK : same ? GP_GOOD : GP_BAD;
}

static int GetAddr(const char* prompt) {
    char option[4];
    uint8_t addr = 0xFF;
    printf("Enter %saddress (0-F): ", prompt);
    gets(option);
    /**/ if (option[0] >= '0' && option[0] <= '9') return option[0] - '0';
    else if (option[0] >= 'A' && option[0] <= 'F') return option[0] - 'A' + 10;
    else if (option[0] >= 'a' && option[0] <= 'f') return option[0] - 'a' + 10;
    else return -1;
}

static void EraseChip(int dump) {
    int addr = GetAddr("");
    if(addr == -1) return;
    printf("[I2C0] Erasing GreenPAK...\n");
    for (int page = 0; page < 16; page++) {
        if(gpak_erase(addr, page)) return;
    }
    printf("[I2C0] Resetting GreenPAK...\n");
    gpak_write_reg(addr, 0xC8, 0x02);
    WaitUSDMTimer(500);
    printf("[I2C0] GreenPAK %s\n", GP_State[ReadChip(dump, 0)]);
}

static void ProgramChip(int dump) {
    int addr = GetAddr("new ");
    if(addr == -1) return;
    nvmData[0xCA] = addr;
    printf("[I2C0] Programming GreenPAK...\n");
    for (int page = 0; page < 16; page++) {
        if(gpak_write(page << 4, &nvmData[page << 4])) return;
    }
    printf("[I2C0] Resetting GreenPAK...\n");
    gpak_write_reg(0, 0xC8, 0x02);
    WaitUSDMTimer(1500);

    printf("[I2C0] GreenPAK %s\n", GP_State[ReadChip(dump, addr)]);
}

static void ProbeI2C(void) {
    for (int i = 0; i < 120; i++) {
        if ((i & 0x7) == 0) printf("\n%02X:", i);
        if (I2C0Probe(i)) printf(" %02x", i);
        else printf(" --");
    }
    printf("\n");
}

void ManageGP(void) {
    char option[4] = { 0 };
    while(option[0] != 'x' && option[0] != 'X') {
        printf("%s",
            "1. Dump & Verify GreenPAK\n"
            "2. Erase GreenPAK\n"
            "3. Program GreenPAK\n"
            "4. Scan I2C Bus\n"
            "X. Exit to main menu\n"
            "] "
        );
        gets(option);
        switch(option[0]) {
        // case '1': if(confirm()) SetGreenPAKAddr(); break;
        case '1': printf("[I2C0] GreenPAK %s\n", GP_State[ReadChip(1, GetAddr(""))]); break;
        case '2': EraseChip(1); break;
        case '3': ProgramChip(1); break;
        case '4': ProbeI2C(); break;
        }
    }
}

int DetectGP(void) {
    InitNvm();
    for(int i=1; i<8; i++) {
        if (I2C0Probe(i << 3)) {
            nvmData[0xCA] = i;
            printf("[I2C0] GreenPAK Detected ($%02X~$%02X) \n", (i << 3), ((i << 3) + 3));
            if(cpu_state.config.post_enable_gpack_ok) {
                printf("[I2C0] GreenPAK Protection Bits: $%02X $%02X $%02X\n", 
                gpak_read_reg(i, 0xE0), // Register Read/Write Protection Bits
                gpak_read_reg(i, 0xE1), // NVM Configuration Protection Bits
                gpak_read_reg(i, 0xE4)  // Protection Lock Bit
                );  
                printf("[I2C0] GreenPAK %s\n", GP_State[ReadChip(0, i)]);
                return;
            }
            break;
        }
    }
    if(I2C0Probe(0)) {
        printf("[I2C0] Blank GreenPAK Detected ($00~$03)\n");
    } else {
        printf("[I2C0] GreenPAK Not Detected\n");
    }
}