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
// " R. Run MCL68K\n"
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
    for(i=0; i<5; i++) {
        if(UART0Getkey() == 0x1B) esc++;
        else WaitMSDMTimer(1);
    }
    if(esc >= 1) {
        printf("\n\n!!! EMERGENCY ERASE !!!");
        // EraseSPI(0, ERASE_ALL);
        Reset();
    }
}


/*  JUST PUTTING THIS HERE FOR NOW

    SHOULD MOVE TO GPIO

    pr1_pru0_pru_r31_3   PRU0     C12       B1        BBB_BR				I*	MCASP0_AHCLKR    Device is requesting the bus when low
    pr1_pru0_pru_r30_5   PRU0     C13       B3        BBB_BG				O	MCASP0_FSR       Assert BG when access is complete (how?)
    pr1_pru0_pru_r31_1   PRU0     B13       A2        BBB_BGACK				I*	MCASP0_FSX       
    pr1_pru0_pru_r30_12  PRU0     G17       B15       GPIO_SUPE           		MMC0_CLK         Indicates the CPU processor state (User/Supervisor)
    pr1_pru0_pru_r30_13  PRU0     G18       B16       GPIO_IACK           		MMC0_CMD         Overrides the FCx pins during an IACK
    pr1_pru0_pru_r31_2   PRU0     D12       B2        BBB_RESET				I*	MCASP0_AXR0      Reset is used primarily during boot
*/

static const char* GetSystemName(uint16_t kHz) {
    typedef struct {
        uint16_t kHz;
        const char* name;
    } SystemID_t;

    static const SystemID_t systems[] = {
        { 16050, "Atari AdSPEED"    },
        { 14320, "Amiga AdSPEED"    },
        { 14190, "Amiga AdSPEED"    },
        { 12000, "NEOGEO"           },
        { 10000, "X68000"           },
        {  8050, "Atari STe (NTSC)" },
        {  8030, "Atari (PAL)"      },
        {  8010, "Atari ST (NTSC)"  },
        {  7830, "Apple Mac"        },
        {  7670, "SEGA Genesis"     },
        {  7600, "SEGA Mega Drive"  },
        {  7160, "Amiga (NTSC)"     },
        {  7090, "Amiga (PAL)"      },
        {  5000, "Apple Lisa"       },
        {     0, NULL               }
    }; 

    for(int i=0; i<13; i++) {
        uint16_t mid_kHz = (systems[i].kHz + systems[i + 1].kHz) / 2;
        if(kHz > mid_kHz) return systems[i].name;
    }
    return "Unknown";
}

int main(void) {
    char option[4] = "?";
    double t1 = 0.0, t2 = 0.0;
    int clk1 = 0, clk2 = 0;
    double MHz;

    extern bool printTimeStamp;
    extern volatile double printf_overhead;

    InitResetHalt();
    InitDMTimer();
    InitCorePLL(1000);
    InitPERPLL();
    InitL3L4Wakeup();
    InitUART0(115200);
    InitPRU();
    InitSPI(24000000);
    InitI2C(400000);

    printf_overhead = 0.0;
    printTimeStamp = false;

    printf("%c%c%c%c\n\n", NAK, CANCEL, CANCEL, CANCEL);

    SetEcho(1);

    printf("%s Build Version, %s, Date %s, Time %s\n\n", banner, Version, __DATE__, __TIME__);

    EmergencyErase();
    printTimeStamp = true;

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
        t2 = ReadDMTimerSeconds(); 
        clk2 = GetPRUClock();
        MHz = (clk2 - clk1) * 0.00001 / (t2 - t1);
        if(MHz < 0.001) {
            printf("[BCLK] Main bus unmeasurable (PRU not running?)\n");
        } else {
            printf("[BCLK] Main bus clock measured at %0.3f\n", MHz);
            cpu_state.config.kHz = MHz * 1000.0 + 0.5;
            cpu_state.config.is_dirty = 1;
        }
    } else {
        MHz = cpu_state.config.kHz * 0.001;
        printf("[BCLK] Main bus clock set to %0.3f\n", MHz);
    }
    if(MHz < 5.0 || MHz > 16.0) {
        printf("[BCLK] Main bus CLK out of spec, using 8MHz\n");
        MHz = 8.0;
        cpu_state.config.kHz = 8000;
        cpu_state.config.is_dirty = 1;
    }

    printf("[BCLK] Hello %s!\n", GetSystemName(cpu_state.config.kHz));

    InitGPMC((float)MHz); 
    InitMMU();

	// 3. based on EEPROM settings, initialize PJIT cache and opcode jump tables
    printf("[BOOT] Initializing cache\n");
	pjit_cache_init(0xA0000000); // has to come after DDR init, before MMU init
    printf("[BOOT] Initializing opcode tables\n");
	emit_opcode_table();

    printf("[BOOT] Image %p ~ %p (%d bytes)\n", &_image_start, &_image_end, (&_image_end - &_image_start));    
    printf("[BOOT] Stack %p ~ %p (%d bytes)\n", &_stack_end, &_stack_top, (&_stack_top - &_stack_end));    

    // Load our initial PC and SP
    // cpu = &cpu_state;
    // cpu_state.a7 = *(uint32_t*)0;
    // cpu_state.pc = *(uint32_t*)4;
    // pjit_jmp((uint16_t*)cpu_state.pc);

    if(cpu_state.config.is_dirty) SaveConfigEEPROM(1); else MakeGoodEEPROM();

    if(!cpu_state.config.cpu_enable_icache) {
        CP15ICacheFlush();
		CP15ICacheDisable();
        printf("[BOOT] ICache disabled\n");
	} else {
        printf("[BOOT] ICache enabled\n");
    }
	
    if(!cpu_state.config.cpu_enable_dcache) {
        CP15DCacheFlush();
        CP15DCacheDisable();
        if(!cpu_state.config.cpu_enable_icache) {
            CP15AuxControlFeatureDisable(0x02);
            printf("[BOOT] DCache & L2 Cache disabled\n");
        } else {
            printf("[BOOT] DCache disabled\n");
        }
	} else {
            printf("[BOOT] DCache & L2 Cache enabled\n");
    }

    ReleaseReset();

    double boot_done = ReadDMTimerSeconds() - printf_overhead;

    printf("[BOOT] Printf overhead %0.5f seconds\n", printf_overhead);
    printf("[BOOT] Completed in %0.5f seconds (%s)\n", boot_done, (boot_done < 0.15) ? "okay" : "slow");
    printTimeStamp = false;

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
        // case 'R': case 'r': startMCL68k(); break;
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