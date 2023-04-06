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

    printf("[I2C0] GreenPAK Protection Bits: $%02X $%02X $%02X\n", 
        gpak_read_reg(0xE0), // Register Read/Write Protection Bits
        gpak_read_reg(0xE1), // NVM Configuration Protection Bits
        gpak_read_reg(0xE4)  // Protection Lock Bit
        );    
}

static void ProbeI2C(void) {
    for (int i = 0; i < 120; i++) {
        if ((i & 0x7) == 0) printf("\n%02X:", i);
        if (I2C0Probe(i)) printf(" %02x", i);
        else printf(" --");
    }
    printf("\n");
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
        case '2': printf("[I2C0] GreenPAK %s\n", ReadChip(1) ? "ok." : "bad!"); break;
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
"\n\n              ____ ______________\n"
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

static void InitI2C(void) {
    static const pin_muxing_t i2c_pins[] = {
        { CONF_I2C0_SCL, (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M0) },
        { CONF_I2C0_SDA, (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M0) },
        { 0xFFFFFFFF, 0xFFFFFFFF },
    };
    I2C0Init(400000);
    config_mux(i2c_pins);
}

static void EmergencyErase(void) {
    int i;
    int esc = 0;
    for(i=0; i<10; i++) {
        WaitMSDMTimer(5);
        if(UART0Getkey() == 0x1B) esc++;
    }
    if(esc >= 1) {
        printf("\n\n!!! EMERGENCY ERASE !!!");
        EraseSPI(0, ERASE_ALL);
        Reset();
    }
}

int main(void) {
    char option[4] = "?";
    double t1, t2;
    int clk;

    InitResetHalt();

    InitDMTimer(); // works
    InitCorePLL(1000); // works
    InitPERPLL(); // works
    UART0Init(115200); // works

    printf("%c%c%c%c", NAK, CANCEL, CANCEL, CANCEL); // flush XMODEM
    printf("%s", banner);

    SPIInit(12000000); // works
    EmergencyErase();

    InitPRU(); // works
    t1 = ReadDMTimerSeconds();

    WaitMSDMTimer(10);

    InitI2C();
    printf("[I2C0] Scanning bus..\n");
    if ((prom_detected = I2C0Probe(0x50))) {
        printf("[I2C0] 101_0000 ($50) EEPROM Detected\n");
        // Load settings?
    }

    if ((gpak_detected = I2C0Probe(0x8))) {
        printf("[I2C0] 000_10xx ($8~$A) GreenPAK Detected\n");
        InitNvm();
    }

    if ((pmic_detected = I2C0Probe(0x24))) {
        printf("[I2C0] 010_0100 ($24) PMIC Detected, Nitro mode enabled\n");
        InitPower(RAIL_DCDC2, 1.35); // works
        InitMPUPLL(1000); // works
    } else {
        InitMPUPLL(300); // works
    }

    DDRInit(); // works
    InitMMU(); // works

    setecho(1);

    t2 = ReadDMTimerSeconds(); 
    clk = GetPRUClock(); // this will be in E-clocks, so 1/10th the actual
    double MHz = clk * 0.00001 / (t2 - t1);

    printf("[CLK7] Main bus clock measured at %0.3f\n", MHz);
    // works partially, no CIA, update to real CLK
    if(MHz >= 4.99f && MHz <= 16.0f) InitGPMC((float)MHz); 

    ReleaseReset();

    printf("[BOOT] Build Date %s, Time %s\n", __DATE__, __TIME__);
    printf("[BOOT] Image %p ~ %p (%d bytes)\n", &_image_start, &_image_end, (&_image_end - &_image_start));
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
        case 'R': case 'r': if (confirm()) run_mcl68k(0); break;
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