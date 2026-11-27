/* control.c -- the payload's side of the control page (control.h): a request from the Amiga is read out of guest memory, served with the guest stopped, and answered.
 *
 * The flash, the EEPROM and the settings are the bootloader's own code (bootloader/src/store.c, settings_set.c, bootloader/hal/config.c, flash.c, i2c0.c), built into the
 * payload, so an image or a setting written from the Amiga is checked and laid out exactly as the bootloader's menu and request page do it. A PJIT bundle from the Amiga goes
 * into the update slot only: the backup slot, which the UART and JTAG write, stays as it was, and the bootloader falls back to it if the update does not start.
 * Everything those functions print goes into the request's reply (and to the UART), so the program on the Amiga shows what the board said.
 *
 * The board's side (guest memory, the staging buffer, the clocks and caches, the reset) is declared below; platform/buffee/buffee.c has it on the board and tests/amiga/
 * mock_board.c on the host, where tests/amiga/ runs the Amiga program and this file together.
 *
 * Copyright (c) 2026 the Buffee Project - http://www.buffee.ca
 * Licensed under Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International (see LICENSE).
 */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "control.h"
#include "config.h"
#include "store.h"

/* ---- the board's side -------------------------------------------------------------------------------------------------------------- */
void      ctl_guest_read(uint32_t a, void *dst, uint32_t n);          /* guest memory, any RAM the guest can reach (chip RAM through the bus) */
void      ctl_guest_write(uint32_t a, const void *src, uint32_t n);
void      ctl_console(char c);                                        /* the UART */
uint8_t  *ctl_stage(void);                                            /* BFC_STAGE_MAX bytes the payload does not otherwise use */
const volatile uint8_t *ctl_bootinfo(void);                           /* what the bootloader told the payload (bootloader/src/payload.c, bootinfo_t) */
void      ctl_hw_init(void);                                          /* the SPI flash and the I2C bus set up again for this code (once) */
int       ctl_amiga_rom(uint32_t base, uint32_t size, uint8_t *dst);  /* the Amiga's own ROM, read through the bus twice and compared: 0 when both reads agree */
void      ctl_reboot(void);
int       ctl_pace(int percent);                                      /* percent >= 0 sets it; returns the pace in force, -1 if it cannot be changed */
int       ctl_clock(int mhz);                                         /* mhz > 0 sets it; returns the ARM's clock in MHz, -1 if it cannot be changed, -2 if mhz is out of range (said why) */
int       ctl_cache(unsigned set, unsigned clear);                    /* BFC_CACHE_* bits; returns the bits in force, -1 if none can be changed */
void      ctl_board_status(void);                                     /* what only the running payload knows (ctl_printf lines) */
extern volatile int uart_break;                                       /* (store.c reads it: never set while a request is served) */

/* ---- the reply ----------------------------------------------------------------------------------------------------------------------- */
#define REPLY_MAX 8192u
static char reply[REPLY_MAX];
static uint32_t reply_len;
/* Every variable here is in the bss, zero meaning nothing yet: an Amiga reset starts the payload again from its entry without loading it again, so boot.S zeroes the bss
 * but initialised data keeps what it held. */
static uint32_t res_init, res_uc;                                      /* what the resident tag's init reported (BFC_RESIDENT); res_uc only with res_init */

int ctl_putchar(int c)
{
    if (reply_len < REPLY_MAX - 1) reply[reply_len++] = (char)c;
    if (c == '\n') ctl_console('\r');
    ctl_console((char)c);
    return c;
}

int ctl_printf(const char *fmt, ...)
{
    char line[256];
    va_list ap;
    int n, i;

    va_start(ap, fmt);
    n = vsnprintf(line, sizeof line, fmt, ap);
    va_end(ap);
    for (i = 0; line[i]; i++) ctl_putchar(line[i]);
    return n;
}

/* ---- big-endian words of the request ------------------------------------------------------------------------------------------------ */
static uint32_t be32(uint32_t v) { return v >> 24 | (v >> 8 & 0xFF00u) | (v << 8 & 0xFF0000u) | v << 24; }

typedef struct { uint32_t magic, cmd, arg[4], data, len, reply, reply_max; int32_t result; uint32_t out[5]; } req_t;   /* bfc_req_t in this CPU's byte order */

static void req_load(uint32_t a, req_t *r)
{
    uint32_t w[16];
    ctl_guest_read(a, w, sizeof w);
    for (int i = 0; i < 16; i++) ((uint32_t *)r)[i] = be32(w[i]);
}

static void req_answer(uint32_t a, const req_t *r)
{
    uint32_t w[6] = { be32((uint32_t)r->result), be32(r->out[0]), be32(r->out[1]), be32(r->out[2]), be32(r->out[3]), be32(r->out[4]) };
    ctl_guest_write(a + 40, w, sizeof w);
    if (r->reply && r->reply_max) {
        uint32_t n = reply_len < r->reply_max - 1 ? reply_len : r->reply_max - 1;
        ctl_guest_write(r->reply, reply, n);
        ctl_guest_write(r->reply + n, "", 1);
    }
}

/* ---- the settings ------------------------------------------------------------------------------------------------------------------ */
static int settings_read, settings_ok;                                /* settings_ok: what LoadConfigEEPROM found (CFG_*), once settings_read */
int SettingsApply(const char *spec);

static void need_settings(void)
{
    if (settings_read) return;
    ctl_hw_init();
    settings_ok = LoadConfigEEPROM();
    settings_read = 1;
}

static const char *const maprom_names[] = { "OFF", "FLASH", "COPY" };

static void show_regions(const config_t *c)
{
    int k;

    if (!c->mem_region[0].size_log2) { ctl_printf("NONE"); return; }
    for (k = 0; k < 4 && c->mem_region[k].size_log2; k++) {
        uint8_t f = c->mem_region[k].flags;
        ctl_printf("%s%lX/%lu%s%s%s", k ? "," : "", (unsigned long)c->mem_region[k].base_mb << 20, (unsigned long)(1ul << (c->mem_region[k].size_log2 - 20)),
                   (f & 2) ? "B" : "", (f & 1) ? "I" : "", (f & 4) ? "F" : "");
    }
}

/* ---- the flash ----------------------------------------------------------------------------------------------------------------------- */
static uint32_t slot_off(int k) { return k ? STORE_PJIT_UPD : STORE_PJIT_OFF; }

static void show_slot(const char *name, uint32_t off, uint32_t max, uint32_t magic, int booted)
{
    uint32_t len = 0, crc = 0, seq = 0;
    int rc = StoreInfo(off, max, magic, &len, &crc, &seq);

    if (rc == STORE_ABSENT) { ctl_printf("  %-7s empty\n", name); return; }
    ctl_printf("  %-7s %lu bytes, CRC %08lX", name, (unsigned long)len, (unsigned long)crc);
    if (magic == STORE_MAGIC_PJIT) ctl_printf(", #%lu", (unsigned long)seq);
    if (rc == STORE_MARKED_BAD) ctl_printf(", marked bad");
    if (booted) ctl_printf(", running");
    ctl_printf("\n");
}

/* A Kickstart that MapROM can take: 256 KB, 512 KB or 1 MB, the ROM's start ($1111 or $1114, then a JMP) at the beginning of the image or of its upper half, and the ROM's own
   checksum (the longs added with the carry folded back in give $FFFFFFFF). Says which Kickstart it is (version.revision at +12). */
static int kickstart_ok(const uint8_t *p, uint32_t len)
{
    uint32_t half = len > 0x80000u ? len - 0x80000u : 0, sum = 0, i, w;
    const uint8_t *k = p + half;

    if (len != 0x40000u && len != 0x80000u && len != 0x100000u) { ctl_printf("A Kickstart is 256 KB, 512 KB or 1 MB, not %lu bytes\n", (unsigned long)len); return 0; }
    if (!(k[0] == 0x11 && (k[1] == 0x11 || k[1] == 0x14) && k[2] == 0x4E && k[3] == 0xF9)) { ctl_printf("Not a Kickstart\n"); return 0; }
    for (i = half; i < len; i += 4) { w = (uint32_t)p[i] << 24 | p[i + 1] << 16 | p[i + 2] << 8 | p[i + 3]; sum += w; if (sum < w) sum++; }
    if (sum != 0xFFFFFFFFu) { ctl_printf("Kickstart checksum wrong (%08lX): damaged or modified\n", (unsigned long)sum); return 0; }
    ctl_printf("Kickstart %u.%u, %lu KB\n", k[12] << 8 | k[13], k[14] << 8 | k[15], (unsigned long)(len >> 10));
    return 1;
}

static int image_check(int what, uint32_t len, uint32_t crc)
{
    uint8_t *st = ctl_stage();

    if (len == 0 || len > BFC_STAGE_MAX) { ctl_printf("Too large: %lu bytes\n", (unsigned long)len); return BFC_E_RANGE; }
    if (~crc32_update(0xFFFFFFFFu, st, len) != crc) { ctl_printf("Arrived damaged (CRC): nothing written\n"); return BFC_E_CRC; }
    if (what == BFC_IMG_PJIT && memcmp(st, "LZTB", 4)) { ctl_printf("Not a PJIT bundle\n"); return BFC_E_FORMAT; }
    if (what == BFC_IMG_PJIT && len > STORE_PJIT_MAX - 0x100u) { ctl_printf("A PJIT bundle is at most %lu bytes\n", (unsigned long)(STORE_PJIT_MAX - 0x100u)); return BFC_E_RANGE; }
    if (what == BFC_IMG_ROM && !kickstart_ok(st, len)) return BFC_E_FORMAT;
    return BFC_OK;
}

static int store_image(int what, uint32_t len, uint32_t crc, uint32_t *seq_out)
{
    uint32_t s0 = 0, s1 = 0, seq;
    int rc = image_check(what, len, crc);

    if (rc) return rc;
    ctl_hw_init();
    if (what == BFC_IMG_PJIT) {
        if (StoreHeader(STORE_PJIT_OFF, STORE_PJIT_MAX, STORE_MAGIC_PJIT, 0, &s0) == STORE_ABSENT) s0 = 0;
        if (StoreHeader(STORE_PJIT_UPD, STORE_PJIT_MAX, STORE_MAGIC_PJIT, 0, &s1) == STORE_ABSENT) s1 = 0;
        seq = (s0 > s1 ? s0 : s1) + 1;
        ctl_printf("Writing the update slot, #%lu\n", (unsigned long)seq);
        rc = StoreWrite(STORE_PJIT_UPD, STORE_PJIT_MAX, STORE_MAGIC_PJIT, ctl_stage(), len, seq);
        *seq_out = seq;
    } else {
        ctl_printf("Writing the ROM image\n");
        rc = StoreWrite(STORE_ROM_OFF, STORE_ROM_MAX, STORE_MAGIC_ROM, ctl_stage(), len, 0);
    }
    return rc == STORE_OK ? BFC_OK : BFC_E_FLASH;
}

/* The flash holds this image if a slot's header has its length and CRC and the slot reads back whole. For PJIT either slot counts (it says which). */
static int verify_image(int what, uint32_t len, uint32_t crc, uint32_t *same)
{
    uint32_t l, c, seq;
    int k;

    ctl_hw_init();
    *same = 0;
    for (k = 0; k < (what == BFC_IMG_PJIT ? 2 : 1); k++) {
        uint32_t off = what == BFC_IMG_PJIT ? slot_off(k) : STORE_ROM_OFF, max = what == BFC_IMG_PJIT ? STORE_PJIT_MAX : STORE_ROM_MAX;
        uint32_t magic = what == BFC_IMG_PJIT ? STORE_MAGIC_PJIT : STORE_MAGIC_ROM;
        const char *name = what == BFC_IMG_PJIT ? (k ? "the update slot" : "the backup slot") : "the ROM image";
        if (StoreInfo(off, max, magic, &l, &c, &seq) != STORE_OK || l != len || c != crc) continue;
        if (StoreLoad(off, max, magic, 0, 0, 0) != STORE_OK) { ctl_printf("%s: header matches, data does not\n", name); continue; }
        ctl_printf("Same as %s", name);
        if (what == BFC_IMG_PJIT) ctl_printf(", #%lu", (unsigned long)seq);
        ctl_printf("\n");
        *same = 1;
        return BFC_OK;
    }
    ctl_printf("Not in the flash\n");
    return BFC_OK;
}

static int copy_rom(void)
{
    uint32_t base, size;

    need_settings();
    base = (uint32_t)settings.maprom_base_64k << 16; size = 1u << settings.maprom_size_log2;
    if (size < 0x40000u || size > BFC_STAGE_MAX) { ctl_printf("MapROM's size (%lu KB) is not a Kickstart's\n", (unsigned long)(size >> 10)); return BFC_E_ARGS; }
    ctl_printf("This Amiga's ROM, %lu KB at $%06lX: ", (unsigned long)(size >> 10), (unsigned long)base);
    if (ctl_amiga_rom(base, size, ctl_stage())) { ctl_printf("two reads differ: nothing written\n"); return BFC_E_FORMAT; }
    if (!kickstart_ok(ctl_stage(), size)) return BFC_E_FORMAT;
    return StoreWrite(STORE_ROM_OFF, STORE_ROM_MAX, STORE_MAGIC_ROM, ctl_stage(), size, 0) == STORE_OK ? BFC_OK : BFC_E_FLASH;
}

/* ---- the status report --------------------------------------------------------------------------------------------------------------- */
/* Short lines (a Shell is about 77 columns wide in topaz 8): what runs, the flash, then the settings as the words the command takes. */
static const char *onoff(int v) { return v ? "on" : "off"; }

static void show_caches(int v)
{
    ctl_printf("L1I %s, L1D %s, L2 %s, slots %s, blocks %s\n", onoff(v & BFC_CACHE_L1I), onoff(v & BFC_CACHE_L1D), onoff(v & BFC_CACHE_L2),
               onoff(v & BFC_CACHE_SLOTS), onoff(v & BFC_CACHE_BLOCKS));
}

static void status(void)
{
    const volatile uint8_t *bi = ctl_bootinfo();
    int booted = -1, pace, mhz, cache;
    const config_t *c = &settings;

    need_settings();
    if (bi && *(const volatile uint32_t *)bi == 0x544F4F42u && bi[36] < 2) booted = bi[36];
    ctl_printf("Buffee PJIT of %s %s\n", __DATE__, __TIME__);
    ctl_board_status();
    pace = ctl_pace(-1); mhz = ctl_clock(0); cache = ctl_cache(0, 0);
    if (mhz > 0) ctl_printf("ARM       %d MHz", mhz);
    if (pace >= 0) ctl_printf(", pace %d %%%s", pace, pace ? "" : " (off)");
    ctl_printf("\n");
    if (cache >= 0) { ctl_printf("Caches    "); show_caches(cache); }
    ctl_printf("\nFlash\n");
    show_slot("PJIT", STORE_PJIT_OFF, STORE_PJIT_MAX, STORE_MAGIC_PJIT, booted == 0);
    show_slot("Update", STORE_PJIT_UPD, STORE_PJIT_MAX, STORE_MAGIC_PJIT, booted == 1);
    show_slot("ROM", STORE_ROM_OFF, STORE_ROM_MAX, STORE_MAGIC_ROM, 0);
    ctl_printf("\nNext boot%s\n", settings_ok == CFG_LOADED ? "" : settings_ok == CFG_NO_EEPROM ? " (no EEPROM: defaults)" : " (defaults)");
    ctl_printf("  MAPROM=%s MAPROMADDR=%lX MAPROMSIZE=%lu\n", maprom_names[c->maprom_src % 3], (unsigned long)c->maprom_base_64k << 16,
               (unsigned long)((1ul << c->maprom_size_log2) >> 10));
    ctl_printf("  FASTRAM="); show_regions(c); ctl_printf("\n");
    ctl_printf("  PACE=%u CLOCK=%u VOLTAGE=%u.%02u\n", (unsigned)c->pace, (unsigned)c->dpll_mul, c->pmic_voltage / 100u, c->pmic_voltage % 100u);
    if (c->post_checkclk) ctl_printf("  BUSCLOCK=0");
    else ctl_printf("  BUSCLOCK=%u.%03u", c->kHz / 1000u, c->kHz % 1000u);
    ctl_printf(" REPORT=%s SELFTEST=%s,%s\n", c->boot_verbose ? "VERBOSE" : "QUIET", c->post_full ? "FULL" : "SHORT", c->post_nobus ? "NOBUS" : "BUS");
}

/* ---- serving a request --------------------------------------------------------------------------------------------------------------- */
static uint32_t text_in(const req_t *r, char *buf, uint32_t max)
{
    uint32_t n = r->len < max - 1 ? r->len : max - 1;
    ctl_guest_read(r->data, buf, n);
    buf[n] = 0;
    for (uint32_t i = 0; i < n; i++) if (!buf[i]) return i;
    return n;
}

static int save_settings(void)
{
    if (settings_ok == CFG_NO_EEPROM) { ctl_printf("No EEPROM: the settings cannot be saved\n"); return BFC_E_EEPROM; }
    return SaveConfigEEPROM() ? BFC_E_EEPROM : BFC_OK;
}

/* Serves the request block at guest address `a`. Called from the fault handler with the guest stopped at its write to the control page. */
void bfc_serve(uint32_t a)
{
    req_t r;
    char text[256];
    int v;

    reply_len = 0;
    req_load(a, &r);
    if (r.magic != BFC_REQ_MAGIC) return;                              /* not a request: the write is dropped, as any write to a ROM */
    r.result = BFC_OK;
    memset(r.out, 0, sizeof r.out);
    ctl_console('\r'); ctl_console('\n');
    switch (r.cmd) {
    case BFC_STATUS:
        status();
        r.out[0] = res_init;
        r.out[1] = res_init ? res_uc : 0x80000000u;
        break;
    case BFC_RESIDENT:
        if (r.arg[0] > BFC_RES_ADDED) { r.result = BFC_E_ARGS; break; }
        res_init = r.arg[0]; res_uc = r.arg[1];
        ctl_printf("[resident init: %u, use count %d]\n", (unsigned)res_init, (int)res_uc);
        break;
    case BFC_SETTINGS:
        need_settings();
        text_in(&r, text, sizeof text);
        v = SettingsApply(text);
        if (v < 0) { r.result = BFC_E_SETTING; break; }
        r.out[0] = (uint32_t)v;
        if (v) r.result = save_settings();
        break;
    case BFC_UPLOAD:
        if (r.arg[0] > BFC_STAGE_MAX || r.len > BFC_STAGE_MAX - r.arg[0]) { r.result = BFC_E_RANGE; break; }
        ctl_guest_read(r.data, ctl_stage() + r.arg[0], r.len);
        break;
    case BFC_STORE:
        if (r.arg[0] != BFC_IMG_PJIT && r.arg[0] != BFC_IMG_ROM) { r.result = BFC_E_ARGS; break; }
        r.result = store_image((int)r.arg[0], r.arg[1], r.arg[2], &r.out[0]);
        break;
    case BFC_VERIFY:
        if (r.arg[0] != BFC_IMG_PJIT && r.arg[0] != BFC_IMG_ROM) { r.result = BFC_E_ARGS; break; }
        r.result = verify_image((int)r.arg[0], r.arg[1], r.arg[2], &r.out[0]);
        break;
    case BFC_COPY_ROM:
        r.result = copy_rom();
        break;
    case BFC_REBOOT:
        ctl_printf("Rebooting\n");
        ctl_reboot();
        break;
    case BFC_PACE:
        if (r.arg[0] > 255) { r.result = BFC_E_ARGS; break; }
        v = ctl_pace((int)r.arg[0]);
        if (v < 0) { ctl_printf("This PJIT cannot change the pace\n"); r.result = BFC_E_COMMAND; break; }
        r.out[0] = (uint32_t)v;
        ctl_printf("Pace %d %%\n", v);
        if (r.arg[1] & 1) { need_settings(); settings.pace = (uint8_t)r.arg[0]; r.result = save_settings(); }
        break;
    case BFC_CLOCK:
        if (r.arg[0] < 24 || r.arg[0] > 1100) { r.result = BFC_E_ARGS; break; }
        v = ctl_clock((int)r.arg[0]);
        if (v == -2) { r.result = BFC_E_ARGS; break; }
        if (v < 0) { ctl_printf("This PJIT cannot change the clock\n"); r.result = BFC_E_COMMAND; break; }
        r.out[0] = (uint32_t)v;
        ctl_printf("ARM clock %d MHz\n", v);
        if (r.arg[1] & 1) {
            need_settings();
            if (v < 300) ctl_printf("Not saved: the board boots at 300 MHz or more\n");
            else { settings.dpll_mul = (uint16_t)v; r.result = save_settings(); }
        }
        break;
    case BFC_CACHE:
        v = ctl_cache(r.arg[0], r.arg[1]);
        if (v < 0) { ctl_printf("This PJIT cannot change its caches\n"); r.result = BFC_E_COMMAND; break; }
        r.out[0] = (uint32_t)v;
        if (((uint32_t)v & r.arg[1]) || (~(uint32_t)v & r.arg[0])) { ctl_printf("Not all of it can change while PJIT runs:\n"); r.result = BFC_E_COMMAND; }
        ctl_printf("Caches "); show_caches(v);
        break;
    default:
        ctl_printf("Unknown request %lu\n", (unsigned long)r.cmd);
        r.result = BFC_E_COMMAND;
        break;
    }
    req_answer(a, &r);
}
