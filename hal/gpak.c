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
#include "i2c.h"

extern int core_pll;

#define PAGE_LIMIT 0x0E
#define PAGE_DELAY 100

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

// Slave Address for GreenPAK
#define SLAVE_ADDRESS 1
#define NVM_CONFIG 0x02
#define NVM_BYTES  240

static uint8_t nvmData[256] = { 
        0x20, 0x70, 0x34, 0x47, 0xE3, 0x80, 0x8A, 0xCA, 0x24, 0x2A, 0xC3, 0x2C, 0x06, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0xB0, 0xFC, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x92, 
        0xFD, 0xD8, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0xD2, 0x3F, 0x3D, 0xEB, 0x0F, 0x00, 0x51, 0x00, 0x00, 0xD4, 0x0F, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x01, 0x00, 0x00, 0x80, 0x80, 0x00, 0x01, 0x00, 0x00, 0x80, 0x80, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x01, 0x00, 0x08, 0x00, 0x14, 0x22, 0x30, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0xDA, 0xE6, 0x00, 0x00, 0xEA, 0xEA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x02, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x20, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x02, 0x00, 0x01, 
        0x00, 0xFE, 0x02, 0x01, 0x00, 0x80, 0x02, 0x00, 0x01, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x02, 
        0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA5, 
};

// Mask determines all the settable (not reserved) bits
static const uint8_t nvmMask[256] = {
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // 0x17 don't care
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x03, 0xFF, 0x7F, 0x00, 0xFF, 0xFF, 0x7F, 0x7F, 0x1E, 0x1E, 0xFE, 0xFE, 0x7F, 0x00, 0x7F, 0x7F, 
        0x7F, 0x7F, 0x7F, 0x7F, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

        0xFF, 0xFF, 0xFF, 0x7E, 0x7F, 0x00, 0x00, 0x0C, 0x13, 0x00, 0x00, 0xFF, 0xFF, 0x38, 0x08, 0x00, 
        0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x7F, 0x00, 0x00, // 0x9C don't care
        0xFF, 0xFF, 0xFF, 0xFF, 0xE7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x6F, 0xFF, 
        0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x6B, 0xFF, 0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

        0x6B, 0xFF, 0xFF, 0xFF, 0xFF, 0x03, 0xFF, 0xFF, 0x06, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
};

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
}

typedef enum { GP_BLANK, GP_GOOD, GP_BAD } GreenPAK_State_t;
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
    printf("Enter %saddress (0-F): ", prompt);
    gets(option);
    /**/ if (option[0] >= '0' && option[0] <= '9') return option[0] - '0';
    else if (option[0] >= 'A' && option[0] <= 'F') return option[0] - 'A' + 10;
    else if (option[0] >= 'a' && option[0] <= 'f') return option[0] - 'a' + 10;
    else return -1;
}

static void EraseChip(int dump, int addr) {
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

static void ProgramChip(int dump, int addr) {
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

uint8_t current_dtack_delay;
uint8_t current_rw_delay;
uint8_t current_ds_delay;

/*
    GreenPAK Timing
    ____                 _________
        |_______________|           Original WEn from GPMC
         :               :
         :               :          About 15ns Skew
    _____:               :      ___
         |_______________: --> |    RW from PD OUT0 in 20ns steps
                                    Register 0x9C lower nibble
    _____:               :________
         : --> |_________|          DS from PD OUT1 in 20ns steps
                                    Register 0x9C upper nibble
*/

static void GetGPTiming(int addr, int quiet) {
    static volatile uint8_t t2;
    volatile int ns2 = 0, ns3 = 0;

    if(addr < 0) {
        t2 = nvmData[0x9C];
    } else {
        if(!I2C0Probe(addr << 3)) return;
        t2 = gpak_read_reg(addr, 0x9C);
    }

    ns2 = (1 + (t2 & 0xF)) * 20 + 15;
    ns3 = (1 + (t2 >> 4)) * 20 + 15;

    current_rw_delay = ns2;
    current_ds_delay = ns3;

    if(!quiet) {
        printf("[I2C0] GreenPAK RW Delay %d ns\n", current_rw_delay);
        printf("[I2C0] GreenPAK DS Delay %d ns\n", current_ds_delay);
    }
}

static void SaveGPTiming(int addr, float rw, float ds) {
    uint8_t t2;

    rw = (rw - 15.0f) / 20.0f - 0.5f;
    if(rw > 15) t2 = 0x0F; else t2 = (int)rw;

    ds = (ds - 15.0f) / 20.0f - 0.5f;
    if(ds > 15) t2 |= 0xF0; else t2 |= ((int)ds) << 4;

    nvmData[0x9C] = t2;
    if(addr >= 0) {
        gpak_write_reg(addr, 0x9C, t2);
    }
}

static void SetGPTiming(int addr) {
    char buffer[8] = { 0 };
    float rw, ds;

    GetGPTiming(addr, 1);

    printf("       Enter RW delay (%dns): ", current_rw_delay);
    gets(buffer);
    rw = buffer[0] ? strtod(buffer, NULL) : current_rw_delay;

    printf("       Enter DS delay (%dns): ", current_ds_delay);
    gets(buffer);
    ds = buffer[0] ? strtod(buffer, NULL) : current_ds_delay;

    SaveGPTiming(addr, rw, ds);
} 

void ManageGP(void) {
    char option[4] = { 0 };
    while(option[0] != 'x' && option[0] != 'X') {
        printf("%s",
            "1. Dump & Verify GreenPAK\n"
            "2. Erase GreenPAK\n"
            "3. Program GreenPAK\n"
            "4. Scan I2C Bus\n"
            "5. Get GreenPAK Bus Timing\n"
            "6. Set GreenPAK Bus Timing\n"
            "X. Exit to main menu\n"
            "] "
        );
        gets(option);
        switch(option[0]) {
        // case '1': if(confirm()) SetGreenPAKAddr(); break;
        case '1': printf("[I2C0] GreenPAK %s\n", GP_State[ReadChip(1, GetAddr(""))]); break;
        case '2': EraseChip(1, GetAddr("")); break;
        case '3': ProgramChip(1, GetAddr("new ")); break;
        case '4': ProbeI2C(); break;
        case '5': GetGPTiming(GetAddr(""), 0); break;
        case '6': SetGPTiming(GetAddr("")); break;
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
                GreenPAK_State_t s = ReadChip(0, i);
                printf("[I2C0] GreenPAK %s\n", GP_State[s]);
                if(s == GP_GOOD) {
                    GetGPTiming(i, 0);                    
                } else if(s == GP_BAD) {
                    EraseChip(0, 1);
                    float ns = (float)core_pll / (cpu_state.config.kHz * 0.001f);
                    SaveGPTiming(-1, ns * 0.5f, ns);
                    ProgramChip(0, 1);
                    GetGPTiming(1, 0);
                }
                return 1;
            }
            break;
        }
    }
    if(I2C0Probe(0)) {
        printf("[I2C0] Blank GreenPAK Detected ($00~$03)\n");
        float ns = (float)core_pll / (cpu_state.config.kHz * 0.001f);
        SaveGPTiming(-1, ns * 0.5f, ns);
        ProgramChip(0, 1);        
        GetGPTiming(1, 0);
        return 1;
    } else {
        printf("[I2C0] GreenPAK Not Detected\n");
        return 0;
    }
}