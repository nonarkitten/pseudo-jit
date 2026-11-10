/// @file buffee_shotgun.c
/// @brief GreenPAK netlist shotgun: the host (tools/buffee/shotgun.py) sends one combination at a time over the UART; the payload applies the register / matrix edits live over I2C0,
///        runs a short bus test (read a zero word, write / read back random chip RAM words) and times it with the cycle counter, prints one result line, restores the baseline registers.
///        The SoC watchdog (WDT1, 4 s) is armed and kicked in every loop: a combination that stalls the bus resets the board, the host reloads the payload and goes on.
///        Command line: `S <id> <op>,<op>,...` with op `M<out>:<src>` (matrix output <out> <- input <src>, decimal), `R<reg>:<val>` (raw byte, hex), `P` (sample matrix 40 and 24); `Q` quits.
///        Output: `SGREADY`, per command `SGSET <id>` (when the bus test starts) and `SGRES <id> rbad rcyc wbad wcyc p40 p24` (bad reads of a zero word, ARM cycles per read, wrong
///        read-backs of random writes, ARM cycles per write + read-back pair, how often matrix inputs 40 / 24 were high of 100).
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"
#define R32(a) (*(volatile uint32_t *)(a))
#define I2C(off) R32(0x44E0B000u + (off))
static int gp_xfer(const uint8_t *cmd, int ncmd, uint8_t *data, int n, int rd)
{
    I2C(0xAC) = 1u << 3; I2C(0x98) = rd ? ncmd : ncmd + n; I2C(0x28) = 0x7FFF; I2C(0xA4) = 0x8600;
    while (I2C(0x24) & (1u << 12)) ;
    I2C(0xA4) |= 1;
    int c = ncmd, d = 0;
    while (c || (!rd && d < n)) {
        uint32_t s = I2C(0x24);
        if (s & 3) break;
        if (s & (1u << 4)) { I2C(0x9C) = c ? cmd[ncmd - c--] : data[d++]; I2C(0x28) = 1u << 4; if (!c && (rd || d == n)) { while (!(I2C(0x24) & 4)) ; I2C(0x28) = 4; } }
    }
    if (rd) {
        I2C(0x98) = n; I2C(0xA4) = (I2C(0xA4) | 3) & ~(1u << 9);
        for (int k = 0; k < n; ) { uint32_t s = I2C(0x24); if (s & 3) break; if (s & 8) { data[k++] = (uint8_t)I2C(0x9C); I2C(0x28) = 8; if (k == n) { while (!(I2C(0x24) & 4)) ; I2C(0x28) = 4; } } }
    } else I2C(0xA4) |= 2;
    while (!(I2C(0x24) & (1u << 8))) ;
    I2C(0x94) |= (1u << 14) | (1u << 6); I2C(0x28) = 0x7FFF; I2C(0x98) = 0;
    return 0;
}
static void gp_write(uint8_t reg, uint8_t v) { gp_xfer(&reg, 1, &v, 1, 0); }
static uint8_t gp_read(uint8_t reg) { uint8_t v = 0; gp_xfer(&reg, 1, &v, 1, 1); return v; }
static void set_matrix(int out, int src)                          // matrix output `out` <- input `src`: 6 bits at bit 6*out
{
    for (int k = 0; k < 6; k++) {
        int bit = 6 * out + k, r = bit >> 3, sh = bit & 7;
        uint8_t v = gp_read((uint8_t)r); v = (uint8_t)((v & ~(1u << sh)) | (((src >> k) & 1u) << sh)); gp_write((uint8_t)r, v);
    }
}
// ---- watchdog -------------------------------------------------------------------------------------------------------------------
#define WDT(off) R32(0x44E35000u + (off))
static void wdt_wait(void) { for (uint32_t s = 0; WDT(0x34) && s < 1000000; s++) ; }
static void wdt_kick(void) { static uint32_t n; WDT(0x30) = ++n; wdt_wait(); }
static void wdt_arm(uint32_t seconds)
{
    R32(0x44E004D4) = 2; for (uint32_t s = 0; ((R32(0x44E004D4) >> 16) & 3) && s < 1000000; s++) ;     // CM_WKUP_WDT1_CLKCTRL: module enabled
    wdt_wait(); WDT(0x24) = 0; wdt_wait();                                                              // no prescaler: the 32.768 kHz clock
    WDT(0x2C) = 0xFFFFFFFFu - 32768u * seconds + 1; wdt_wait();
    WDT(0x30) = 1; wdt_wait();                                                                          // load the counter
    WDT(0x48) = 0xBBBB; wdt_wait(); WDT(0x48) = 0x4444; wdt_wait();                                       // enable sequence
}
static void wdt_disarm(void) { WDT(0x48) = 0xAAAA; wdt_wait(); WDT(0x48) = 0x5555; wdt_wait(); }
// ---- UART (polled; the IRQ is masked in a bare-metal payload) ---------------------------------------------------------------------
#define UART_RX_READY (R32(0x44E09014) & 1)
static int getline_(char *b, int n)                              // returns the length, or -1 after ESC x3
{
    int len = 0, esc = 0;
    for (;;) {
        wdt_kick();
        if (!UART_RX_READY) continue;
        uint8_t c = (uint8_t)R32(0x44E09000);
        esc = c == 0x1B ? esc + 1 : 0;
        if (esc >= 3) return -1;
        if (c == '\n' || c == '\r') { if (len) { b[len] = 0; return len; } continue; }
        if (len < n - 1) b[len++] = (char)c;
    }
}
static uint32_t rnd(void) { static uint32_t x = 20261031; x = x * 1664525u + 1013904223u; return x >> 8; }
#define ZW 0x61000u
#ifndef WR_BASE
#define WR_BASE 0x70000u
#endif
#ifndef NR
#define NR 60000
#endif
static uint8_t base_regs[0xC8];
#define GPMC(off) R32(0x50000000u + (off))
static uint32_t gpmc_base[8];                                     // CONFIG1..7 of CS2 (0xB0..0xD8 step 0x20 is CS1; CS2 is 0xC0 + ...): saved at start, restored after each command
static int sample_in(int input, int n) { int hi = 0; for (int k = 0; k < n; k++) hi += (gp_read((uint8_t)(0x74 + input / 8)) >> (input & 7)) & 1; return hi; }
static unsigned hexv(const char *s) { unsigned v = 0; while (*s) { char c = *s++; v = v * 16 + (c <= '9' ? c - '0' : (c | 32) - 'a' + 10); } return v; }
static int decv(const char *s) { int v = 0; while (*s >= '0' && *s <= '9') v = v * 10 + *s++ - '0'; return v; }
int main(void)
{
    printf("=== GreenPAK shotgun ===\n");
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
    write_BE_word(0xDFF096, 0x7FFF);
    write_BE_word(ZW, 0);
    for (int r = 0; r < 0xC8; r++) base_regs[r] = (r >= 0x74 && r < 0x80) ? 0 : gp_read((uint8_t)r);
    for (int k = 0; k < 7; k++) gpmc_base[k] = GPMC(0x60 + 0x30 * 2 + 4 * k);    // CS2 CONFIG1..7 at 0x60 + 0x30 * cs
    printf("SGGPMC CONFIG1..7: %08X %08X %08X %08X %08X %08X %08X\n", (unsigned)gpmc_base[0], (unsigned)gpmc_base[1], (unsigned)gpmc_base[2], (unsigned)gpmc_base[3], (unsigned)gpmc_base[4], (unsigned)gpmc_base[5], (unsigned)gpmc_base[6]);
    wdt_arm(4);
#ifdef KIPPER
    *(volatile uint8_t *)0xE8004A = 0x00; *(volatile uint8_t *)0xE80048 = 0x20;              /* configure the Zorro II RAM board at $200000 (what expansion.library does) */
    for (volatile int w = 0; w < 3000000; w++) ;
#endif
    printf("SGREADY 0x9C=%02X 0xB5=%02X\n", base_regs[0x9C], base_regs[0xB5]);
    char line[200];
    for (;;) {
        int n = getline_(line, sizeof line);
        if (n < 0 || line[0] == 'Q') break;
        if (line[0] != 'S') continue;
        char *p = line + 2; int id = decv(p); while (*p && *p != ' ') p++; while (*p == ' ') p++;
        int gpmc_dirty = 0;
        int probes = 0, p40 = -1, p24 = -1, nr = NR, phases = 7;
        while (*p) {
            char *e = p; while (*e && *e != ',') e++;
            char saved = *e; *e = 0;
            if (*p == 'M') { int out = decv(p + 1); char *c = p; while (*c && *c != ':') c++; set_matrix(out, decv(c + 1)); }
            else if (*p == 'R') { char *c = p; while (*c && *c != ':') c++; char t = *c; *c = 0; unsigned reg = hexv(p + 1); *c = t; gp_write((uint8_t)reg, (uint8_t)hexv(c + 1)); }
            else if (*p == 'G') { char *c = p; while (*c && *c != ':') c++; char t = *c; *c = 0; unsigned k = hexv(p + 1); *c = t; GPMC(0x60 + 0x30 * 2 + 4 * 6) = 0; for (volatile int w = 0; w < 20000; w++) ; GPMC(0x60 + 0x30 * 2 + 4 * (k - 1)) = hexv(c + 1); gpmc_dirty = 1; }   // G<n>:<hex>: CS2 CONFIG<n> <- value (CS off while writing)
            else if (*p == 'P') probes = 1;
            else if (*p == 'N') nr = decv(p + 1);
            else if (*p == 'L') {                                         // L<mult>:<div>: relock the Core DPLL (DCO = 24 * mult / div MHz; boot value 1000:24 = GPMC unit 20.000 ns); 869:21 trims 28 units to 4 PAL CLK7
                char *c = p; while (*c && *c != ':') c++; unsigned mult = decv(p + 1), div = decv(c + 1);
                R32(0x44E00490u) = (R32(0x44E00490u) & ~7u) | 4;          // CM_CLKMODE_DPLL_CORE: MN bypass
                while (!(R32(0x44E0045Cu) & 0x100) || (R32(0x44E0045Cu) & 1)) ;
                R32(0x44E00468u) = (R32(0x44E00468u) & ~0x7FF7Fu) | (mult << 8) | (div - 1);
                R32(0x44E00490u) = (R32(0x44E00490u) & ~7u) | 7;          // lock
                while ((R32(0x44E0045Cu) & 0x100) || !(R32(0x44E0045Cu) & 1)) ;
            }
            else if (*p == 'T') phases = decv(p + 1);                    // 1: reads only, 2: chip RAM writes only, 3: both (for Logic captures of one phase)                        // loop length (for a Logic capture: millions)
            *e = saved; p = saved ? e + 1 : e;
            wdt_kick();
        }
        if (gpmc_dirty) { GPMC(0x60 + 0x30 * 2 + 4 * 6) = gpmc_base[6]; for (volatile int w = 0; w < 20000; w++) ; }
        (void)probes; p40 = sample_in(40, 100); p24 = sample_in(24, 100); int q45 = sample_in(45, 100), q46 = sample_in(46, 100), q22 = sample_in(22, 100), q20 = sample_in(20, 100), q18 = sample_in(18, 100); wdt_kick();
        printf("SGIDLE %d asn_out(18)=%d d5q(20)=%d wrrel(22)=%d lut24=%d lut45=%d lut46=%d  (of 100)\n", id, q18, q20, q22, p24, q45, q46);
        printf("SGCFG %d %08X %08X %08X %08X %08X %08X %08X  (CS2 CONFIG1..7 as the GPMC holds them; CONFIG1 bit 4 = TIMEPARAGRANULARITY x2)\n", id, (unsigned)GPMC(0x60 + 0x30 * 2), (unsigned)GPMC(0x60 + 0x30 * 2 + 4), (unsigned)GPMC(0x60 + 0x30 * 2 + 8), (unsigned)GPMC(0x60 + 0x30 * 2 + 12), (unsigned)GPMC(0x60 + 0x30 * 2 + 16), (unsigned)GPMC(0x60 + 0x30 * 2 + 20), (unsigned)GPMC(0x60 + 0x30 * 2 + 24));
        printf("SGSET %d\n", id);
        uint32_t rbad = 0, wbad = 0, cbad = 0, w2bad = 0;
        uint32_t c0 = bbb_pmu_cycles();
        for (int i = 0; i < (phases & 1 ? nr : 0); i++) { if (read_BE_word(ZW)) rbad++; if ((i & 0x3FFF) == 0) wdt_kick(); }
        uint32_t c1 = bbb_pmu_cycles();
        for (int i = 0; i < (phases & 2 ? nr : 0); i++) {
            uint32_t a = WR_BASE + ((rnd() & 0x7FFF) << 1); uint16_t v = (uint16_t)rnd();
            write_BE_word(a, v); if (read_BE_word(a) != v) wbad++; if (read_BE_word(a) != v) w2bad++;
            if ((i & 0x3FFF) == 0) wdt_kick();
        }
        uint32_t c2 = bbb_pmu_cycles();
        uint32_t scyc = 0, sbad = 0;
        if (phases & 4) {                                              // back-to-back halfword stores to consecutive chip RAM (posted: the GPMC sets the rate), read back and verified afterwards
            const uint32_t WS = 8192, SB = 0x80000u;
            uint32_t ns = nr < 2 * WS ? 2 * WS : (uint32_t)nr;
            uint32_t s0 = bbb_pmu_cycles();
            for (uint32_t i = 0; i < ns; i++) { write_BE_word(SB + ((i & (WS - 1)) << 1), (uint16_t)(i * 40503u + 4660u)); if ((i & 0x3FFF) == 0) wdt_kick(); }
            (void)read_BE_word(ZW);                                    // a read cannot overtake posted writes: drains the queue before the clock is read
            uint32_t s1 = bbb_pmu_cycles();
            scyc = (s1 - s0) / ns;
            for (uint32_t k = 0; k < WS; k++) { uint32_t i = ns - WS + k; if (read_BE_word(SB + ((i & (WS - 1)) << 1)) != (uint16_t)(i * 40503u + 4660u)) sbad++; if ((k & 0x3FFF) == 0) wdt_kick(); }
        }
        write_BE_word(ZW, 0);
        uint32_t cx = 0, first = 0xFFFF; for (int i = 0; i < 20000; i++) { uint8_t v = (uint8_t)rnd(); *(volatile uint8_t *)0xBFE301 = v; uint8_t g = *(volatile uint8_t *)0xBFE301; if (g != v) { cbad++; cx |= v ^ g; if (first == 0xFFFF) first = (v << 8) | g; } if ((i & 0x3FFF) == 0) wdt_kick(); }   // CIAA DDRB (the parallel port direction): write / read-back through the 6800-style cycle
        *(volatile uint8_t *)0xBFE301 = 0;
        printf("SGRES %d cxor=%02X cfirst=%04X rbad=%u rcyc=%u wbad=%u wcyc=%u cbad=%u scyc=%u sbad=%u p40=%d p24=%d w2=%u\n", id, (unsigned)cx, (unsigned)first, (unsigned)rbad, (unsigned)((c1 - c0) / (nr ? nr : 1)), (unsigned)wbad, (unsigned)((c2 - c1) / (nr ? nr : 1)), (unsigned)cbad, (unsigned)scyc, (unsigned)sbad, p40, p24, (unsigned)w2bad);
        for (int r = 0; r < 0xC8; r++) { if (r >= 0x74 && r < 0x80) continue; if (gp_read((uint8_t)r) != base_regs[r]) gp_write((uint8_t)r, base_regs[r]); }
        if (gpmc_dirty) { GPMC(0x60 + 0x30 * 2 + 4 * 6) = 0; for (volatile int w = 0; w < 20000; w++) ; for (int k = 0; k < 6; k++) GPMC(0x60 + 0x30 * 2 + 4 * k) = gpmc_base[k]; GPMC(0x60 + 0x30 * 2 + 4 * 6) = gpmc_base[6]; for (volatile int w = 0; w < 20000; w++) ; }
        wdt_kick();
    }
    for (int r = 0; r < 0xC8; r++) { if (r >= 0x74 && r < 0x80) continue; if (gp_read((uint8_t)r) != base_regs[r]) gp_write((uint8_t)r, base_regs[r]); }
    wdt_disarm();
    *(volatile uint8_t *)0xBFE001 |= 1;
    printf("SGQUIT\n");
    return 0;
}
