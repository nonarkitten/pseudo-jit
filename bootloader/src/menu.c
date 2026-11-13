/*
 * menu.c -- the menu: what the bootloader offers when the boot stopped (ESC, a failed self-test, no PJIT in the flash) or PJIT returned. Every choice is a number;
 * 0 goes back (or, at the top, reboots). The settings have their own page (settings.c).
 *
 * Copyright (c) 2021-2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include "main.h"
#include "console.h"
#include "store.h"
#include "payload.h"
#include "post.h"
#include "busclock.h"
#include "amiga.h"

#define DUMP_ROWS 8                     /* rows of 16 bytes a memory dump shows (one for a peripheral: a longer dump reads reserved registers, e.g. GPMC 0x5000007c, which hangs) */

static const char* const eeprom_state[] = { "settings loaded", "blank: defaults", "bad CRC: defaults", "not found: defaults" };     /* by CFG_* */
static const char* const post_state[] = { "passed", "FAILED", "stopped" };                                                         /* by POST_* */

/* The entries that use the Amiga's bus need it running (main.c starts it only with the GreenPAK). */
static int NeedBus(void) {
    if (!boot.bus) printf("The Amiga's bus is not running (no GreenPAK, or the boot was stopped while the host held RESET)\n");
    return boot.bus;
}

/* ---- the images in the flash ---------------------------------------------------------------------------------------------------------- */

static void SlotStatus(const char* name, uint32_t off, uint32_t max, uint32_t magic) {
    uint32_t n = 0;
    int rc = StoreLoad(off, max, magic, 0, &n, 0);

    if (rc == STORE_OK) printf("  %s: ok, %lu bytes\n", name, (unsigned long)n);
    else printf("  %s: %s\n", name, rc == STORE_BAD_CRC ? "damaged (CRC)" : "absent");
}

/* The Amiga's own ROM (the MapROM address and size of the settings) read through the bus, as an image for the ROM slot. */
static void StoreAmigaROM(void) {
    uint32_t base = (uint32_t)settings.maprom_base_64k << 16, size = 1u << settings.maprom_size_log2;

    if (!NeedBus()) return;
    printf("Reading %lu KB at %06lX\n", (unsigned long)(size >> 10), (unsigned long)base);
    AmigaCopy((void*)RX_BUF, base, size);
    StoreWrite(STORE_ROM_OFF, STORE_ROM_MAX, STORE_MAGIC_ROM, (void*)RX_BUF, size, 0);
}

static void FlashMenu(void) {
    uint32_t n;

    for (;;) {
        printf("\nFLASH IMAGES\n");
        SlotStatus("PJIT", STORE_PJIT_OFF, STORE_PJIT_MAX, STORE_MAGIC_PJIT);
        SlotStatus("ROM ", STORE_ROM_OFF, STORE_ROM_MAX, STORE_MAGIC_ROM);
        printf(" 1. Store a PJIT bundle (XMODEM)\n"
               " 2. Store a ROM image (XMODEM)\n"
               " 3. Store a copy of the Amiga's ROM\n"
               " 4. Erase the PJIT bundle\n"
               " 5. Erase the ROM image\n"
               " 6. Update the bootloader (XMODEM)\n"
               " 0. Back\n");
        switch (MenuChoice()) {
        case 0: return;
        case 1: if ((n = LoadXMODEM(RX_BUF))) StoreWrite(STORE_PJIT_OFF, STORE_PJIT_MAX, STORE_MAGIC_PJIT, (void*)RX_BUF, n, 0); break;
        case 2: if ((n = LoadXMODEM(RX_BUF))) StoreWrite(STORE_ROM_OFF, STORE_ROM_MAX, STORE_MAGIC_ROM, (void*)RX_BUF, n, 0); break;
        case 3: StoreAmigaROM(); break;
        case 4: if (Confirm("Erase the PJIT bundle")) StoreErase(STORE_PJIT_OFF); break;
        case 5: if (Confirm("Erase the ROM image")) StoreErase(STORE_ROM_OFF); break;
        case 6: if ((n = LoadXMODEM(RX_BUF))) UpdateBootloader((void*)RX_BUF, n); break;
        default: break;
        }
    }
}

/* ---- diagnostics ---------------------------------------------------------------------------------------------------------------------- */

static void BoardSummary(void) {
    printf("Bootloader  %s, %s %s, %lu bytes\n", Version, __DATE__, __TIME__, (unsigned long)(&_image_end - &_image_start));
    printf("ARM         %d MHz%s\n", boot.pmic ? settings.dpll_mul : 300, boot.pmic ? "" : " (no PMIC, or the last boot failed)");
    printf("Bus clock   %0.3f MHz, %s\n", boot.bus_mhz, HostName());
    printf("GreenPAK    %s\n", boot.greenpak ? "found" : "NOT FOUND");
    printf("Data lines  %s (%s)\n", AmigaLanesName(), AmigaLanesFrom());
    printf("EEPROM      %s\n", eeprom_state[boot.eeprom & 3]);
    printf("SPI flash   ID %04X\n", ReadSPIID());
    printf("Self-test   %s\n", post_state[boot.post % 3]);
}

static int IsAmigaWindow(uint32_t a) { return a < 0x01000000u || (a >= 0x02000000u && a < 0x03000000u); }   /* the GPMC: 16-bit cycles only, a 32-bit access can hang it */

static void PeekMemory(void) {
    char q[16];
    uint32_t a, v;
    int rows, r, c;

    if (!ParseHex(Ask("Address (hex): ", q, sizeof q), &a)) return;
    a &= ~3u;
    if (IsAmigaWindow(a) && !NeedBus()) return;
    rows = (IsAmigaWindow(a) || (a >= 0x80000000u && a < 0xA0000000u - 16 * DUMP_ROWS)) ? DUMP_ROWS : 1;      /* the Amiga window or the DDR */
    for (r = 0; r < rows; r++, a += 16) {
        printf("%08lx:", (unsigned long)a);
        for (c = 0; c < 4; c++) {
            v = IsAmigaWindow(a) ? (uint32_t)AmigaRead16(a + 4 * c) << 16 | AmigaRead16(a + 4 * c + 2) : *(volatile uint32_t*)(a + 4 * c);
            printf(" %08lx", (unsigned long)v);
        }
        printf("\n");
    }
}

static void PokeMemory(void) {
    char q[16];
    uint32_t a, v;

    if (!ParseHex(Ask("Address (hex): ", q, sizeof q), &a) || !ParseHex(Ask("Value (hex): ", q, sizeof q), &v)) return;
    a &= ~3u;
    if (IsAmigaWindow(a) && !NeedBus()) return;
    if (IsAmigaWindow(a)) {
        AmigaWrite16(a, v >> 16); AmigaWrite16(a + 2, v);
        printf("%08lx: %04x%04x\n", (unsigned long)a, AmigaRead16(a), AmigaRead16(a + 2));
    } else {
        *(volatile uint32_t*)a = v;
        printf("%08lx: %08lx\n", (unsigned long)a, (unsigned long)*(volatile uint32_t*)a);
    }
}

static void DiagnosticsMenu(void) {
    for (;;) {
        printf("\nDIAGNOSTICS\n"
               " 1. Board summary\n"
               " 2. Test all of the DDR (erases a loaded PJIT)\n"
               " 3. Full self-test and boot report at the next boot\n"
               " 4. Read memory (hex dump)\n"
               " 5. Write a 32-bit word\n"
               " 6. Find the board kind again (the GreenPAK supply test, stored)\n"
               " 0. Back\n");
        switch (MenuChoice()) {
        case 0: return;
        case 1: BoardSummary(); break;
        case 2: UART0Flush(); DDRLineTest(); DDRFullTest(); UART0Flush(); break;
        case 3: settings.first_boot = 1; if (SaveConfigEEPROM() == 0) printf("The next boot runs the full self-test with the report on\n"); break;
        case 4: PeekMemory(); break;
        case 5: PokeMemory(); break;
        case 6: if (AmigaRetestLanes() < 0) printf("The supply test cannot tell (no GreenPAK, or its registers hold what its NVM holds)\n");
                else printf("Board kind: %s\n", AmigaLanesName());
                break;
        default: break;
        }
    }
}

/* ---- the top -------------------------------------------------------------------------------------------------------------------------- */

void Menu(void) {
    console_verbose = true;                                     /* what the menu starts reports what it does */
    printTimeStamp = false;
    UART0Flush();
    for (;;) {
        printf("\nBUFFEE %s\n"
               " 1. Boot PJIT from the flash\n"
               " 2. Run a PJIT bundle sent over XMODEM (not stored)\n"
               " 3. Flash images\n"
               " 4. Settings\n"
               " 5. Diagnostics\n"
               " 0. Reboot\n", Version);
        switch (MenuChoice()) {
        case 0: if (Confirm("Reboot")) Reset(); break;
        case 1: if (NeedBus()) PJITBoot(); break;
        case 2: if (NeedBus() && PJITReceive() == 0 && StageROM() == 0) PJITRun(1); break;
        case 3: FlashMenu(); break;
        case 4: SettingsMenu(); break;
        case 5: DiagnosticsMenu(); break;
        default: break;
        }
        UART0Flush();                                           /* ESC x3 that brought us back from PJIT */
    }
}
