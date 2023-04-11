#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "pinmux.h"
#include "hw_init.h"
#include "hw_mmu.h"
#include "hw_ddr.h"
#include "hw_flash.h"
#include "hw_gpmc.h"
#include "gpak.h"
#include "pjit.h"

#define NAK     0x15
#define CANCEL  0x18

static int confirm(void) {
    char query[4];
    printf("Are you sure [y/N]? ");
    gets(query);
    if (query[0] == 'y' || query[0] == 'Y') {
        return true;
    } else {
        printf("Cancelled\n");
        return false;
    }
}

#define PRU_SHARED_RAM (SOC_PRUICSS1_REGS + SOC_PRUICSS_SHARED_RAM_OFFSET)

static void SetEClock(void) {
    volatile uint32_t* smem = (volatile uint32_t*)0x4A310000;
    char query[4];

    printf("Set E Clock divisor [6-10]: ");
    gets(query);
    uint32_t div = atoi(query);
    if(div) {
        smem[1] = 0;
        smem[0] = div;
        WaitMSDMTimer(100);
        div = smem[1];
        printf("E Clock divisor set to %d\n", div);
    } else {
        printf("Cancelled\n");
    }
}

static int GetPRUClock(void) {
    volatile uint32_t* smem = (volatile uint32_t*)0x4A310000;
    return (int)smem[2];
}

int pmic_detected;
int gpak_detected;
int prom_detected;

// Mask determines all the settable (not reserved) bits
//                      ↓↓ 0 1 2 3 4 5 6 7 8 9 a b c d e f
const char nvmMask0[] =  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
const char nvmMask1[] =  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
const char nvmMask2[] =  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
const char nvmMask3[] =  "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF";
const char nvmMask4[] =  "FFFFFFFFFFFFFFFF0000000000000000";
const char nvmMask5[] =  "00000000000000000000000000000000";
const char nvmMask6[] =  "03FF7F00FFFF7F7F1E1EFEFE7F007F7F";
const char nvmMask7[] =  "7F7F7F7FFFFFFFFFFFFFFFFFFFFFFFFF";
const char nvmMask8[] =  "FFFFFF7E7F00000C130000FFFF380800";
const char nvmMask9[] =  "FFFFFFFFFFFFFFFFFFFFFF7FFF7F0000";
const char nvmMask10[] = "FFFFFFFFE7FFFFFFFFFFFFFFFFFF6FFF";
const char nvmMask11[] = "FEFFFFFFFFFFFF6BFFFEFFFFFFFFFFFF";
const char nvmMask12[] = "6BFFFFFFFF03FFFF06FFFFFF00000000";
const char nvmMask13[] = "00000000000000000000000000000000";
const char nvmMask14[] = "00000000000000000000000000000000";
const char nvmMask15[] = "00000000000000000000000000000000";
//                      ↑↑ 0 1 2 3 4 5 6 7 8 9 a b c d e f

const char* const nvmMasks[16] = {
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
const char nvmString0[] = "1D702007C200CAC2240B030000000000";
const char nvmString1[] = "000000000000D0384600000D00000000";
const char nvmString2[] = "00000000000000000000000000000000";
const char nvmString3[] = "000000D23F3DD00F00D1090000000000";
const char nvmString4[] = "00000000000000000000000000000000";
const char nvmString5[] = "00000000000000000000000000000000";
const char nvmString6[] = "00010000808000010000808000000000";
const char nvmString7[] = "00000001000000000000000000000000";
const char nvmString8[] = "00010000001422300C00000000000000";
const char nvmString9[] = "DAE50000EAEAAAAA0000000000000000";
const char nvmString10[] = "00FFEF20000100020000010000020001";
const char nvmString11[] = "00000201000002000100000201000002";
const char nvmString12[] = "00010000020001000000020100000000";
const char nvmString13[] = "00000000000000000000000000000000";
////                               ↓↓ 0 1 2 3 4 5 6 7 8 9 a b c d e f
const char nvmString14[] = "00000000000000000000000000000000";
////                               ↑↑ 0 1 2 3 4 5 6 7 8 9 a b c d e f
const char nvmString15[] = "000000000000000000000000000000A5";

const char* const nvmString[16] = {
    nvmString0,  nvmString1,  nvmString2,  nvmString3,
    nvmString4,  nvmString5,  nvmString6,  nvmString7,
    nvmString8,  nvmString9,  nvmString10, nvmString11,
    nvmString12, nvmString13, nvmString14, nvmString15
};

#ifndef VERSION
#define VERSION "n/a"
#endif
static const char* Version = VERSION;

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

static void ProbeI2C(void) {
    for (int i = 0; i < 120; i++) {
        if ((i & 0x7) == 0) printf("\n%02X:", i);
        if (I2C0Probe(i)) printf(" %02x", i);
        else printf(" --");
    }
    printf("\n");
}

static unsigned short crc16_ccitt(const void *buf, uint32_t len, uint16_t init) {
    static const unsigned short crc16_table[256]= {
        0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
        0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
        0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
        0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
        0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
        0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
        0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
        0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
        0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
        0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
        0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
        0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
        0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
        0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
        0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
        0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
        0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
        0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
        0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
        0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
        0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
        0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
        0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
        0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
        0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
        0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
        0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
        0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
        0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
        0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
        0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
        0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
    };
    uint8_t *_b = (uint8_t *)buf;
    uint16_t crc = init;
    while (len--) {
        crc = (crc << 8) ^ crc16_table[((crc >> 8) ^ *_b++) & 0x00FF];
    }
    return crc;
}

static void SaveConfigI2C(int boot_good) {
    uint8_t addr[2] = { 0 };
    const uint8_t* config = 6 + (uint8_t*)&cpu_state.config;
    config_t i2c_config;

    cpu_state.config.is_dirty = 0;
    cpu_state.config.last_boot_good = boot_good;
    cpu_state.config.crc16 = crc16_ccitt(config, sizeof(config_t) - 6, 0xFFFF);    
    printf("[I2C0] Saving and verifying settings\n");
    I2C0SendCmd( 0x50, addr, 2, &cpu_state.config, sizeof(config_t));
    I2C0ReadCmd( 0x50, addr, 2, &i2c_config, sizeof(config_t));
    if(memcmp(&i2c_config, &cpu_state.config, sizeof(config_t))) {
        printf("[I2C0] Verify failed! EEPROM may be bad\n");
    }
}
static const char zero = 0;
static const char one = 1;

static void MakeGoodI2C(void) {
    uint8_t addr[2] = { 0 };
    addr[1] = __offsetof(config_t, last_boot_good);
    I2C0SendCmd( 0x50, addr, 2, &zero, 1);
}

static void LoadConfigI2C(void) {
    uint8_t addr[2] = { 0 };
    const uint8_t* config = 6 + (uint8_t*)&cpu_state.config;
    config_t i2c_config;
	int err = 0;

    I2C0ReadCmd( 0x50, addr, 2, &i2c_config, sizeof(config_t));
    if(i2c_config.ident == 0x704A4954) {
        uint16_t calc_crc = crc16_ccitt(config, sizeof(config_t) - 6, 0xFFFF);
        if(calc_crc == i2c_config.crc16) {
            printf("[I2C0] Settings loaded, last boot was %s\n", i2c_config.last_boot_good ? "good" : "bad");
            MakeGoodI2C();
            cpu_state.config = i2c_config;
            return;
        } else {
            printf("[I2C0] CRC fail, default settings loaded\n");
        }
    } else {
        printf("[I2C0] Ident wrong, default settings loaded\n");
    }
    cpu_state.config = default_config;
    cpu_state.config.is_dirty = 1;
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

static void ManageGP(void) {
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

static void InitResetHalt(void) {
    InitGPIO();
    AssertReset();
    static const pin_muxing_t reset_pins[] = {
        { CONF_UART1_RTSN,  (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M7) }, /* HALT gpio0_13 */
        { CONF_MCASP0_AXR0, (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M7) }, /* RESET gpio3_16 */
        { 0xFFFFFFFF, 0xFFFFFFFF },
    };
    config_mux(reset_pins);
}




uint8_t _getchar(void) { return UART0Read(); }
void _putchar(uint8_t ch) { UART0Write(ch); }

const char* banner =
"              ____ ______________\n"
"  ______     |    |   \\__    ___/\n"
"  \\  __ \\    |    |   | |    |\n"
"  |  |_) )\\__|    |   | |    |\n"
"  |   __/\\________|___| |____|\n"
"  |__|\n\n";

const char* menu =
"\nMENU\n----\nTESTS:\n"
" 1. Quick-test DDR memory\n"
" 2. Dump first 4K of SPI Flash\n"
" 3. Quick-test SPI flash\n"
" 4. Test GPMC\n"
" 5. Test printf\n"
" 6. Scan I2C Bus\n"
" 7. Run Native BogoMIPS test\n"
" 8. Run PJIT BogoMIPS test\n"
" 9. Scan and verify GreenPAK\n"
"SETUP:\n"
" J. Jump to PJIT\n"
" C. Set E Clock Divider\n"
" G. Manage GreenPAK\n"
" E. Erase SPI flash\n"
" P. Program SPI flash\n"
" H. Print help (this)\n"
" X. Reboot\n";

static void JumpPJIT(void) {
    asm __volatile("setend be");
    ((void(*)(void))0x80000000)();
}

static void InitI2C(int bps) {
    static const pin_muxing_t i2c_pins[] = {
        { CONF_I2C0_SCL, (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M0) },
        { CONF_I2C0_SDA, (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M0) },
        { 0xFFFFFFFF, 0xFFFFFFFF },
    };
    I2C0Init(bps);
    config_mux(i2c_pins);
}

static void EmergencyErase(void) {
    int i;
    int esc = 0;
    for(i=0; i<10; i++) {
        WaitMSDMTimer(3);
        if(UART0Getkey() == 0x1B) esc++;
    }
    if(esc >= 1) {
        printf("\n\n!!! EMERGENCY ERASE !!!");
        EraseSPI(0, ERASE_ALL);
        Reset();
    }
}

#define debugSessionDAP 0
static uint32_t printRegisterValue(int port, const char *text, uint32_t addr) {
    uint32_t a = *(uint32_t*)addr;
    printf("%08x %s\n", a, text);
    return a;
}

static void DebugBoot(void) {
    uint32_t reg_val;
    uint32_t boot_sequence;
    int i;
    int esc = 0;

    for(i=0; i<5; i++) {
        WaitMSDMTimer(3);
        if(UART0Getkey() == 0x20) esc++;
    }
    // if(esc == 0) return;

    // CONTROL: device_id
    reg_val = printRegisterValue(debugSessionDAP, "CONTROL: device_id", 0x44E10600);
    if ( (reg_val & 0x0FFFFFFF) == 0xb94402e ) {printf("  * AM335x family\n");}
    if ( (reg_val & 0xF0000000) == (0 << 28) ) {printf("  * Silicon Revision 1.0\n");}
    if ( (reg_val & 0xF0000000) == (1 << 28) ) {printf("  * Silicon Revision 2.0\n");}
    if ( (reg_val & 0xF0000000) == (2 << 28) ) {printf("  * Silicon Revision 2.1\n");}

    // ROM: PRM_RSTST
    printf("\n");
    reg_val = printRegisterValue(debugSessionDAP, "PRM_DEVICE: PRM_RSTST", 0x44E00F08);
    if (reg_val & 1<<0  ) {printf("  * Bit 0 : GLOBAL_COLD_RST\n");}
    if (reg_val & 1<<1  ) {printf("  * Bit 1 : GLOBAL_WARM_RST\n");}
    if (reg_val & 1<<4  ) {printf("  * Bit 4 : WDT1_RST\n");}
    if (reg_val & 1<<5  ) {printf("  * Bit 5 : EXTERNAL_WARM_RST\n");}

    // CONTROL: control_status
    printf("\n");
    reg_val = printRegisterValue(debugSessionDAP, "CONTROL: control_status", 0x44E10040);
    boot_sequence = (reg_val & 0x1F);
    if ( (reg_val & 3<<22 ) == 0<<22 ) {printf("  * SYSBOOT[15:14] = 00b (19.2 MHz)\n");}
    if ( (reg_val & 3<<22 ) == 1<<22 ) {printf("  * SYSBOOT[15:14] = 01b (24 MHz)\n");}
    if ( (reg_val & 3<<22 ) == 2<<22 ) {printf("  * SYSBOOT[15:14] = 10b (25 MHz)\n");}
    if ( (reg_val & 3<<22 ) == 3<<22 ) {printf("  * SYSBOOT[15:14] = 11b (26 MHz)\n");}
    if ( (reg_val & 3<<20 ) != 0<<20 ) {printf("  * SYSBOOT[13:12] have been set improperly!\n");}
    if ( (reg_val & 3<<18 ) == 0<<18 ) {printf("  * SYSBOOT[11:10] = 00b No GPMC CS0 addr/data muxing\n");}
    if ( (reg_val & 3<<18 ) == 1<<18 ) {printf("  * SYSBOOT[11:10] = 01b GPMC CS0 addr/addr/data muxing\n");}
    if ( (reg_val & 3<<18 ) == 2<<18 ) {printf("  * SYSBOOT[11:10] = 10b GPMC CS0 addr/data muxing\n");}
    if ( (reg_val & 3<<18 ) == 3<<18 ) {printf("  * SYSBOOT[11:10] = 11b ILLEGAL VALUE!\n");}
    if ( (reg_val & 1<<17 ) == 0<<17 ) {printf("  * SYSBOOT[9] = 0 GPMC CS0 Ignore WAIT input\n");}
    if ( (reg_val & 1<<17 ) == 1<<17 ) {printf("  * SYSBOOT[9] = 1 GPMC CS0 Use WAIT input\n");}
    if ( (reg_val & 1<<16 ) == 0<<16 ) {printf("  * SYSBOOT[8] = 0 GPMC CS0 8-bit data bus\n");}
    if ( (reg_val & 1<<16 ) == 1<<16 ) {printf("  * SYSBOOT[8] = 1 GPMC CS0 16-bit data bus\n");}
    if ( (reg_val & 7<<8  ) == 3<<8  ) {printf("  * Device Type = General Purpose (GP)\n");}
    else                               {printf("  * Device Type is NOT GP\n");}
    if ( (reg_val & 0xFF  ) == 0x01  ) {printf("  * SYSBOOT[8] = 1 GPMC CS0 16-bit data bus\n");}
    if ( (reg_val & 3<<6  ) == 0<<6  ) {printf("  * SYSBOOT[7:6] = 00b MII (EMAC boot modes only)\n");}
    if ( (reg_val & 3<<6  ) == 1<<6  ) {printf("  * SYSBOOT[7:6] = 01b RMII (EMAC boot modes only)\n");}
    if ( (reg_val & 3<<6  ) == 2<<6  ) {printf("  * SYSBOOT[7:6] = 10b ILLEGAL VALUE!\n");}
    if ( (reg_val & 3<<6  ) == 3<<6  ) {printf("  * SYSBOOT[7:6] = 11b RGMII no internal delay (EMAC boot modes only)\n");}
    if ( (reg_val & 1<<5  ) == 0<<5  ) {printf("  * SYSBOOT[5] = 0 CLKOUT1 disabled\n");}
    if ( (reg_val & 1<<5  ) == 1<<5  ) {printf("  * SYSBOOT[5] = 1 CLKOUT1 enabled\n");}

    if (boot_sequence == 0x01)      {printf("  * Boot Sequence : UART0 -> XIP w/WAIT (MUX2) -> MMC0 -> SPI0\n");} 
    else if (boot_sequence == 0x02) {printf("  * Boot Sequence : UART0 -> SPI0 -> NAND -> NANDI2C\n");}
    else if (boot_sequence == 0x03) {printf("  * Boot Sequence : UART0 -> SPI0 -> XIP (MUX2) -> MMC0\n");}
    else if (boot_sequence == 0x04) {printf("  * Boot Sequence : UART0 -> XIP w/WAIT (MUX1) -> MMC0 -> NAND\n");}
    else if (boot_sequence == 0x05) {printf("  * Boot Sequence : UART0 -> XIP (MUX1) -> SPI0 -> NANDI2C\n");}
    else if (boot_sequence == 0x06) {printf("  * Boot Sequence : EMAC1 -> SPI0 -> NAND -> NANDI2C\n");}
    else if (boot_sequence == 0x07) {printf("  * Boot Sequence : EMAC1 -> MMC0 -> XIP w/WAIT (MUX2) -> NAND\n");}
    else if (boot_sequence == 0x08) {printf("  * Boot Sequence : EMAC1 -> MMC0 -> XIP (MUX2) -> NANDI2C\n");}
    else if (boot_sequence == 0x09) {printf("  * Boot Sequence : EMAC1 -> XIP w/WAIT (MUX1) -> NAND -> MMC0\n");}
    else if (boot_sequence == 0x0A) {printf("  * Boot Sequence : EMAC1 -> XIP (MUX1) -> SPI0 -> NANDI2C\n");}
    else if (boot_sequence == 0x0B) {printf("  * Boot Sequence : USB0 -> NAND -> SPI0 -> MMC0\n");}
    else if (boot_sequence == 0x0C) {printf("  * Boot Sequence : USB0 -> NAND -> XIP (MUX2) -> NANDI2C\n");}
    else if (boot_sequence == 0x0D) {printf("  * Boot Sequence : USB0 -> NAND -> XIP (MUX1) -> SPI0\n");}
    else if (boot_sequence == 0x0F) {printf("  * Boot Sequence : UART0 -> EMAC1 -> Reserved -> Reserved\n");}
    else if (boot_sequence == 0x10) {printf("  * Boot Sequence : XIP (MUX1) -> UART0 -> EMAC1 -> MMC0 \n");}
    else if (boot_sequence == 0x11) {printf("  * Boot Sequence : XIP w/WAIT (MUX1) -> UART0 -> EMAC1 -> MMC0\n");} 
    else if (boot_sequence == 0x12) {printf("  * Boot Sequence : NAND -> NANDI2C -> USB0 -> UART0\n");}
    else if (boot_sequence == 0x13) {printf("  * Boot Sequence : NAND -> NANDI2C -> MMC0 -> UART0\n");}
    else if (boot_sequence == 0x14) {printf("  * Boot Sequence : NAND -> NANDI2C -> SPI0 -> EMAC1\n");}
    else if (boot_sequence == 0x15) {printf("  * Boot Sequence : NANDI2C -> MMC0 -> EMAC1 -> UART0\n");}
    else if (boot_sequence == 0x16) {printf("  * Boot Sequence : SPI0 -> MMC0 -> UART0 -> EMAC1\n");}
    else if (boot_sequence == 0x17) {printf("  * Boot Sequence : MMC0 -> SPI0 -> UART0 -> USB0\n");}
    else if (boot_sequence == 0x18) {printf("  * Boot Sequence : SPI0 -> MMC0 -> USB0 -> UART0\n");}
    else if (boot_sequence == 0x19) {printf("  * Boot Sequence : SPI0 -> MMC0 -> EMAC1 -> UART0\n");}
    else if (boot_sequence == 0x1A) {printf("  * Boot Sequence : XIP (MUX2) -> UART0 -> SPI0 -> MMC0\n");}
    else if (boot_sequence == 0x1B) {printf("  * Boot Sequence : XIP w/WAIT (MUX2) -> UART0 -> SPI0 -> MMC0\n");}
    else if (boot_sequence == 0x1C) {printf("  * Boot Sequence : MMC1 -> MMC0 -> UART0 -> USB0\n");}
    else if (boot_sequence == 0x1F) {printf("  * Boot Sequence : Fast External Boot -> EMAC1 -> UART0 -> Reserved\n");}
    else                            {printf("  * ILLEGAL BOOT SEQUENCE!\n");}

    // ROM: Tracing Vector 1
    printf("\n");
    reg_val = printRegisterValue(debugSessionDAP, "ROM: Current tracing vector, word 1", 0x4030CE40);
    if (reg_val & 1<<0  ) {printf("  * Bit 0  : [General] Passed the public reset vector\n");}
    if (reg_val & 1<<1  ) {printf("  * Bit 1  : [General] Entered main function\n");}
    if (reg_val & 1<<2  ) {printf("  * Bit 2  : [General] Running after the cold reset\n");}
    if (reg_val & 1<<3  ) {printf("  * Bit 3  : [Boot] Main booting routine entered\n");}
    if (reg_val & 1<<4  ) {printf("  * Bit 4  : [Memory Boot] Memory booting started\n");}
    if (reg_val & 1<<5  ) {printf("  * Bit 5  : [Peripheral Boot] Peripheral booting started\n");}
    if (reg_val & 1<<6  ) {printf("  * Bit 6  : [Boot] Booting loop reached last device\n");}
    if (reg_val & 1<<7  ) {printf("  * Bit 7  : [Boot] GP header found\n");}
    // if (reg_val & 1<<8  ) {printf("  * Bit 8  : [Boot] Reserved\n");}
    // if (reg_val & 1<<9  ) {printf("  * Bit 9  : [Boot] Reserved\n");}
    // if (reg_val & 1<<10 ) {printf("  * Bit 10 : [Peripheral Boot] Reserved\n");}
    // if (reg_val & 1<<11 ) {printf("  * Bit 11 : [Peripheral Boot] Reserved\n");}
    if (reg_val & 1<<12 ) {printf("  * Bit 12 : [Peripheral Boot] Device initialized\n");}
    if (reg_val & 1<<13 ) {printf("  * Bit 13 : [Peripheral Boot] ASIC ID sent\n");}
    if (reg_val & 1<<14 ) {printf("  * Bit 14 : [Peripheral Boot] Image received\n");}
    if (reg_val & 1<<15 ) {printf("  * Bit 15 : [Peripheral Boot] Peripheral booting failed\n");}
    if (reg_val & 1<<16 ) {printf("  * Bit 16 : [Peripheral Boot] Booting Message not received (timeout)\n");}
    if (reg_val & 1<<17 ) {printf("  * Bit 17 : [Peripheral Boot] Image size not received (timeout)\n");}
    if (reg_val & 1<<18 ) {printf("  * Bit 18 : [Peripheral Boot] Image not received (timeout)\n");}
    // if (reg_val & 1<<19 ) {printf("  * Bit 19 : Reserved\n");}
    if (reg_val & 1<<20 ) {printf("  * Bit 20 : [Configuration Header] CHSETTINGS found\n");}
    if (reg_val & 1<<21 ) {printf("  * Bit 21 : [Configuration Header] CHSETTINGS executed\n");}
    if (reg_val & 1<<22 ) {printf("  * Bit 22 : [Configuration Header] CHRAM executed\n");}
    if (reg_val & 1<<23 ) {printf("  * Bit 23 : [Configuration Header] CHFLASH executed\n");}
    if (reg_val & 1<<24 ) {printf("  * Bit 24 : [Configuration Header] CHMMCSD clocks executed\n");}
    if (reg_val & 1<<25 ) {printf("  * Bit 25 : [Configuration Header] CHMMCSD bus width executed\n");}
    // if (reg_val & 1<<26 ) {printf("  * Bit 26 : Reserved\n");}
    // if (reg_val & 1<<27 ) {printf("  * Bit 27 : Reserved\n");}
    // if (reg_val & 1<<28 ) {printf("  * Bit 28 : Reserved\n");}
    // if (reg_val & 1<<29 ) {printf("  * Bit 29 : Reserved\n");}
    // if (reg_val & 1<<30 ) {printf("  * Bit 30 : Reserved\n");}
    // if (reg_val & 1<<31 ) {printf("  * Bit 31 : Reserved\n");}

    // ROM: Tracing Vector 2
    printf("\n");
    reg_val = printRegisterValue(debugSessionDAP, "ROM: Current tracing vector, word 2", 0x4030CE44);
    // if (reg_val & 1<<0  ) {printf("  * Bit 0  : [Companion chip] Reserved\n");}
    // if (reg_val & 1<<1  ) {printf("  * Bit 1  : [Companion chip] Reserved\n");}
    // if (reg_val & 1<<2  ) {printf("  * Bit 2  : [Companion chip] Reserved\n");}
    // if (reg_val & 1<<3  ) {printf("  * Bit 3  : [Companion chip] Reserved\n");}
    if (reg_val & 1<<4  ) {printf("  * Bit 4  : [USB] USB connect\n");}
    if (reg_val & 1<<5  ) {printf("  * Bit 5  : [USB] USB configured state\n");}
    if (reg_val & 1<<6  ) {printf("  * Bit 6  : [USB] USB VBUS valid\n");}
    if (reg_val & 1<<7  ) {printf("  * Bit 7  : [USB] USB session valid\n");}
    // if (reg_val & 1<<8  ) {printf("  * Bit 8  : Reserved\n");}
    // if (reg_val & 1<<9  ) {printf("  * Bit 9  : Reserved\n");}
    // if (reg_val & 1<<10 ) {printf("  * Bit 10 : Reserved\n");}
    // if (reg_val & 1<<11 ) {printf("  * Bit 11 : Reserved\n");}
    if (reg_val & 1<<12 ) {printf("  * Bit 12 : [Memory Boot] Memory booting trial 0\n");}
    if (reg_val & 1<<13 ) {printf("  * Bit 13 : [Memory Boot] Memory booting trial 1\n");}
    if (reg_val & 1<<14 ) {printf("  * Bit 14 : [Memory Boot] Memory booting trial 2\n");}
    if (reg_val & 1<<15 ) {printf("  * Bit 15 : [Memory Boot] Memory booting trial 3\n");}
    if (reg_val & 1<<16 ) {printf("  * Bit 16 : [Memory Boot] Execute GP image\n");}
    if (reg_val & 1<<17 ) {printf("  * Bit 17 : [Peripheral Boot] Start authentication of peripheral boot image\n");}
    if (reg_val & 1<<18 ) {printf("  * Bit 18 : [Memory & Peripheral Boot] Jumping to Initial SW\n");}
    // if (reg_val & 1<<19 ) {printf("  * Bit 19 : [Memory & Peripheral Boot] Image authentication failed\n");}
    if (reg_val & 1<<20 ) {printf("  * Bit 20 : [Memory & Peripheral Boot] Start image authentication\n");}
    if (reg_val & 1<<21 ) {printf("  * Bit 21 : [Memory & Peripheral Boot] Image authentication failed\n");}
    if (reg_val & 1<<22 ) {printf("  * Bit 22 : [Memory & Peripheral Boot] Analyzing SpeedUp\n");}
    if (reg_val & 1<<23 ) {printf("  * Bit 23 : [Memory & Peripheral Boot] Speedup failed\n");}
    // if (reg_val & 1<<24 ) {printf("  * Bit 24 : [Memory & Peripheral Boot] Reserved\n");}
    // if (reg_val & 1<<25 ) {printf("  * Bit 25 : [Memory & Peripheral Boot] Reserved\n");}
    // if (reg_val & 1<<26 ) {printf("  * Bit 26 : [Memory & Peripheral Boot] Reserved\n");}
    // if (reg_val & 1<<27 ) {printf("  * Bit 27 : [Memory & Peripheral Boot] Reserved\n");}
    if (reg_val & 1<<28 ) {printf("  * Bit 28 : [Memory & Peripheral Boot] Authentication procedure failed\n");}
    // if (reg_val & 1<<29 ) {printf("  * Bit 29 : Reserved\n");}
    // if (reg_val & 1<<30 ) {printf("  * Bit 30 : Reserved\n");}
    // if (reg_val & 1<<31 ) {printf("  * Bit 31 : Reserved\n");}


    // ROM: Tracing Vector 3
    printf("\n");
    reg_val = printRegisterValue(debugSessionDAP, "ROM: Current tracing vector, word 3", 0x4030CE48);
    if (reg_val & 1<<0  ) {printf("  * Bit 0  : [Memory Boot] Memory booting device NULL\n");}
    if (reg_val & 1<<1  ) {printf("  * Bit 1  : [Memory Boot] Memory booting device XIP\n");}
    if (reg_val & 1<<2  ) {printf("  * Bit 2  : [Memory Boot] Memory booting device XIPWAIT\n");}
    if (reg_val & 1<<3  ) {printf("  * Bit 3  : [Memory Boot] Memory booting device NAND\n");}
    // if (reg_val & 1<<4  ) {printf("  * Bit 4  : [Memory Boot] Memory booting device XIPWAIT (MUX1)\n");}
    if (reg_val & 1<<5  ) {printf("  * Bit 5  : [Memory Boot] Memory booting device MMCSD0\n");}
    // if (reg_val & 1<<6  ) {printf("  * Bit 6  : Reserved\n");}
    if (reg_val & 1<<7  ) {printf("  * Bit 7  : [Memory Boot] Memory booting device MMCSD1\n");}
    // if (reg_val & 1<<8  ) {printf("  * Bit 8  : Reserved\n");}
    // if (reg_val & 1<<9  ) {printf("  * Bit 9  : Reserved\n");}
    // if (reg_val & 1<<10 ) {printf("  * Bit 10 : [Memory Boot] Memory booting device NAND_I2C\n");}
    // if (reg_val & 1<<11 ) {printf("  * Bit 11 : Reserved\n");}
    if (reg_val & 1<<12 ) {printf("  * Bit 12 : Memory booting device SPI\n");}
    // if (reg_val & 1<<13 ) {printf("  * Bit 13 : Reserved\n");}
    // if (reg_val & 1<<14 ) {printf("  * Bit 14 : Reserved\n");}
    // if (reg_val & 1<<15 ) {printf("  * Bit 15 : Reserved\n");}
    if (reg_val & 1<<16 ) {printf("  * Bit 16 : Peripheral booting device UART0\n");}
    // if (reg_val & 1<<17 ) {printf("  * Bit 17 : Reserved\n");}
    // if (reg_val & 1<<18 ) {printf("  * Bit 18 : Reserved\n");}
    // if (reg_val & 1<<19 ) {printf("  * Bit 19 : Reserved\n");}
    if (reg_val & 1<<20 ) {printf("  * Bit 20 : [Peripheral Boot] Peripheral booting device USB\n");}
    // if (reg_val & 1<<21 ) {printf("  * Bit 21 : Reserved\n");}
    if (reg_val & 1<<22 ) {printf("  * Bit 22 : [Peripheral Boot] Peripheral booting device GPGMAC0\n");}
    // if (reg_val & 1<<23 ) {printf("  * Bit 23 : Reserved\n");}
    if (reg_val & 1<<24 ) {printf("  * Bit 24 : Peripheral booting device NULL\n");}
    // if (reg_val & 1<<25 ) {printf("  * Bit 25 : Reserved\n");}
    // if (reg_val & 1<<26 ) {printf("  * Bit 26 : Reserved\n");}
    // if (reg_val & 1<<27 ) {printf("  * Bit 27 : Reserved\n");}
    // if (reg_val & 1<<28 ) {printf("  * Bit 28 : Reserved\n");}
    // if (reg_val & 1<<29 ) {printf("  * Bit 29 : Reserved\n");}
    // if (reg_val & 1<<30 ) {printf("  * Bit 30 : Reserved\n");}
    // if (reg_val & 1<<31 ) {printf("  * Bit 31 : Reserved\n");}

    // ROM: Copy of PRM_RSTST
    printf("\n");
    reg_val = printRegisterValue(debugSessionDAP, "ROM: Current copy of PRM_RSTST", 0x4030CE4C);
    if (reg_val & 1<<0  ) {printf("  * Bit 0 : GLOBAL_COLD_RST\n");}
    if (reg_val & 1<<1  ) {printf("  * Bit 1 : GLOBAL_WARM_RST\n");}
    if (reg_val & 1<<4  ) {printf("  * Bit 4 : WDT1_RST\n");}
    if (reg_val & 1<<5  ) {printf("  * Bit 5 : EXTERNAL_WARM_RST\n");}

    // ROM: Cold Reset Tracing Vector 1
    printf("\n");
    reg_val = printRegisterValue(debugSessionDAP, "ROM: Cold reset tracing vector, word 1", 0x4030CE50);
    if (reg_val & 1<<0  ) {printf("  * Bit 0  : [General] Passed the public reset vector\n");}
    if (reg_val & 1<<1  ) {printf("  * Bit 1  : [General] Entered main function\n");}
    if (reg_val & 1<<2  ) {printf("  * Bit 2  : [General] Running after the cold reset\n");}
    if (reg_val & 1<<3  ) {printf("  * Bit 3  : [Boot] Main booting routine entered\n");}
    if (reg_val & 1<<4  ) {printf("  * Bit 4  : [Memory Boot] Memory booting started\n");}
    if (reg_val & 1<<5  ) {printf("  * Bit 5  : [Peripheral Boot] Peripheral booting started\n");}
    if (reg_val & 1<<6  ) {printf("  * Bit 6  : [Boot] Booting loop reached last device\n");}
    if (reg_val & 1<<7  ) {printf("  * Bit 7  : [Boot] GP header found\n");}
    // if (reg_val & 1<<8  ) {printf("  * Bit 8  : [Boot] Reserved\n");}
    // if (reg_val & 1<<9  ) {printf("  * Bit 9  : [Boot] Reserved\n");}
    // if (reg_val & 1<<10 ) {printf("  * Bit 10 : [Peripheral Boot] Reserved\n");}
    // if (reg_val & 1<<11 ) {printf("  * Bit 11 : [Peripheral Boot] Reserved\n");}
    if (reg_val & 1<<12 ) {printf("  * Bit 12 : [Peripheral Boot] Device initialized\n");}
    if (reg_val & 1<<13 ) {printf("  * Bit 13 : [Peripheral Boot] ASIC ID sent\n");}
    if (reg_val & 1<<14 ) {printf("  * Bit 14 : [Peripheral Boot] Image received\n");}
    if (reg_val & 1<<15 ) {printf("  * Bit 15 : [Peripheral Boot] Peripheral booting failed\n");}
    if (reg_val & 1<<16 ) {printf("  * Bit 16 : [Peripheral Boot] Booting Message not received (timeout)\n");}
    if (reg_val & 1<<17 ) {printf("  * Bit 17 : [Peripheral Boot] Image size not received (timeout)\n");}
    if (reg_val & 1<<18 ) {printf("  * Bit 18 : [Peripheral Boot] Image not received (timeout)\n");}
    // if (reg_val & 1<<19 ) {printf("  * Bit 19 : Reserved\n");}
    if (reg_val & 1<<20 ) {printf("  * Bit 20 : [Configuration Header] CHSETTINGS found\n");}
    if (reg_val & 1<<21 ) {printf("  * Bit 21 : [Configuration Header] CHSETTINGS executed\n");}
    if (reg_val & 1<<22 ) {printf("  * Bit 22 : [Configuration Header] CHRAM executed\n");}
    if (reg_val & 1<<23 ) {printf("  * Bit 23 : [Configuration Header] CHFLASH executed\n");}
    if (reg_val & 1<<24 ) {printf("  * Bit 24 : [Configuration Header] CHMMCSD clocks executed\n");}
    if (reg_val & 1<<25 ) {printf("  * Bit 25 : [Configuration Header] CHMMCSD bus width executed\n");}
    // if (reg_val & 1<<26 ) {printf("  * Bit 26 : Reserved\n");}
    // if (reg_val & 1<<27 ) {printf("  * Bit 27 : Reserved\n");}
    // if (reg_val & 1<<28 ) {printf("  * Bit 28 : Reserved\n");}
    // if (reg_val & 1<<29 ) {printf("  * Bit 29 : Reserved\n");}
    // if (reg_val & 1<<30 ) {printf("  * Bit 30 : Reserved\n");}
    // if (reg_val & 1<<31 ) {printf("  * Bit 31 : Reserved\n");}

    // ROM: Cold Reset Tracing Vector 2
    printf("\n");
    reg_val = printRegisterValue(debugSessionDAP, "ROM: Cold reset tracing vector, word 2", 0x4030CE54);
    // if (reg_val & 1<<0  ) {printf("  * Bit 0  : [Companion chip] Reserved\n");}
    // if (reg_val & 1<<1  ) {printf("  * Bit 1  : [Companion chip] Reserved\n");}
    // if (reg_val & 1<<2  ) {printf("  * Bit 2  : [Companion chip] Reserved\n");}
    // if (reg_val & 1<<3  ) {printf("  * Bit 3  : [Companion chip] Reserved\n");}
    if (reg_val & 1<<4  ) {printf("  * Bit 4  : [USB] USB connect\n");}
    if (reg_val & 1<<5  ) {printf("  * Bit 5  : [USB] USB configured state\n");}
    if (reg_val & 1<<6  ) {printf("  * Bit 6  : [USB] USB VBUS valid\n");}
    if (reg_val & 1<<7  ) {printf("  * Bit 7  : [USB] USB session valid\n");}
    // if (reg_val & 1<<8  ) {printf("  * Bit 8  : Reserved\n");}
    // if (reg_val & 1<<9  ) {printf("  * Bit 9  : Reserved\n");}
    // if (reg_val & 1<<10 ) {printf("  * Bit 10 : Reserved\n");}
    // if (reg_val & 1<<11 ) {printf("  * Bit 11 : Reserved\n");}
    if (reg_val & 1<<12 ) {printf("  * Bit 12 : [Memory Boot] Memory booting trial 0\n");}
    if (reg_val & 1<<13 ) {printf("  * Bit 13 : [Memory Boot] Memory booting trial 1\n");}
    if (reg_val & 1<<14 ) {printf("  * Bit 14 : [Memory Boot] Memory booting trial 2\n");}
    if (reg_val & 1<<15 ) {printf("  * Bit 15 : [Memory Boot] Memory booting trial 3\n");}
    if (reg_val & 1<<16 ) {printf("  * Bit 16 : [Memory Boot] Execute GP image\n");}
    if (reg_val & 1<<17 ) {printf("  * Bit 17 : [Peripheral Boot] Start authentication of peripheral boot image\n");}
    if (reg_val & 1<<18 ) {printf("  * Bit 18 : [Memory & Peripheral Boot] Jumping to Initial SW\n");}
    if (reg_val & 1<<19 ) {printf("  * Bit 19 : [Memory & Peripheral Boot] Reserved\n");}
    if (reg_val & 1<<20 ) {printf("  * Bit 20 : [Memory & Peripheral Boot] Start image authentication\n");}
    if (reg_val & 1<<21 ) {printf("  * Bit 21 : [Memory & Peripheral Boot] Image authentication failed\n");}
    if (reg_val & 1<<22 ) {printf("  * Bit 22 : [Memory & Peripheral Boot] Analyzing SpeedUp\n");}
    if (reg_val & 1<<23 ) {printf("  * Bit 23 : [Memory & Peripheral Boot] SpeedUp failed\n");}
    // if (reg_val & 1<<24 ) {printf("  * Bit 24 : [Memory & Peripheral Boot] Reserved\n");}
    // if (reg_val & 1<<25 ) {printf("  * Bit 25 : [Memory & Peripheral Boot] Reserved\n");}
    // if (reg_val & 1<<26 ) {printf("  * Bit 26 : [Memory & Peripheral Boot] Reserved\n");}
    // if (reg_val & 1<<27 ) {printf("  * Bit 27 : [Memory & Peripheral Boot] Reserved\n");}
    if (reg_val & 1<<28 ) {printf("  * Bit 28 : [Memory & Peripheral Boot] Authentication procedure failed\n");}
    // if (reg_val & 1<<29 ) {printf("  * Bit 29 : Reserved\n");}
    // if (reg_val & 1<<30 ) {printf("  * Bit 30 : Reserved\n");}
    // if (reg_val & 1<<31 ) {printf("  * Bit 31 : Reserved\n");}

    // ROM: Cold Reset Tracing Vector 3
    printf("\n");
    reg_val = printRegisterValue(debugSessionDAP, "ROM: Cold reset tracing vector, word 3", 0x4030CE58);
    if (reg_val & 1<<0  ) {printf("  * Bit 0  : [Memory Boot] Memory booting device NULL\n");}
    if (reg_val & 1<<1  ) {printf("  * Bit 1  : [Memory Boot] Memory booting device XIP\n");}
    if (reg_val & 1<<2  ) {printf("  * Bit 2  : [Memory Boot] Memory booting device XIPWAIT\n");}
    if (reg_val & 1<<3  ) {printf("  * Bit 3  : [Memory Boot] Memory booting device NAND\n");}
    // if (reg_val & 1<<4  ) {printf("  * Bit 4  : [Memory Boot] Reserved\n");}
    if (reg_val & 1<<5  ) {printf("  * Bit 5  : [Memory Boot] Memory booting device MMCSD0\n");}
    // if (reg_val & 1<<6  ) {printf("  * Bit 6  : Reserved\n");}
    if (reg_val & 1<<7  ) {printf("  * Bit 7  : [Memory Boot] Memory booting device MMCSD1\n");}
    // if (reg_val & 1<<8  ) {printf("  * Bit 8  : Reserved\n");}
    // if (reg_val & 1<<9  ) {printf("  * Bit 9  : Reserved\n");}
    // if (reg_val & 1<<10 ) {printf("  * Bit 10 : [Memory Boot] Reserved\n");}
    // if (reg_val & 1<<11 ) {printf("  * Bit 11 : Reserved\n");}
    if (reg_val & 1<<12 ) {printf("  * Bit 12 : Memory booting device SPI\n");}
    // if (reg_val & 1<<13 ) {printf("  * Bit 13 : Reserved\n");}
    // if (reg_val & 1<<14 ) {printf("  * Bit 14 : Reserved\n");}
    // if (reg_val & 1<<15 ) {printf("  * Bit 15 : Reserved\n");}
    if (reg_val & 1<<16 ) {printf("  * Bit 16 : Peripheral booting device UART0\n");}
    // if (reg_val & 1<<17 ) {printf("  * Bit 17 : Reserved\n");}
    if (reg_val & 1<<18 ) {printf("  * Bit 18 : [Peripheral Boot] Reserved\n");}
    // if (reg_val & 1<<19 ) {printf("  * Bit 19 : Reserved\n");}
    if (reg_val & 1<<20 ) {printf("  * Bit 20 : [Peripheral Boot] Peripheral booting device USB\n");}
    // if (reg_val & 1<<21 ) {printf("  * Bit 21 : [Peripheral Boot] Reserved\n");}
    if (reg_val & 1<<22 ) {printf("  * Bit 22 : [Peripheral Boot] Peripheral booting device GPGMAC0\n");}
    // if (reg_val & 1<<23 ) {printf("  * Bit 23 : Reserved\n");}
    if (reg_val & 1<<24 ) {printf("  * Bit 24 : Peripheral booting device NULL\n");}
    // if (reg_val & 1<<25 ) {printf("  * Bit 25 : Reserved\n");}
    // if (reg_val & 1<<26 ) {printf("  * Bit 26 : Reserved\n");}
    // if (reg_val & 1<<27 ) {printf("  * Bit 27 : Reserved\n");}
    // if (reg_val & 1<<28 ) {printf("  * Bit 28 : Reserved\n");}
    // if (reg_val & 1<<29 ) {printf("  * Bit 29 : Reserved\n");}
    // if (reg_val & 1<<30 ) {printf("  * Bit 30 : Reserved\n");}
    // if (reg_val & 1<<31 ) {printf("  * Bit 31 : Reserved\n");}
}

int main(void) {
    char option[4] = "?";
    double t1, t2;
    int clk1, clk2;

    InitResetHalt();

    InitDMTimer(); // works
    InitCorePLL(1000); // works
    InitPERPLL(); // works
    EnableL3L4Wakeup();
    UART0Init(115200); // works
    InitPRU(); // works

    printf("%c%c%c%c\n\n", NAK, CANCEL, CANCEL, CANCEL); // flush XMODEM
    // DebugBoot();

    printf("%s", banner);

    printf("[BOOT] Build Version, %s, Date %s, Time %s\n", Version, __DATE__, __TIME__);

    SPIInit(24000000); // works
    EmergencyErase();

    InitI2C(100000);

    printf("[I2C0] Scanning bus..\n");
    if ((prom_detected = I2C0Probe(0x50))) {
        printf("[I2C0] 101_0000 ($50) EEPROM Detected\n");
        LoadConfigI2C();
    } else {
        printf("[I2C0] Default settings loaded\n");
        cpu_state.config = default_config;
    }

    if(cpu_state.config.post_enable_checkclk) {
        t1 = ReadDMTimerSeconds();
        clk1 = GetPRUClock();
    }

    if ((gpak_detected = I2C0Probe(0x8))) {
        printf("[I2C0] 000_10xx ($8~$A) GreenPAK Detected\n");
        InitNvm();
        if(cpu_state.config.post_enable_gpack_ok) {
            printf("[I2C0] GreenPAK Protection Bits: $%02X $%02X $%02X\n", 
            gpak_read_reg(0xE0), // Register Read/Write Protection Bits
            gpak_read_reg(0xE1), // NVM Configuration Protection Bits
            gpak_read_reg(0xE4)  // Protection Lock Bit
            );  
            printf("[I2C0] GreenPAK %s\n", ReadChip(0) ? "ok." : "bad!");
        }
    }

    if (pmic_detected = I2C0Probe(0x24)) {
        //cpu_state.config.last_boot_good
        if(InitPower(RAIL_DCDC2, cpu_state.config.pmic_voltage * 0.01)) {
            InitMPUPLL(cpu_state.config.dpll_mul); // works
            printf("[I2C0] Nitro mode enabled\n");
        } else {
            printf("[I2C0] Humble mode enabled\n");
            InitMPUPLL(300); // works
        }

    } else {
            printf("[I2C0] Humble mode enabled\n");
        InitMPUPLL(300); // works
    }
	
    if(!cpu_state.config.cpu_enable_icache) {
        CP15ICacheFlush();
		CP15ICacheDisable();
	}
	
    if(!cpu_state.config.cpu_enable_dcache) {
        CP15DCacheFlush();
        CP15DCacheDisable();
        if(!cpu_state.config.cpu_enable_icache) {
            CP15AuxControlFeatureDisable(0x02);
        }
	}    

    DDRInit(); // works
    if(cpu_state.config.post_enable_long_mem) DDRTest();
    
    InitMMU(); // works
    setecho(1);

    if(cpu_state.config.post_enable_checkclk) {
        WaitMSDMTimer(20);
        t2 = ReadDMTimerSeconds(); 
        clk2 = GetPRUClock(); // this will be in E-clocks, so 1/10th the actual
        double MHz = (clk2 - clk1) * 0.00001 / (t2 - t1);

        // works partially, no CIA, update to real CLK
        if(MHz < 5.0 || MHz > 16.0) {
            printf("[CLK7] Unable to read main bus CLK\n");
            MHz = 8.0;
        } else {
            printf("[CLK7] Main bus clock measured at %0.3f\n", MHz);
        }
        cpu_state.config.kHz = MHz * 1000.0 + 0.5;
        cpu_state.config.is_dirty = 1;
        InitGPMC((float)MHz); 
    } else {
        double MHz = cpu_state.config.kHz * 0.001;
        printf("[CLK7] Main bus clock set to %0.3f\n", MHz);
        InitGPMC((float)MHz); 
    }

	// 3. based on EEPROM settings, initialize PJIT cache and opcode jump tables
	// pjit_cache_init(0xA0000000); // has to come after DDR init, before MMU init
	// emit_opcode_table();

    printf("[BOOT] Image %p ~ %p (%d bytes)\n", &_image_start, &_image_end, (&_image_end - &_image_start));    
    printf("[BOOT] Stack %p ~ %p (%d bytes)\n", &_stack_end, &_stack_top, (&_stack_top - &_stack_end));    
    if(cpu_state.config.is_dirty) SaveConfigI2C(1);
    else MakeGoodI2C();
    ReleaseReset();
    printf("[BOOT] Completed in %0.5f seconds\n", ReadDMTimerSeconds());

    while (1) {
        switch (option[0]) {
            /* TESTS */
        case '1': DDRTest(); break;
        case '2': SPIDump(); break;
        case '3': if (confirm()) SPITest(); break;
        case '4': TestGPMC(); break;
        case '5': test_printf(); break;
        case '6': ProbeI2C(); break;
        case '7': test_native_bogomips(); break;
        case '8': printf("Unimplemented\n"); break;
        case '9': ReadChip(1); break;

            /* SETUP */
        case 'J': case 'j': if (confirm()) JumpPJIT(); break;
        // case 'R': case 'r': if (confirm()) run_mcl68k(0); break;
        case 'C': case 'c': SetEClock(); break;
        case 'G': case 'g': ManageGP(); break;
        case 'E': case 'e': if (confirm()) EraseSPI(0, ERASE_ALL); break;
        case 'P': case 'p': if (confirm()) WriteImage(&_image_start, &_image_end); break;
        case 'X': case 'x': if (confirm()) Reset(); break;

        default: printf("Unimplemented..\n"); // fallthru
        case 'H': case 'h': case '?': printf("%s", menu); break;
        } // end switch

        printf("Ready\n] ");
        gets(option);

    } // end while

    return 0;
}

// TO-DO:

// - MMU init uses old tables; need to rework
// - menu needs to live in an interrupt, not main()
// - GPIO interrupts are not enabled (BG/BGACK/BR)
// - fixup inlining
// - 24-bit address cleaning
// - 68030 and FPU instructions
// - 68030 extended EA modes
// - 68040 MMU instructions (partial)