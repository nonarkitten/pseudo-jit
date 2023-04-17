/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * This is part of PJIT the Pseudo-JIT 68K emulator.
 *
 * PJIT is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * PJIT is licensed under a Creative Commons
 * Attribution-NonCommercial-ShareAlike 4.0 International License.
 *
 * Under the terms of this license you are free copy and redistribute
 * the material in any medium or format as well as remix, transform,
 * and build upon the material.
 *
 * You must give appropriate credit, provide a link to the license,
 * and indicate if changes were made. You may do so in any reasonable
 * manner, but not in any way that suggests the licensor endorses you
 * or your use.
 *
 * You may not use the material for commercial purposes.
 *
 * If you remix, transform, or build upon the material, you must
 * distribute your contributions under the same license as the original.
 *
 * You may not apply legal terms or technological measures that legally
 * restrict others from doing anything the license permits.
 *
 * Portions of PJIT have been derived from the following:
 *
 *     Castaway (formerly FAST), GPL version 2 License
 *     Copyright (c) 1994-2002 Martin Döring, Joachim Hönig
 *    
 *     Cyclone 68K, GPL version 2 License
 *     Copyright (c) 2004,2011 Dave "FinalDave" Haywood
 *     Copyright (c) 2005-2011 Graûvydas "notaz" Ignotas
 *    
 *     TI StarterWare, modified BSD 3-Clause License
 *     Copyright (C) 2010 Texas Instruments Incorporated - http://www.ti.com/
 *
 *     libbbb, Apache License, Version 2.0
 *     Copyright 2015 University of Applied Sciences Western Switzerland / Fribourg
 */

#include "main.h"

#define NAK     0x15
#define CANCEL  0x18

int confirm(void) {
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

static const char* Version = STR(VERSION);

static void InitResetHalt(void) {
    InitGPIO();
    AssertReset();
    static const pin_muxing_t reset_pins[] = {
        { PINMUX_CONF_UART1_RTSN,  (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M7) }, /* HALT gpio0_13 */
        { PINMUX_CONF_MCASP0_AXR0, (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M7) }, /* RESET gpio3_16 */
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
" 6. Run Native BogoMIPS test\n"
" 7. Run PJIT BogoMIPS test\n"
"SETUP:\n"
" J. Jump to PJIT\n"
" C. Set E Clock Divider\n"
" R. Run MCL68K\n"
" S. Manage SPI Flash\n"
" E. Manage EEPROM Config\n"
" G. Manage GreenPAK\n"
" H. Print help (this)\n"
" X. Reboot\n";

static void JumpPJIT(void) {
    asm __volatile("setend be");
    ((void(*)(void))0x80000000)();
}

static void InitI2C(int bps) {
    static const pin_muxing_t i2c_pins[] = {
        { PINMUX_CONF_I2C0_SCL, (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M0) },
        { PINMUX_CONF_I2C0_SDA, (PIN_CFG_INEN | PIN_CFG_PTUP | PIN_CFG_M0) },
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

static void write_LE_word(uint32_t address, uint16_t data) {
    if(address < 0x02000000) *(uint16_t*)address = data;
    else write_BE_word(address, data);
}
static uint16_t read_LE_word(uint32_t address) {
    if(address < 0x02000000) return *(uint16_t*)address;
    else return read_BE_word(address);
}
static void write_LE_byte(uint32_t address, uint8_t data) {
    if(address < 0x02000000) *(uint8_t*)(address ^ 1) = data;
    else write_BE_byte(address, data);
}
static uint8_t read_LE_byte(uint32_t address) {
    if(address < 0x02000000) return *(uint8_t*)(address ^ 1);
    else return read_BE_byte(address);
}

static void startMCL68k(void) {        
    extern void (*WriteWord)(uint32_t address, uint16_t data);
    extern uint16_t (*ReadWord)(uint32_t address);
    extern void (*WriteByte)(uint32_t address, uint8_t data);
    extern uint8_t (*ReadByte)(uint32_t address);       
    extern const int tiny_BASIC_size;   
    extern uint8_t tiny_BASIC[];
    char option[4] = { 0 };

    printf("Run a) tinyBASIC or b) baremetal? ");
    gets(option);
    bool run_tb = (option[0] == 'a' || option[0] == 'A');
    printf("Do you have the Katra (byte swapper) board [y/N]? ");
    gets(option);
    if (option[0] == 'y' || option[0] == 'Y') {
        WriteWord = write_BE_word;
        ReadWord = read_BE_word;
        WriteByte = write_BE_byte;
        ReadByte = read_BE_byte;
    } else {
        WriteWord = write_LE_word;
        ReadWord =  read_LE_word ;
        WriteByte = write_LE_byte;
        ReadByte =  read_LE_byte ;
    }
    if (run_tb) {
        memcpy((void*)0x80000000, tiny_BASIC, tiny_BASIC_size);
        run_mcl68k(0x80000000);
    } else {
        run_mcl68k(0);
    }
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
    double MHz;

    InitResetHalt();
    InitDMTimer();
    InitCorePLL(1000);
    InitPERPLL();
    InitL3L4Wakeup();
    InitUART0(115200);
    InitPRU();
    InitSPI(24000000);

    printf("%c%c%c%c\n\n", NAK, CANCEL, CANCEL, CANCEL);
    EmergencyErase();

    SetEcho(1);

    printf("%s[BOOT] Build Version, %s, Date %s, Time %s\n" , banner , Version, __DATE__, __TIME__);

    InitI2C(100000);

    printf("[I2C0] Scanning bus..\n");
    prom_detected = DetectEEPROM();
    gpak_detected = DetectGP();
    pmic_detected = DetectPMIC();

    if(cpu_state.config.post_enable_checkclk) {
        t1 = ReadDMTimerSeconds();
        clk1 = GetPRUClock();
    }

    printf("[I2C0] %s mode enabled\n", pmic_detected ? "Nitro" : "Humble");
	
    InitDDR();

    if(cpu_state.config.post_enable_long_mem) DDRTest();
    
    if(cpu_state.config.post_enable_checkclk) {
        WaitMSDMTimer(20);
        t2 = ReadDMTimerSeconds(); 
        clk2 = GetPRUClock();
        MHz = (clk2 - clk1) * 0.00001 / (t2 - t1);
        printf("[CLK7] Main bus clock measured at %0.3f\n", MHz);
        cpu_state.config.kHz = MHz * 1000.0 + 0.5;
        cpu_state.config.is_dirty = 1;
    } else {
        MHz = cpu_state.config.kHz * 0.001;
        printf("[CLK7] Main bus clock set to %0.3f\n", MHz);
    }
    if(MHz < 5.0 || MHz > 16.0) {
        printf("[CLK7] Main bus CLK out of spec, using 8MHz\n");
        MHz = 8.0;
        cpu_state.config.kHz = 8000;
        cpu_state.config.is_dirty = 1;
    }

    InitGPMC((float)MHz); 
    InitMMU();

	// 3. based on EEPROM settings, initialize PJIT cache and opcode jump tables
    printf("[BOOT] Initializing cache\n");
	pjit_cache_init(0xA0000000); // has to come after DDR init, before MMU init
    printf("[BOOT] Initializing opcode tables\n");
	// emit_opcode_table();

    printf("[BOOT] Image %p ~ %p (%d bytes)\n", &_image_start, &_image_end, (&_image_end - &_image_start));    
    printf("[BOOT] Stack %p ~ %p (%d bytes)\n", &_stack_end, &_stack_top, (&_stack_top - &_stack_end));    

    if(cpu_state.config.is_dirty) SaveConfigEEPROM(1); else MakeGoodEEPROM();

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
        case '6': test_native_bogomips(); break;
        case '7': printf("Unimplemented\n"); break;

            /* SETUP */
        case 'J': case 'j': if (confirm()) JumpPJIT(); break;
        case 'R': case 'r': startMCL68k(); break;
        case 'C': case 'c': SetEClock(); break;
        case 'G': case 'g': ManageGP(); break;
        case 'E': case 'e': ManageConfig(); break;
        case 'S': case 's': ManageSPI(); break;
        case 'X': case 'x': if (confirm()) Reset(); break;

        default: printf("Unimplemented\n"); // fallthru
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