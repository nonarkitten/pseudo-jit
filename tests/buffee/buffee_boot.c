/// @file buffee_boot.c
/// @brief Boots the Amiga's Kickstart on PJIT: the ROM is copied into DRAM and mapped at $F00000 (MapROM), the 68000 starts as after a reset (SSP and PC from the
///        first two longs of the ROM, which the overlay presents at $0; supervisor mode, interrupts masked) and runs until the Amiga is reset (Ctrl-A-A, then the bootloader
///        recalibrates the bus and starts this again). Guest code in the ROM ($F00000 and up) runs from the translation cache; everything below that (the OS's own code
///        in RAM) is interpreted, because the cache does not follow self-modifying code and AmigaOS loads code into memory that held other code.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"
#include "cp15.h"

#define NOBP 1



/* #define TRACEW 1 */     /* custom-chip write log (every write faults: it perturbs the run); define it for the blitter-register log in the stall dump */
#ifndef TRACEW
#define NOTRACE 1
#endif
extern void emit_opcode_table(void);
#ifndef DRT
#define DRT 0xF00000u
#endif
#ifndef BREAK_AT
#define BREAK_AT 0xF800F0u
#endif

uint32_t pjit_trace[0x80000 / 4] __attribute__((aligned(16))); uint32_t pjit_trace_pos; uint32_t pjit_trace_frz; uint16_t pjit_xmem[320]; uint32_t pjit_vdbg[512];
#ifdef PACE
/* PACE=n: the interpreter runs against a virtual MC68000 clock at n percent of 7.09 MHz (100 = a real 68000's pace). pjit/runtime.s (PJIT_PACE) reads these big-endian: the table of estimated cycles per
   opcode word (platform/buffee/pace_tab.h, tools/data/gen_pace_tab.py), the ARM cycles per 68000 cycle and the virtual clock (a PMU cycle count). Interpreted code only: build with -DDRT=0xFFFFFFFFu. */
#include "pace_tab.h"
#ifndef PACE_MHZ
#define PACE_MHZ 1000u                                                /* the CPU clock the K scaling assumes (the EEPROM's dpll_mul: 1000 = 1 GHz) */
#endif
uint32_t pjit_pace_k, pjit_pace_dl[8], pjit_pace_wraps, pjit_pace_gap[2 + 32];
#endif
#ifdef VBLLOG
uint8_t pjit_vbl_ring[0x40000] __attribute__((aligned(16))); uint32_t pjit_vbl_pos;
#ifndef PACE
uint32_t pjit_pace_dl[8];
#endif
#endif
extern void pjit_prefetch_trace(void);
extern uint32_t pjit_dlog[256], pjit_dlog_pos;


static uint32_t ec_add(uint32_t a, uint32_t b) { uint64_t t = (uint64_t)a + b; return (uint32_t)t + (uint32_t)(t >> 32); }
static uint32_t rd32(const uint8_t *p) { return (uint32_t)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]; }

static uint32_t rl(uint32_t a) { return (uint32_t)read_BE_word(a) << 16 | read_BE_word(a + 2); }
#ifndef SAMPV
#define SAMPV 0
#endif
#ifndef SAMPLES
#define SAMPLES 24
#endif
static void gname(uint32_t a) { char b[28]; int n = 0; if (a < 0x1000000) for (; n < 26; n++) { uint16_t w = read_BE_word(a & ~1u); uint8_t c = (a & 1) ? (w & 255) : (w >> 8); if (c < 32 || c > 126) break; b[n] = c; a++; } b[n] = 0; printf("%s", b); }
static void dump_tasks(void)
{
    uint32_t eb = rl(4); static const struct { uint32_t off; const char *n; } L[] = { { 0x196, "ready" }, { 0x1A4, "wait" } };
    uint32_t tt = rl(eb + 0x114);
    printf("tasks: ThisTask %06X ", (unsigned)tt); if (tt) gname(rl(tt + 10)); printf("; IDNestCnt %d TDNestCnt %d\n", (int)(int8_t)(read_BE_word(eb + 0x126) >> 8), (int)(int8_t)(read_BE_word(eb + 0x126) & 255));
    for (int l = 0; l < 2; l++) for (uint32_t t = rl(eb + L[l].off), k = 0; t && rl(t) && k < 40; t = rl(t), k++) {
        printf("  %-5s %06X pri %d state %02X sigwait %08X sigrecvd %08X  ", L[l].n, (unsigned)t, (int)(int8_t)(read_BE_word(t + 8) & 255), (unsigned)(read_BE_word(t + 14) & 255), (unsigned)rl(t + 0x16), (unsigned)rl(t + 0x1A)); gname(rl(t + 10)); printf("\n"); }
}
static void dump_state(void)
{
    printf("regs:"); for (int i = 0; i < 8; i++) printf(" D%d=%08X", i, pjit_get_dn(i)); for (int i = 0; i < 8; i++) printf(" A%d=%08X", i, pjit_get_an(i)); printf(" PC(next)=%08X\n", (unsigned)(uint32_t)cpu->pc);
    { extern uint32_t pjit_dlog[], pjit_dlog_pos; printf("exceptions delivered (vector@guest PC), oldest first:");
      for (int k = 32; k >= 1; k--) { uint32_t e = ((pjit_dlog_pos / 8 - k) & 31) * 2; printf(" %X@%X", (unsigned)pjit_dlog[e], (unsigned)pjit_dlog[e + 1]); } printf("\n"); 
#ifdef SLOTDUMP
      { uint32_t e = ((pjit_dlog_pos / 8 - 1) & 31) * 2, pc = pjit_dlog[e + 1] & ~1u, vec = pjit_dlog[e];
        printf("last exception vector %X at guest %08X: slot cache around it (tag of its line %04X):\n", (unsigned)vec, (unsigned)pc, (unsigned)cpu->tags[(pc >> 7) & 0xFFF]);
        for (uint32_t g = (pc - 8) & ~1u; g < pc + 10; g += 2) {
            volatile uint32_t *sl = (volatile uint32_t *)((uint32_t)cpu->cache_data + ((g & 0x7FFFFu) << 3));
            printf("  %08X (ROM word %04X): %08X %08X %08X %08X\n", (unsigned)g, (unsigned)read_BE_word(g), (unsigned)__builtin_bswap32(sl[0]), (unsigned)__builtin_bswap32(sl[1]), (unsigned)__builtin_bswap32(sl[2]), (unsigned)__builtin_bswap32(sl[3]));
        } }
#endif
    }
    { uint32_t a1 = pjit_get_an(1) & 0x1FFFFE, a2 = pjit_get_an(2) & 0x1FFFFE;
      printf("RastPort@%06X:", (unsigned)a1); for (uint32_t k = 0; k < 0x60; k += 2) printf(" %04X", read_BE_word(a1 + k)); printf("\nBitMap@%06X:", (unsigned)a2); for (uint32_t k = 0; k < 0x30; k += 2) printf(" %04X", read_BE_word(a2 + k)); printf("\n"); }
    printf("stack:"); for (uint32_t a = pjit_get_an(7) & 0x1FFFFC; a < (pjit_get_an(7) & 0x1FFFFC) + 96; a += 4) printf(" %08X", (unsigned)(read_BE_word(a) << 16 | read_BE_word(a + 2))); printf("\n");
    uint32_t cnt = buffee_wlog_pos;
    printf("custom register writes so far %u, the last 64:", (unsigned)cnt);
    for (uint32_t j = cnt > 64 ? cnt - 64 : 0; j < cnt; j++) printf(" %03X=%X", (unsigned)(buffee_wlog[2 * (j & 4095)] & 0xFFF), (unsigned)buffee_wlog[2 * (j & 4095) + 1]);
    { printf("\nshadow: DIWSTRT %04X DIWSTOP %04X DDFSTRT %04X DDFSTOP %04X BPLCON0 %04X BPLCON1 %04X BPLCON2 %04X BPL1MOD %04X BPL2MOD %04X COP1LC %04X%04X COPJMP1 %04X DMACON %04X; BPL1PT %04X%04X\n",
              buffee_shadow[0x8E / 2], buffee_shadow[0x90 / 2], buffee_shadow[0x92 / 2], buffee_shadow[0x94 / 2], buffee_shadow[0x100 / 2], buffee_shadow[0x102 / 2], buffee_shadow[0x104 / 2], buffee_shadow[0x108 / 2], buffee_shadow[0x10A / 2],
              buffee_shadow[0x80 / 2], buffee_shadow[0x82 / 2], buffee_shadow[0x88 / 2], buffee_shadow[0x96 / 2], buffee_shadow[0xE0 / 2], buffee_shadow[0xE2 / 2]);
      uint32_t lc = ((uint32_t)buffee_shadow[0x84 / 2] << 16 | buffee_shadow[0x86 / 2]) & 0x1FFFFE;
      printf("COP2LC %06X; shadow colour0 %04X\n", (unsigned)lc, buffee_shadow[0x180 / 2]);
#ifdef WLOGALL
      { static uint32_t h[256]; uint32_t n = buffee_wlog_pos; for (uint32_t j = n > 4096 ? n - 4096 : 0; j < n; j++) h[(buffee_wlog[2 * (j & 4095)] & 0x1FE) >> 1]++;
        printf("CPU writes by custom register (last %u of %u):", (unsigned)(n > 4096 ? 4096 : n), (unsigned)n);
        for (int r = 0; r < 256; r++) if (h[r]) printf(" %03X:%u(last %04X)", r * 2, (unsigned)h[r], buffee_shadow[r]); printf("\n"); }
#endif
      dump_tasks();
      { uint8_t a = *(volatile uint8_t *)0xBFE001, b = *(volatile uint8_t *)0xBFD100; uint32_t idx = 0; (void)*(volatile uint8_t *)0xBFDD00; for (int i = 0; i < 60; i++) { uint32_t t0 = bbb_pmu_cycles(); while (bbb_pmu_cycles() - t0 < 10000000u) ; if (*(volatile uint8_t *)0xBFDD00 & 0x10) idx++; }
        printf("drive: CIAA PRA %02X (/RDY %d /TK0 %d /CHNG %d), CIAB PRB %02X (motor %s, sel0 %s, side %d, dir %d), index pulses in 0.6 s: %u, INTREQR %04X INTENAR %04X ADKCONR %04X DSKBYTR %04X\n", a, !!(a & 0x20), !!(a & 0x10), !!(a & 4), b, (b & 0x80) ? "off" : "ON", (b & 8) ? "no" : "YES", !(b & 4), !!(b & 2), (unsigned)idx, read_BE_word(0xDFF01E), read_BE_word(0xDFF01C), read_BE_word(0xDFF010), read_BE_word(0xDFF01A)); }
      { uint32_t t0 = bbb_pmu_cycles(), changes = 0, n = 0; uint16_t last = read_BE_word(0xDFF00A), first = last, lo = 0xFFFF, hi = 0; uint8_t lmb = 0; uint32_t small = 0, big = 0; int32_t sx = 0, sy = 0;
        printf("mouse (JOY0DAT) for 12 s - move it now: start %04X\n", last);
        while (bbb_pmu_cycles() - t0 < 12000u * 1000000u) { uint16_t v = read_BE_word(0xDFF00A); n++; if (v != last) { int dx = (int8_t)((v & 255) - (last & 255)), dy = (int8_t)((v >> 8) - (last >> 8)); if (dx >= -1 && dx <= 1 && dy >= -1 && dy <= 1) small++; else big++; sx += dx; sy += dy; changes++; last = v; } if (!(*(volatile uint8_t *)0xBFE001 & 0x40)) lmb = 1; if (v < lo) lo = v; if (v > hi) hi = v; }
        printf("mouse: %u reads, JOY0DAT changed %u times (first %04X last %04X, y range %02X-%02X, x range %02X-%02X), left button seen %d, JOY1DAT %04X, POTGOR %04X; steps of at most 1 count: %u, larger jumps: %u, net dx %d dy %d\n", (unsigned)n, (unsigned)changes, first, last, lo >> 8, hi >> 8, lo & 255, hi & 255, lmb, read_BE_word(0xDFF00C), read_BE_word(0xDFF016), (unsigned)small, (unsigned)big, (int)sx, (int)sy); }
      { uint32_t eb = rl(4), lib = rl(eb + 0x17A), gb = 0; printf("libs:"); { uint32_t q = lib; for (int k = 0; k < 30 && q && rl(q); k++, q = rl(q)) { uint32_t nm = rl(q + 10); printf(" %06X:%c%c%c%c%c%c", (unsigned)q, read_BE_word(nm) >> 8, read_BE_word(nm) & 255, read_BE_word(nm + 2) >> 8, read_BE_word(nm + 2) & 255, read_BE_word(nm + 4) >> 8, read_BE_word(nm + 4) & 255); } printf("\n"); } for (int k = 0; k < 40 && lib && rl(lib); k++, lib = rl(lib)) { uint32_t nm = rl(lib + 10); if (rl(nm) == 0x67726170 /* "grap" */ && rl(nm + 4) == 0x68696373) { gb = lib; break; } }
        uint32_t av = gb ? rl(gb + 34) : 0, lof = av ? rl(av + 4) : 0, st = lof ? rl(lof + 4) : 0;
        printf("ExecBase %06X GfxBase %06X ActiView %06X LOFCprList %06X start %06X DxOffset %d DyOffset %d Modes %04X\n", (unsigned)eb, (unsigned)gb, (unsigned)av, (unsigned)lof, (unsigned)st, av ? (int)(int16_t)read_BE_word(av + 14) : 0, av ? (int)(int16_t)read_BE_word(av + 12) : 0, av ? read_BE_word(av + 16) : 0);
        if (gb) { printf("GfxBase words $A0-$140:"); for (uint32_t k = 0xA0; k < 0x140; k += 2) printf(" %04X", read_BE_word(gb + k)); printf("\n"); }
        if (st) lc = st;
      if (gb) { uint32_t lib2 = rl(rl(4) + 0x17A); uint32_t ci = rl(gb + 0x26); printf("COPINIT %06X:", (unsigned)ci); for (int k = 0; k < 96; k++) printf(" %04X", read_BE_word(ci + 2 * k)); printf("\n");
        printf("GFXBASE $%06X words $20-$60:", (unsigned)gb); for (int k = 0x20; k < 0x60; k += 2) printf(" %04X", read_BE_word(gb + k)); printf("\n");
        { uint32_t sp0 = 0; for (int k = 0; k < 94; k++) if (read_BE_word(ci + 2 * k) == 0x0120) sp0 = ((uint32_t)read_BE_word(ci + 2 * k + 2) << 16 | read_BE_word(ci + 2 * k + 6)) & 0x1FFFFE;
          printf("SPRITE0 data @%06X:", (unsigned)sp0); for (int k = 0; k < 40; k++) printf(" %04X", read_BE_word(sp0 + 2 * k)); printf("\n"); }
        { uint32_t q = lib2; for (int k = 0; k < 30 && q && rl(q); k++, q = rl(q)) { uint32_t nm = rl(q + 10); if (rl(nm) == 0x696E7475 /* "intu" */) { printf("INTUITIONBASE @%06X words $30-$50:", (unsigned)q); for (int w = 0x30; w < 0x50; w += 2) printf(" %04X", read_BE_word(q + w)); printf("\n"); uint32_t scr = rl(q + 0x38); if (scr) { printf("ACTIVE SCREEN @%06X words $00-$20:", (unsigned)scr); for (int w = 0; w < 0x20; w += 2) printf(" %04X", read_BE_word(scr + w)); printf("\n"); } } } } }
      { uint32_t vp0 = av ? rl(av) : 0, ri = vp0 ? rl(vp0 + 36) : 0, bm = ri ? rl(ri + 4) : 0;
        if (bm) { uint32_t bpr = read_BE_word(bm), rows = read_BE_word(bm + 2), p1 = rl(bm + 8) & 0x1FFFFE; printf("BMINFO bpr %u rows %u plane1 %06X\n", (unsigned)bpr, (unsigned)rows, (unsigned)p1);
          for (uint32_t r = 0; r < rows; r++) { printf("BM %03u:", (unsigned)r); for (uint32_t k = 0; k < bpr; k += 2) printf("%04X", read_BE_word(p1 + r * bpr + k)); printf("\n"); } } }
        { uint32_t vp = av ? rl(av) : 0; for (int k = 0; k < 4 && vp; k++, vp = rl(vp)) { printf("ViewPort@%06X: Next %06X DWidth %d DHeight %d DxOffset %d DyOffset %d Modes %04X ExtModes %02X RasInfo %06X\n", (unsigned)vp, (unsigned)rl(vp), read_BE_word(vp + 24), read_BE_word(vp + 26), (int)(int16_t)read_BE_word(vp + 28), (int)(int16_t)read_BE_word(vp + 30), read_BE_word(vp + 32), (unsigned)(read_BE_word(vp + 34) & 0xFF), (unsigned)rl(vp + 36)); { uint32_t ri = rl(vp + 36), bm = ri ? rl(ri + 4) : 0; printf("  RasInfo@%06X BitMap@%06X\n", (unsigned)ri, (unsigned)bm); if (bm) printf("  RasInfo: BitMap@%06X BytesPerRow %u Rows %u Flags %02X Depth %u; RxOffset %d RyOffset %d; planes %06X %06X %06X\n", (unsigned)bm, read_BE_word(bm), read_BE_word(bm + 2), read_BE_word(bm + 4) >> 8, read_BE_word(bm + 4) & 255, (int)(int16_t)read_BE_word(ri + 8), (int)(int16_t)read_BE_word(ri + 10), (unsigned)rl(bm + 8), (unsigned)rl(bm + 12), (unsigned)rl(bm + 16)); } } } }
      printf("copper list at %06X:", (unsigned)lc);
      for (uint32_t i = 0; i < 400; i++) { uint32_t a = read_BE_word(lc + 4 * i), b = read_BE_word(lc + 4 * i + 2); if (a == 0xFFFF && b == 0xFFFE) { printf(" END"); break; } printf(" %03X=%04X", a, b); } }
    printf("\nDMACONR %04X INTENAR %04X INTREQR %04X VPOSR %04X\n", read_BE_word(0xDFF002), read_BE_word(0xDFF01C), read_BE_word(0xDFF01E), read_BE_word(0xDFF004));
}

int main(void)
{
    printf("=== Kickstart on PJIT ===\n");
    pjit_cache_init(BUFFEE_CACHE_TOP);
    cpu->trap_exit_mask = 0x8000;                    /* TRAP #15 stops the run: used for the ROM breakpoints below (patched into the DRAM copy of the ROM) */
    emit_opcode_table();
    cpu->dr_threshold = DRT;
#ifdef NOIRQ
    pjit_irq_enable(0);                              /* the guest never sees an interrupt: separates interrupt delivery from everything else */
#else
    pjit_irq_enable(1);
#endif
    uint32_t t0 = bbb_pmu_cycles();
    uint32_t sum = buffee_maprom(0x80000);
    const uint8_t *rom = (const uint8_t *)(BUFFEE_MAPROM_PHYS + 0x80000u);
    uint32_t ssp = __builtin_bswap32(*(const uint32_t *)rom), pc = __builtin_bswap32(*(const uint32_t *)(rom + 4));
    printf("ROM copied (%u ms, word sum %08X); version words %02X%02X %02X%02X; reset vectors SSP %08X PC %08X\n", (bbb_pmu_cycles() - t0) / 1000000, sum,
           rom[12], rom[13], rom[14], rom[15], ssp, pc);
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, ssp);
    cpu->ssp = cpu->usp = __builtin_bswap32(ssp);
    cpu->vbr = 0;
    pjit_set_sr(0x2700);
    cpu->irq_level = 0; cpu->irq_flag = 0;
    pjit_cache_flush();
    for (uint32_t a = 0xF80000; a < 0x1000000; a += 0x10000) { }
    static const uint32_t dumps[][2] = { { 0xF800C0, 0x60 }, { 0xF83600, 0x60 } };
    for (int d = 0; d < 2; d++) {
        printf("ROM %06X:", (unsigned)dumps[d][0]);
        for (uint32_t i = 0; i < dumps[d][1]; i++) printf(" %02X", rom[dumps[d][0] - 0xF80000 + i]);
        printf("\n");
    }
    /* breakpoints: a TRAP #15 over the first halfword of an instruction in the DRAM copy of the ROM. They change the ROM's checksum (which the Kickstart tests first), so one
       long near the end is adjusted (one's complement sum) to keep it 0xFFFFFFFF. A hit prints the registers and the last interpreted instructions and stops. */
#if defined(CLEAN) || defined(NOBP)
    static const uint32_t bps[] = { 0 };
#else
    static const uint32_t bps[] = { 0xF8293E, 0xFBD3F6, 0xF86B4C, 0xFBD400, 0xF835FE, 0xF80404, 0xF80408, 0 };
#endif
    uint8_t *romp = (uint8_t *)(BUFFEE_MAPROM_PHYS + 0x80000u);
    static uint16_t saved[8];
    for (int i = 0; bps[i]; i++) { uint8_t *bp = romp + (bps[i] - 0xF80000); saved[i] = bp[0] << 8 | bp[1]; bp[0] = 0x4E; bp[1] = 0x4F; }
    { uint32_t sum = 0; for (uint32_t o = 0; o < 0x80000; o += 4) sum = ec_add(sum, rd32(romp + o));
      uint32_t x = ec_add(rd32(romp + 0x7FFE8), ~sum); romp[0x7FFE8] = x >> 24; romp[0x7FFE9] = x >> 16; romp[0x7FFEA] = x >> 8; romp[0x7FFEB] = x;
      sum = 0; for (uint32_t o = 0; o < 0x80000; o += 4) sum = ec_add(sum, rd32(romp + o)); printf("ROM sum with breakpoints: %08X\n", sum); }
    CP15DCacheCleanFlushBuff((uint32_t)romp, 0x80000);
#if !defined(CLEAN) && !defined(NOTRACE)
#ifdef WLOGALL
    buffee_wlog_all = 1;
#endif
    buffee_trace_custom();
#endif
#ifdef PACE
    { const volatile uint8_t *bi = (const volatile uint8_t *)0x9F5FFF00u;
      unsigned pace = *(const volatile uint32_t *)bi == 0x544F4F42u ? bi[11] : PACE;     /* bootinfo byte 11 = the EEPROM's pace percent (0 = off); without bootinfo the build's PACE */
      extern void pjit_prefetch_pace(void);
      if (pace > 1000) pace = 1000;
      if (pace) {                                                       /* K = ARM cycles per 68000 cycle: 141.04 ns at 7.09 MHz x the CPU clock in MHz / 1000, scaled by the percent */
          pjit_pace_k = __builtin_bswap32(14104u * (PACE_MHZ) / 1000u / pace);
          cpu->prefetch_hook = (uint32_t)pjit_prefetch_pace;
          printf("[PACE] interpreted code (chip RAM, the Amiga window) paced to %u%% of an MC68000 at 7.09 MHz (CPU %u MHz)\n", pace, (unsigned)(PACE_MHZ)); }
      else printf("[PACE] off\n"); }
#endif
#ifdef WTRACE
    buffee_trace_custom();                                              /* every CPU write to the custom chips faults and is logged (buffee_wl) */
#endif
#if !defined(CLEAN) && defined(PCTRACE)
    cpu->prefetch_hook = (uint32_t)pjit_prefetch_trace;
#endif      /* ring of (PC, opcode) of the interpreter */
    printf("autoconfig space $E80000:"); for (uint32_t a = 0xE80000; a < 0xE80020; a += 2) printf(" %04X", read_BE_word(a)); printf("\n                  $E90000:"); for (uint32_t a = 0xE90000; a < 0xE90008; a += 2) printf(" %04X", read_BE_word(a)); printf("\n");
#ifdef SCRUB_EXEC
    /* test aid: a J-Link reset leaves the Amiga's chip RAM as it was, so Kickstart sees the previous boot's ExecBase and warm-starts with that boot's memory list; clear the ExecBase pointer at $4 so
       every boot is a cold start, as after a power cycle (overlay off for the write, back on before the guest starts: the reset vectors come from the ROM) */
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
    for (uint32_t a = 0; a < 0x200000; a += 2) write_BE_word(a, 0);                       /* all of chip RAM (2 MB window: a 1 MB machine mirrors it): 0.9 s */
    *(volatile uint8_t *)0xBFE001 |= 1;
#endif
#ifndef KEEP_EXEC
    /* a reset of Buffee (J-Link, ESC x3 and run again) leaves the Amiga's chip RAM as it was, so Kickstart would find the old ExecBase and warm-start with its memory list (and a region that
       was added last time). Make that ExecBase invalid, as a power cycle does: ChkBase (+$26) is checked against ~ExecBase, and $4 is cleared (overlay off for the writes, back on before the guest
       starts: the reset vectors come from the ROM). Cheap: three writes, no scrub. */
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;
    { uint32_t eb = read_BE_word(4) << 16 | read_BE_word(6);
      if (!(eb & 1) && eb >= 0x400 && eb < 0x200000 - 0x40) { write_BE_word(eb + 0x26, 0); write_BE_word(eb + 0x28, 0); printf("old ExecBase $%06X invalidated\n", (unsigned)eb); }
      write_BE_word(4, 0); write_BE_word(6, 0); }
    *(volatile uint8_t *)0xBFE001 |= 1;
#endif
    printf("starting at $%08X\n", pc);
#ifndef CLEAN
    sample_stop = SAMPLES; sample_verbose = SAMPV; buffee_sampler_start(500000000u);
#endif                /* a line every 0.5 s: where is the guest */
#ifdef FREEZEWATCH
    buffee_sampler_start(FREEZEWATCH);              /* the freeze watch's tick (and the PROF tally) */
#endif
#ifdef PROFILE
    buffee_sampler_start(PROFILE);                  /* a sample every PROFILE host cycles, tallied for the ESC report's PROF line */
#endif
#ifdef HANGWDT
    {   /* hang diagnosis: the SoC watchdog WDT1 resets the board HANGWDT seconds after the guest starts (never kicked); with -DCRUMBS the last translations / write faults are in the PRU shared RAM (buffee_crumbs prints them) */
        volatile uint32_t *cm = (volatile uint32_t *)0x44E004D4, *w = (volatile uint32_t *)0x44E35000;
        *cm = 2; for (uint32_t t = 0; ((*cm >> 16) & 3) && t < 1000000; t++) ;
#define WW() for (uint32_t t = 0; w[0x34 / 4] && t < 1000000; t++) ;
        WW(); w[0x24 / 4] = 0; WW(); w[0x2C / 4] = 0xFFFFFFFFu - 32768u * HANGWDT + 1; WW(); w[0x30 / 4] = 1; WW(); w[0x48 / 4] = 0xBBBB; WW(); w[0x48 / 4] = 0x4444; WW();
        volatile uint32_t *cr = (volatile uint32_t *)0x9F5FFC00u; for (int i = 0; i < 80; i++) cr[i] = 0; __asm__ volatile ("mcr p15, 0, %0, c7, c10, 1\n dsb sy" :: "r"(cr) : "memory");
    }
#endif
    uint32_t r;
    for (;;) {
        r = pjit_run((uint16_t *)pc);
        uint32_t hit = (uint32_t)cpu->pc - 2;
        if (r != 0x2F || (hit != 0xF8293E && hit != 0xFBD3F6 && hit != 0xF86B4C)) break;                        /* a one-shot, non-terminal breakpoint: put the instruction back and go on */
        if (hit == 0xF86B4C) { uint32_t n = buffee_wlog_pos; printf("custom register writes (last 70 of %u):", (unsigned)n); for (uint32_t j = n > 70 ? n - 70 : 0; j < n; j++) printf(" %03X=%X", (unsigned)(buffee_wlog[2 * (j & 4095)] & 0xFFF), (unsigned)buffee_wlog[2 * (j & 4095) + 1]); printf("\n"); }
        if (hit == 0xF86B4C) printf("  DMACONR %04X VPOSR %04X INTENAR %04X INTREQR %04X\n", read_BE_word(0xDFF002), read_BE_word(0xDFF004), read_BE_word(0xDFF01C), read_BE_word(0xDFF01E));
        if (hit == 0xF86B4C) { printf("  VHPOSR:"); for (int j = 0; j < 6; j++) { printf(" %04X", read_BE_word(0xDFF006)); for (volatile int k = 0; k < 20000; k++) ; } printf("; DMACONR over time:"); for (int j = 0; j < 5; j++) { printf(" %04X", read_BE_word(0xDFF002)); for (volatile int k = 0; k < 2000000; k++) ; } printf("\n"); }
        printf("[BP %06X] D0=%08X D1=%08X A0=%08X A6=%08X A7=%08X stack:", (unsigned)hit, pjit_get_dn(0), pjit_get_dn(1), pjit_get_an(0), pjit_get_an(6), pjit_get_an(7)); for (uint32_t a = pjit_get_an(7); a < pjit_get_an(7) + 24; a += 4) printf(" %08X", (unsigned)(read_BE_word(a) << 16 | read_BE_word(a + 2))); printf("\n");
        uint8_t *bp = romp + (hit - 0xF80000); int ki = hit == 0xF8293E ? 0 : hit == 0xFBD3F6 ? 1 : 2; bp[0] = saved[ki] >> 8; bp[1] = saved[ki]; CP15DCacheCleanFlushBuff((uint32_t)bp, 2); pjit_cache_flush(); pc = hit;
    }
    if (buffee_sample_stopped) {
        printf("sampler stop: last PC host %08X r4 %08X\n", (unsigned)buffee_last_pc, (unsigned)buffee_last_r4);
        printf("  DMACONR word %04X %04X %04X, bytes %02X %02X, VPOSR %04X VHPOSR %04X %04X, INTREQR %04X INTENAR %04X\n", read_BE_word(0xDFF002), read_BE_word(0xDFF002), read_BE_word(0xDFF002),
               *(volatile uint8_t *)0xDFF002, *(volatile uint8_t *)0xDFF003, read_BE_word(0xDFF004), read_BE_word(0xDFF006), read_BE_word(0xDFF006), read_BE_word(0xDFF01E), read_BE_word(0xDFF01C));
        /* blitter probe: starved of bus cycles by our polling, or a wedged sequencer (BSF latch set, BRF never comes)? */
        for (volatile int k = 0; k < 30000000; k++) ;                                                           /* ~100 ms with no chip bus access at all */
        printf("  after idle: DMACONR %04X\n", read_BE_word(0xDFF002));
        write_BE_word(0xDFF096, 0x8400);                                                                        /* BLTPRI (nasty): the blitter wins every contested cycle */
        for (volatile int k = 0; k < 3000000; k++) ;
        printf("  BLTPRI on:  DMACONR %04X\n", read_BE_word(0xDFF002));
        write_BE_word(0xDFF096, 0x0100); for (volatile int k = 0; k < 3000000; k++) ; printf("  BPLEN off:  DMACONR %04X\n", read_BE_word(0xDFF002)); write_BE_word(0xDFF096, 0x8100);
        write_BE_word(0xDFF096, 0x0040); for (volatile int k = 0; k < 3000000; k++) ; printf("  BLTEN off:  DMACONR %04X\n", read_BE_word(0xDFF002)); write_BE_word(0xDFF096, 0x8040); for (volatile int k = 0; k < 3000000; k++) ; printf("  BLTEN on:   DMACONR %04X\n", read_BE_word(0xDFF002));
        write_BE_word(0xDFF096, 0x0200); for (volatile int k = 0; k < 3000000; k++) ; printf("  DMAEN off:  DMACONR %04X\n", read_BE_word(0xDFF002)); write_BE_word(0xDFF096, 0x8200); for (volatile int k = 0; k < 3000000; k++) ; printf("  DMAEN on:   DMACONR %04X\n", read_BE_word(0xDFF002));
        write_BE_word(0xDFF040, 0); write_BE_word(0xDFF042, 0); write_BE_word(0xDFF058, 0x0041);                /* a benign restart: no channels, 1 line x 1 word */
        for (volatile int k = 0; k < 3000000; k++) ;
        printf("  restarted:  DMACONR %04X\n", read_BE_word(0xDFF002));
        { uint32_t n = buffee_wlog_pos, c = 0; printf("  blitter-register writes (of %u custom writes), last 48, oldest first:", (unsigned)n);
          for (uint32_t j = n > 4096 ? n - 4096 : 0; j < n; j++) { uint32_t a = buffee_wlog[2 * (j & 4095)] & 0xFFF; if ((a >= 0x040 && a < 0x060) || a == 0x096) c++; }
          uint32_t skip = c > 48 ? c - 48 : 0;
          for (uint32_t j = n > 4096 ? n - 4096 : 0; j < n; j++) { uint32_t w = buffee_wlog[2 * (j & 4095)], a = w & 0xFFF; if ((a >= 0x040 && a < 0x060) || a == 0x096) { if (skip) { skip--; continue; } printf(" %03X=%X/%u", (unsigned)a, (unsigned)buffee_wlog[2 * (j & 4095) + 1], (unsigned)(w >> 28)); } }
          printf("\n"); }
        dump_state(); return 1; }
    printf("pjit_run returned 0x%X at %06X; last instructions:", r, (unsigned)((uint32_t)cpu->pc - 2));
    { uint32_t pos = pjit_trace_pos / 8; for (int j = 80; j >= 1; j--) { uint32_t e = ((pos - j) & 0xFFFF) * 2; printf(" %06X:%04X", (unsigned)__builtin_bswap32(pjit_trace[e]) & 0xFFFFFF, (unsigned)__builtin_bswap32(pjit_trace[e + 1]) & 0xFFFF); } }
    printf("\n");
    if ((uint32_t)cpu->pc - 2 == 0xFBD400) { /* the reservation found a used byte: show the library base */
        printf("expansion base A6=%08X; A0=%08X D1=%04X; bytes from base+$58:\n", pjit_get_an(6), pjit_get_an(0), pjit_get_dn(1) & 0xFFFF);
        for (uint32_t a = pjit_get_an(6) + 0x50; a < pjit_get_an(6) + 0x110; a += 16) { printf("  %06X:", (unsigned)a); for (uint32_t k = 0; k < 16; k += 2) printf(" %04X", read_BE_word(a + k)); printf("\n"); }
    }
    printf("exceptions delivered (vector@PC, last 16):"); for (int j = 16; j >= 1; j--) { uint32_t e = ((pjit_dlog_pos / 8 - j) & 31) * 2; printf(" %u@%08X", (unsigned)pjit_dlog[e], (unsigned)pjit_dlog[e + 1]); } printf("\n");
    for (int i = 0; i < 8; i++) printf(" D%d=%08X", i, pjit_get_dn(i));
    printf("\n");
    for (int i = 0; i < 8; i++) printf(" A%d=%08X", i, pjit_get_an(i));
    printf("\n stack:");
    for (uint32_t a = pjit_get_an(7); a < pjit_get_an(7) + 48; a += 4) printf(" %08X", (unsigned)(read_BE_word(a) << 16 | read_BE_word(a + 2)));
    printf("\nlow memory:");
    for (uint32_t a = 0x100; a < 0x110; a += 4) printf(" %08X", (unsigned)(read_BE_word(a) << 16 | read_BE_word(a + 2))); printf("  ["); for (uint32_t a = 0; a < 0x20; a += 4) printf(" %08X", (unsigned)(read_BE_word(a) << 16 | read_BE_word(a + 2)));
    printf("\n");
    return r == BUFFEE_RESET_EXIT ? BUFFEE_RESET_EXIT : 1;
}
