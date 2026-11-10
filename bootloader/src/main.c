/*
 * Copyright (c) 2020-2023 Renee Cousins, the Buffee Project - http://www.buffee.ca
 *
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */

#include "cp15.h"
#include "main.h"
#include "irq.h"
#include "lzt.h"
#include "flash.h"
#include "store.h"

#define NAK     0x15
#define CANCEL  0x18

static uint32_t hx(const char* s) { uint32_t v = 0; for (; *s; s++) { int d = (*s >= '0' && *s <= '9') ? *s - '0' : (*s >= 'a' && *s <= 'f') ? *s - 'a' + 10 : (*s >= 'A' && *s <= 'F') ? *s - 'A' + 10 : -1; if (d < 0) break; v = v * 16 + d; } return v; }

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
volatile uint32_t* smem = (volatile uint32_t*)0x4A310000;

static void StartPRUMain(void) {
    smem[3] = 8;     /* PRU (GreenPAK design 4): PRU_TERMn is asserted when the E counter is this (6800 cycles): 5-9 read reliably, but a write needs its data and CS to last until E falls, which only 8 gives (7: TOD / timer writes are lost) */
    smem[0] = 10;
    int timeout = 5;
    while ((smem[1] == 0) && timeout--) WaitMSDMTimer(100);
    int e_clock = smem[1];
    if (e_clock == 0) printf("E Clock not set (PRU hung?)");
    else printf("E Clock divisor set to %d\n", e_clock);
}

static void SetEClock(void) {
    char query[4];

    printf("Set E Clock divisor [6-10]: ");
    gets(query);
    uint32_t div = atoi(query);
    if(div >= 6 && div <= 10) {
        PRUFullReset(PRU0);
        WaitMSDMTimer(2);
        smem[0] = div;
        StartPRUMain();
    } else {
        printf("Cancelled\n");
    }
}

/* The PRU measures CLK7 against its own cycle counter (pru/main_gp4.c): window i (5 ms each) leaves the cumulative CLK7 edges in smem[8 + 2i] and PRU cycles (200 MHz) in smem[9 + 2i], smem[6] = windows done.
 * Returns the mean kHz over the first n windows (0 = no clock / not ready); n = 1 is the 5 ms look, 8 the 40 ms measurement. */
#define PRU_WINDOWS 8
static double PRUClockKHz(int n) {
    if (n < 1) n = 1; else if (n > PRU_WINDOWS) n = PRU_WINDOWS;
    for (int t = 0; smem[6] < (uint32_t)n && t < 100; t++) WaitMSDMTimer(1);
    if (smem[6] < (uint32_t)n) return 0.0;
    uint32_t edges = smem[8 + 2 * (n - 1)], cycles = smem[9 + 2 * (n - 1)];
    return cycles ? edges * 200000.0 / cycles : 0.0;
}

/* Spread of the per-window frequency over the 8 windows in ppm (max - min): a steady crystal clock is a few tens, a PLL-driven FPGA clock or a loose connection shows more. */
static int PRUClockSpreadPPM(void) {
    double lo = 1e9, hi = 0.0; uint32_t pe = 0, pc = 0;
    if (smem[6] < PRU_WINDOWS) return -1;
    for (int i = 0; i < PRU_WINDOWS; i++) {
        uint32_t e = smem[8 + 2 * i], c = smem[9 + 2 * i];
        double f = (e - pe) * 200000.0 / (c - pc);
        if (f < lo) lo = f; if (f > hi) hi = f; pe = e; pc = c;
    }
    return (int)((hi - lo) / lo * 1e6);
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
"  \\  __ \\    |    |   |#|    |###\n"
"  |  |_) )\\__|    |   |#|    |#\n"
"  |   __/\\________|___|#|____|#\n"
"  |__|###  #############  #####\n"
"    ###\n\n";

const char* menu =
"\nMENU\n----\nTESTS:\n"
" 1. Quick-test DDR memory\n"
" 2. Dump first 4K of SPI Flash\n"
" 3. Quick-test SPI flash\n"
" 4. Test printf\n"
" 5. Run the BogoMIPS test\n"
"SETUP:\n"
" J. Jump to PJIT\n"
" L. Load PJIT bundle (xmodem) and run it\n"
" R. Run the loaded PJIT   B. Autoboot sequence   F. Stored images (flash)\n U. Update the bootloader in the flash (xmodem a bootloader.bin)\n"
" C. Set E Clock Divider\n"
" S. Manage SPI Flash\n"
" E. Manage EEPROM Config\n"
" H. Print help (this)\n"
" M. Peek memory (hex dump)\n"
" W. Poke a 32-bit word\n"
" X. Reboot\n";

/* The PJIT payload (platform/buffee, make bundle-<test>): an LZTB bundle received over xmodem into DRAM, unpacked to its link addresses at the top of the DDR
   (docs/buffee/BUFFEE_MAP.md; the image starts with its exception vectors at 0x9F600000, whose first word branches to _start), then called like a function: it hands the CPU back. */
#define PJIT_RX_BUF   0x90000000u       /* the bundle as received */
#define PJIT_SCRATCH  0x91000000u       /* the unpacker's scratch: the largest part (1 MB) + slack */
#define PJIT_IMAGE    0x9F600000u
extern uint32_t LoadXMODEM(uint32_t start);

/* What the bootloader tells the payload (read by platform/buffee/buffee.c): the boot settings and whether the MapROM image is already in the DRAM. */
#define BOOTINFO ((volatile bootinfo_t*)0x9F5FFF00u)
#define BOOTINFO_MAGIC 0x544F4F42u      /* 'BOOT' */
#define MAPROM_STAGE   0x9FE00000u      /* the 1 MB behind guest $F00000-$FFFFFF (BUFFEE_MAPROM_PHYS); the image sits at its offset inside that megabyte */
typedef struct { uint32_t magic; uint16_t maprom_base_64k; uint8_t maprom_src, maprom_size_log2, ram_mode, boot_flags, rom_staged, pad; uint32_t rom_size; struct { uint16_t base_mb; uint8_t size_log2, flags; } region[4]; } bootinfo_t;     /* region[] at byte 16: the guest RAM regions of the EEPROM */

static int pjit_loaded;

/* Step 5 of the boot: the MapROM image into the DRAM. Returns 0 when it is there (or not wanted), -1 when it was wanted and could not be had (the caller stops at the menu). */
static int StageROM(void) {
    config_t *c = &settings;
    bootinfo_t bi = { BOOTINFO_MAGIC, c->maprom_base_64k, c->maprom_src, c->maprom_size_log2, c->ram_mode, c->boot_flags, 0, c->pace, 0 };
    bi.boot_flags |= BOOT_24BIT;                                         /* the 24-bit address space is the only one until a 68020+ model exists (bootinfo byte 11 = pace percent) */
    for (int k = 0; k < 4; k++) { bi.region[k].base_mb = c->mem_region[k].base_mb; bi.region[k].size_log2 = c->mem_region[k].size_log2; bi.region[k].flags = c->mem_region[k].flags; }
    uint32_t base = (uint32_t)c->maprom_base_64k << 16, size = 1u << c->maprom_size_log2;
    if (c->maprom_src != MAPROM_OFF) {
        if ((base & 0xFFFFFu) + size > 0x100000u || size < 0x10000u) { printf("MapROM: base %06lX size %lu KB does not fit one megabyte\n", (unsigned long)base, (unsigned long)(size >> 10)); return -1; }
        uint32_t *sect = (uint32_t*)MAPROM_STAGE;
        uint8_t *dst = (uint8_t*)(MAPROM_STAGE + (base & 0xFFFFFu));
        double t0 = ReadDMTimerSeconds();
        for (uint32_t i = 0; i < 0x100000u / 4; i++) sect[i] = 0xFFFFFFFFu;               /* an empty bus reads 0xFF */
        if (c->maprom_src == MAPROM_FLASH) {
            uint32_t n = 0;
            int rc = StoreLoad(STORE_ROM_OFF, STORE_ROM_MAX, STORE_MAGIC_ROM, dst, &n, 0);
            if (rc) { printf("MapROM: the image in the flash is %s\n", rc == STORE_BAD_CRC ? "damaged (CRC)" : "not there"); return -1; }
            bi.rom_size = n;
            printf("MapROM: %lu bytes from the flash at %06lX", (unsigned long)n, (unsigned long)base);
        } else {
            volatile uint16_t *src = (volatile uint16_t*)base;                              /* halfword copy through the window keeps the Amiga's byte order */
            uint16_t *d16 = (uint16_t*)dst;
            for (uint32_t i = 0; i < size / 2; i++) { d16[i] = src[i]; if ((i & 0xFFF) == 0 && uart_break) return -1; }
            bi.rom_size = size;
            printf("MapROM: %lu KB copied from the Amiga's ROM at %06lX", (unsigned long)(size >> 10), (unsigned long)base);
        }
        bi.rom_staged = 1;
        printf(" in %0.3f s\n", ReadDMTimerSeconds() - t0);
    }
    CP15DCacheCleanFlush();
    *BOOTINFO = bi;
    CP15DCacheCleanFlush();
    return 0;
}

/* Unpack a received or stored bundle at PJIT_RX_BUF into its link addresses. */
static int UnpackPJIT(uint32_t n) {
    lzt_neon_enable();
    double t0 = ReadDMTimerSeconds();
    int rc = lztb_load((const uint8_t*)PJIT_RX_BUF, (uint8_t*)0, (uint8_t*)PJIT_SCRATCH);
    CP15DCacheCleanFlush();
    CP15ICacheFlush();
    printf("Unpacked %lu bytes: rc %d in %0.3f s\n", (unsigned long)n, rc, ReadDMTimerSeconds() - t0);
    pjit_loaded = (rc == 0);
    return rc;
}

/* clean_reset: pulse the Amiga RESET first (a menu run: a bus test may have left the machine in any state). The autoboot path does not: a 68000 sees its RESET once, at power-on, and the machine is
   exactly as the calibration left it (overlay on, DMA off, phase known), so no second pulse and no second calibration. */
static void RunPJIT(int clean_reset) {
    if (!pjit_loaded) { printf("PJIT is not loaded\n"); return; }
    *(volatile uint32_t*)0x9F5FFFF0u = 0;                   /* mailbox: 1 = restart after an Amiga reset */
    if (clean_reset) {
        AssertReset(); WaitMSDMTimer(100); ReleaseReset(); WaitMSDMTimer(100);       /* the machine starts clean: Agnus, Paula, the CIAs and the overlay (it is on after a reset) */
        ASNCalibrate();
    }
    int ret;
    while ((ret = ((int(*)(void))PJIT_IMAGE)()) == 0x100 || (ret & ~0xF) == 0x110) {         /* the payload saw RESET: recalibrate the bus (the colour clock may have re-phased), run it again */
        if (ret == 0x100) {
        double t0 = ReadDMTimerSeconds();
        while (!(*(volatile uint32_t*)(0x481AE000u + 0x138) & (1u << 16)) && ReadDMTimerSeconds() - t0 < 2.0) ;
        WaitMSDMTimer(50);
        printf("\nAmiga reset: recalibrating\n");
        } else {
            /* 0x110 + k: re-initialise one Buffee-side piece (tests/buffee/buffee_reroll.c: which of them holds the per-boot blitter-wedge state?): bit 0 = restart the PRU, bit 1 = reload the GreenPAK image, bit 2 = InitGPMC again; 0x110 alone only recalibrates (control) */
            int k = ret & 0xF;
            if (k & 1) { PRUFullReset(PRU0); WaitMSDMTimer(2); smem[0] = 10; StartPRUMain(); }
            if (k & 2) GPReloadImage();
            if (k & 4) InitGPMC((float)(settings.kHz * 0.001));
            printf("\nre-roll %d\n", k);
        }
        ASNCalibrate();
        *(volatile uint32_t*)0x9F5FFFF0u = 1;
    }
    printf("PJIT returned %d\n", ret);
}

static void LoadPJIT(void) {
    uint32_t n = LoadXMODEM(PJIT_RX_BUF);
    if (n == 0) return;
    if (UnpackPJIT(n)) return;
    StageROM();
    RunPJIT(1);
}

/* The boot sequence after the hardware is up (steps 5-7 of the plan): MapROM, load PJIT from the flash (AUTOLOAD), start it (AUTORUN). Any failure, or ESC three times, stops at the menu.
   BootStage does the loading (steps 5-6; the flash needs no Amiga bus): the boot runs it BEFORE the clock-phase calibration, right after the Amiga's reset was released, so the ~200 ms the
   CIAs need after the release is spent on work; AutoBoot only starts what was staged. Return codes: 0 ready to run, 1 stop at the menu (message printed), 2 ESC x3. */
#define STAGE_NONE (-99)
static int boot_stage_rc = STAGE_NONE;
static int BootStage(void) {
    config_t *c = &settings;
    if (StageROM()) return 1;
    if (uart_break) return 2;
    if (!(c->boot_flags & BOOT_AUTOLOAD)) { printf("Autoload is off: menu\n"); return 1; }
    uint32_t n = 0;
    int rc = StoreLoad(STORE_PJIT_OFF, STORE_PJIT_MAX, STORE_MAGIC_PJIT, (void*)PJIT_RX_BUF, &n, 0);
    if (rc) { printf("PJIT in the flash: %s\n", rc == STORE_BAD_CRC ? "damaged (CRC), menu" : "none stored, menu"); return 1; }
    if (uart_break) return 2;
    if (UnpackPJIT(n)) { printf("Unpack failed: menu\n"); return 1; }
    if (!(c->boot_flags & BOOT_AUTORUN)) { printf("PJIT loaded; autorun is off (R to run)\n"); return 1; }
    return 0;
}
static void AutoBoot(void) {
    int rc = (boot_stage_rc != STAGE_NONE) ? boot_stage_rc : BootStage();
    boot_stage_rc = STAGE_NONE;
    if (rc == 2) goto stop;
    if (rc) return;
    printf("Starting PJIT (ESC x3 inside PJIT: menu)\n");
    UART0Ready();
    if (uart_break) goto stop;
    RunPJIT(0);
    return;
stop:
    uart_break = 0;
    printf("\nESC x3: menu\n");
}

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
        EraseSPI(0, ERASE_4K);          // the boot header: the ROM finds no image in the SPI flash and falls back to the UART ('CCC'); a chip erase takes tens of seconds
        WaitMSDMTimer(400);
        printf(" done\n");
        Reset();
    }
}

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
/* The nearest entry by clock (1-based index, 0 = out of the table): stored in the EEPROM as host_type next to the measured kHz.
 * The clock alone cannot tell a CDTV or a MiniMig from an A500 of the same video standard; this is the first line of the identification. */
static int GetSystemIndex(uint16_t kHz) {
    for(int i=0; i<13; i++) {
        uint16_t mid_kHz = (systems[i].kHz + systems[i + 1].kHz) / 2;
        if(kHz > mid_kHz) return i + 1;
    }
    return 0;
}

static const char* GetSystemName(int index) {
    return (index >= 1 && index <= 13) ? systems[index - 1].name : "Unknown";
}

int main(void) {
    char option[4] = "?";
    double MHz;

    extern bool printTimeStamp;
    extern volatile double printf_overhead;

    InitResetHalt();
    InitDMTimer();
    InitPERPLL();
    InitL3L4Wakeup();
    InitUART0(115200);
    InitPRU();
    InitSPI(24000000);

    printf_overhead = 0.0;
    printTimeStamp = false;

    printf("%c%c%c%c\n\n", NAK, CANCEL, CANCEL, CANCEL);
    EmergencyErase();

    SetEcho(1);

    printf("%sBuild Version, %s, Date %s, Time %s\n\n" , banner , Version, __DATE__, __TIME__);
    printTimeStamp = true;

    InitI2C(100000);

    printf("Scanning bus..\n");
    prom_detected = DetectEEPROM();
    gpak_detected = DetectGP();
    pmic_detected = DetectPMIC();

    /* Core DPLL: DCO = 24 * mult / div MHz; 28 GPMC units (20 ns at 1000/24) must equal 4 CLK7 = 563.9 ns, so 993.4 MHz (952/23) measured to
       zero drift against the PAL bus clock on the bench (869/21 left +0.001 clk per cycle). -DCORE_PLL_MULT=1000 -DCORE_PLL_DIV=24 selects the plain 1 GHz clock. */
#ifndef CORE_PLL_MULT
#define CORE_PLL_MULT 952
#define CORE_PLL_DIV  23
#endif
    if (pmic_detected) InitCorePLL(CORE_PLL_MULT, CORE_PLL_DIV);
    printf("%s mode enabled\n", pmic_detected ? "Nitro" : "Humble");
	
    InitDDR();

    if(settings.post_enable_long_mem) DDRTest();
    
    if(settings.post_enable_checkclk) {
        /* The PRU has been measuring CLK7 since it started (5 ms windows against its own cycle counter). A stored host type gets the 5 ms look: within 0.5% of the stored clock it is
         * the same host and the stored value stays (no EEPROM write, the same GPMC timing every boot); anything else (no host type yet, a different clock) takes the 40 ms measurement. */
        double now5 = PRUClockKHz(1);
        int type = settings.host_type;
        if(type && now5 > 0.0 && now5 > settings.kHz * 0.995 && now5 < settings.kHz * 1.005) {
            MHz = settings.kHz * 0.001;
            printf("Main bus clock %0.3f (5 ms check %0.3f, host unchanged)\n", MHz, now5 * 0.001);
        } else {
            double kHz = PRUClockKHz(PRU_WINDOWS);
            MHz = kHz * 0.001;
            printf("Main bus clock measured at %0.3f (spread %d ppm)\n", MHz, PRUClockSpreadPPM());
            settings.kHz = kHz + 0.5;
            settings.host_type = GetSystemIndex(settings.kHz);
            settings.is_dirty = 1;
        }
    } else {
        MHz = settings.kHz * 0.001;
        printf("Main bus clock set to %0.3f\n", MHz);
    }
    if(MHz < 5.0 || MHz > 16.0) {
        printf("Main bus CLK out of spec, using 8MHz\n");
        MHz = 8.0;
        settings.kHz = 8000;
        settings.is_dirty = 1;
    }

    printf("Hello %s!\n", GetSystemName(GetSystemIndex(settings.kHz)));

    StartPRUMain();
    InitGPMC((float)MHz); 
    ReleaseReset();                                          /* the GreenPAK image, the PRU (E clock) and the bus pads are up: the Amiga's reset ends now (the CIAs need ~200 ms after it before they take a write) */
    InitMMU();

    printf("Image %p ~ %p (%d bytes)\n", &_image_start, &_image_end, (&_image_end - &_image_start));    
    printf("Stack %p ~ %p (%d bytes)\n", &_stack_end, &_stack_top, (&_stack_top - &_stack_end));    

    if(settings.is_dirty) SaveConfigEEPROM(1); else MakeGoodEEPROM();

    if(!settings.cpu_enable_icache) {
        CP15ICacheFlush();
		CP15ICacheDisable();
        printf("ICache disabled\n");
	} else {
        printf("ICache enabled\n");
    } 
	
    if(!settings.cpu_enable_dcache) {
        CP15DCacheFlush();
        CP15DCacheDisable();
        if(!settings.cpu_enable_icache) {
            CP15AuxControlFeatureDisable(0x02);
            printf("DCache & L2 Cache disabled\n");
        } else {
            printf("DCache disabled\n");
        }
	} else {
            printf("DCache & L2 Cache enabled\n");
    }

    /* load the ROM image and the bundle from the flash while the Amiga comes out of reset (not when the ROM is copied through the Amiga's own bus: that needs the calibration first) */
    if (settings.maprom_src != MAPROM_RAM && (settings.boot_flags & BOOT_AUTOLOAD)) boot_stage_rc = BootStage();
    ASNCalibrate();
    InitIRQ();
    irq_reset_cb = ASNCalibrate;

    double boot_done = ReadDMTimerSeconds() - printf_overhead;
    printf("Printf overhead %0.5f seconds\n", printf_overhead);
    printf("Completed in %0.5f seconds (%s)\n", boot_done, (boot_done < 0.250) ? "okay" : "slow");
    printTimeStamp = false;

    AutoBoot();
    uart_break = 0;

    while (1) {
        switch (option[0]) {
            /* TESTS */
        case '1': DDRTest(); break;
        case '2': SPIDump(); break;
        case '3': if (confirm()) SPITest(); break;
        case '4': test_printf(); break;
        case '5': test_native_bogomips(); break;

            /* SETUP */
        case 'J': case 'j': if (confirm()) JumpPJIT(); break;
        case 'L': case 'l': LoadPJIT(); break;
        case 'R': case 'r': RunPJIT(1); break;
        case 'B': case 'b': AutoBoot(); break;
        case 'F': case 'f': ManageStore(); break;
        case 'U': case 'u': {                                            /* update the bootloader in the flash from an xmodem'd bootloader.bin (the ROM never offers CCC while one is there) */
            uint32_t n = LoadXMODEM(PJIT_RX_BUF);
            if (n && (*(volatile uint32_t*)PJIT_RX_BUF >> 24) == 0xEA && n < 0x1C000) WriteImage((void*)PJIT_RX_BUF, (void*)(PJIT_RX_BUF + n));
            else if (n) printf("That does not look like a bootloader image (first word %08lX, %lu bytes)\n", (unsigned long)*(volatile uint32_t*)PJIT_RX_BUF, (unsigned long)n);
        } break;
        case 'C': case 'c': SetEClock(); break;
        case 'E': case 'e': ManageConfig(); break;
        case 'S': case 's': ManageSPI(); break;
        case 'M': case 'm': {
            char q[16]; printf("Address (hex): "); gets(q);
            uint32_t a = hx(q) & ~3u;
            int rows = (a < 0x01000000u || (a >= 0x02000000u && a < 0x03000000u)) ? 8 : 1;   /* peripherals: one row (a longer dump reads reserved registers, e.g. GPMC 0x5000007c, which hangs) */
            for (int r = 0; r < rows; r++, a += 16) {
                printf("%08lx:", (unsigned long)a);
                if (a < 0x01000000u || (a >= 0x02000000u && a < 0x03000000u))    /* the Amiga window (GPMC): 16-bit cycles only, a 32-bit access can hang the GPMC */
                    for (int c = 0; c < 4; c++) { uint32_t hi = *(volatile uint16_t*)(a + 4*c), lo = *(volatile uint16_t*)(a + 4*c + 2); printf(" %08lx", (unsigned long)((hi << 16) | lo)); }
                else for (int c = 0; c < 4; c++) printf(" %08lx", (unsigned long)*(volatile uint32_t*)(a + 4*c));
                printf("\n");
            }
        } break;
        case 'W': case 'w': {
            char q[16]; printf("Address (hex): "); gets(q); uint32_t a = hx(q) & ~3u;
            printf("Value (hex): "); gets(q); uint32_t val = hx(q);
            if (a < 0x01000000u || (a >= 0x02000000u && a < 0x03000000u)) { *(volatile uint16_t*)a = val >> 16; *(volatile uint16_t*)(a + 2) = val; printf("%08lx: %04x%04x\n", (unsigned long)a, *(volatile uint16_t*)a, *(volatile uint16_t*)(a + 2)); }
            else { *(volatile uint32_t*)a = val; printf("%08lx: %08lx\n", (unsigned long)a, (unsigned long)*(volatile uint32_t*)a); }
        } break;
        case 'X': case 'x': if (confirm()) Reset(); break;

        default: printf("Unimplemented\n"); // fallthru
        case 0:
        case 'H': case 'h': case '?': printf("%s", menu); break;
        } // end switch

        printf("Ready\n] ");
        gets(option);

    } // end while

    return 0;
}
