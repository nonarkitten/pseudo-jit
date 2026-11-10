/* oracle_host.c -- runs a Kickstart ROM on golden/mcl68k.c (Ted Fried's 68000 core, an independent implementation) against the deterministic Amiga model
 * (amiga.c). Prints a snapshot (instruction count, hash of the hardware writes, PC, D0-D7, A0-A7, SR) every SNAP instructions; tests/qemu/amiga_diff.c does the
 * same on PJIT in QEMU and tools/amiga/compare.py finds the first snapshot that differs.
 *   oracle_host <rom> [instructions] [snap]  > oracle.log         (Kickstart3.1.rom 40.063: instructions default 20000000, snap 4096) */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#include "amiga.h"

extern uint32_t m68k_data_reg[8], m68k_address_reg[8], m68k_a7_S, mc68k_pc, memory_base;
extern uint16_t mc68k_flags;
extern void (*WriteWord)(uint32_t, uint16_t); extern uint16_t (*ReadWord)(uint32_t);
extern void (*WriteByte)(uint32_t, uint8_t); extern uint8_t (*ReadByte)(uint32_t);
extern void (*mcl68k_step_hook)(void); extern uint16_t (*mcl68k_iack_vector)(void);
extern int16_t run_mcl68k(void *start);
uint16_t GetIPLx(void);
int UART0Ready(void) { return 0; } uint8_t UART0Read(void) { return 0; } void UART0Write(uint8_t c) { (void)c; }

#define CHIP 0x100000u
static uint8_t chip[CHIP], rom[0x80000];
static am_t am;
static uint64_t icount, deadline, snap = 4096, limit = 20000000, next_snap, dense_from;
static uint64_t step_for(uint64_t at) { return (dense_from && at >= dense_from) ? 1 : snap; }
static int cur_ipl;
static jmp_buf jb;
static int trace;

static uint8_t rb(uint32_t a)
{
    a &= 0xFFFFFF;
    if (a < 0x200000) { if (a < 0x80000 && am_overlay(&am)) return rom[a]; return chip[a & (CHIP - 1)]; }
    if (a >= 0xF80000) return rom[a - 0xF80000];
    if (a >= 0xF00000) return 0xFF;
    { uint8_t v = (uint8_t)am_read(&am, a, 1, icount); if (trace) fprintf(stderr, "%llu R1 %06X = %02X\n", (unsigned long long)icount, a, v); return v; }
}
static void wb(uint32_t a, uint8_t v)
{
    a &= 0xFFFFFF;
    if (a < 0x200000) { chip[a & (CHIP - 1)] = v; return; }
    if (a >= 0xF00000) return;
    if (trace) fprintf(stderr, "%llu W1 %06X <- %02X\n", (unsigned long long)icount, a, v);
    am_write(&am, a, 1, v, icount);
}
static uint16_t rw(uint32_t a)
{
    a &= 0xFFFFFF;
    if (a < 0x200000 || a >= 0xF00000) return (uint16_t)(rb(a) << 8 | rb(a + 1));
    { uint16_t v = (uint16_t)am_read(&am, a, 2, icount); if (trace) fprintf(stderr, "%llu R2 %06X = %04X\n", (unsigned long long)icount, a, v); return v; }
}
static void ww(uint32_t a, uint16_t v)
{
    a &= 0xFFFFFF;
    if (a < 0x200000 || a >= 0xF00000) { wb(a, v >> 8); wb(a + 1, (uint8_t)v); return; }
    if (trace) fprintf(stderr, "%llu W2 %06X <- %04X\n", (unsigned long long)icount, a, v);
    am_write(&am, a, 2, v, icount);
}

static void snapshot(void)
{
    printf("S %u %08x%08x %06X", (unsigned)icount, (unsigned)(am.hash >> 32), (unsigned)am.hash, mc68k_pc & 0xFFFFFF);
    for (int i = 0; i < 8; i++) printf(" %08X", m68k_data_reg[i]);
    for (int i = 0; i < 7; i++) printf(" %08X", m68k_address_reg[i]);
    printf(" %08X", (mc68k_flags & 0x2000) ? m68k_a7_S : m68k_address_reg[7]);                /* the active stack pointer */
    printf(" %04X", mc68k_flags & 0xA71F);
    { uint32_t sp = ((mc68k_flags & 0x2000) ? m68k_a7_S : m68k_address_reg[7]) & (CHIP - 1) & ~1u;                      /* the 16 bytes at the stack pointer */
      for (int i = 0; i < 8; i++) { uint32_t a = (sp + 2 * i) & (CHIP - 1); printf(" %02X%02X", chip[a], chip[(a + 1) & (CHIP - 1)]); } }
    printf("\n");
}
static void recompute(void)
{
    uint64_t d = am_deadline(&am);
    if (d <= icount) d = icount + 1;
    deadline = d < next_snap ? d : next_snap;
    cur_ipl = am_ipl(&am);
}
static int stop_spin;
static void hook(void)
{
    stop_spin = 0;
    { static int w; if (!w && icount > 10 && getenv("WATCH_SSP") && m68k_a7_S > 0x100000) { w = 1; fprintf(stderr, "SSP %08X at icount %llu PC %06X flags %04X\n", m68k_a7_S, (unsigned long long)icount, mc68k_pc, mc68k_flags); } }
    if (icount >= limit) longjmp(jb, 1);
    if (icount >= deadline || cur_ipl != am_ipl(&am)) {
        am_events(&am, icount);
        if (icount == next_snap) { snapshot(); next_snap += step_for(next_snap); }
        recompute();
    }
    icount++;
}
uint16_t GetIPLx(void)
{
    /* the BIU's masked interrupt: a level above the SR's mask (7 is not maskable), in bits 10:8 */
    int mask = (mc68k_flags >> 8) & 7, l = am_ipl(&am);
    uint16_t r = (l > mask || l == 7) ? (uint16_t)(l << 8) : 0;
    if (!r && ++stop_spin > 100000) {                      /* a STOP spins here without executing anything: let time pass */
        stop_spin = 0; if (icount < deadline) icount = deadline; am_events(&am, icount);
        if (icount == next_snap) { snapshot(); next_snap += step_for(next_snap); }
        recompute();
        if (icount >= limit) longjmp(jb, 1);
    }
    return r;
}
static uint16_t iack(void) { return (uint16_t)(24 + am_ipl(&am)); }

int main(int argc, char **argv)
{
    if (argc < 2) return 2;
    trace = getenv("AM_TRACE") != 0;
    FILE *f = fopen(argv[1], "rb"); if (!f || fread(rom, 1, sizeof rom, f) != sizeof rom) { fprintf(stderr, "ROM?\n"); return 1; }
    if (argc > 2) limit = strtoull(argv[2], 0, 0);
    if (argc > 3) snap = strtoull(argv[3], 0, 0);
    if (argc > 4) dense_from = strtoull(argv[4], 0, 0);
    memset(chip, 0, sizeof chip);
    am_init(&am, rom); next_snap = snap; recompute();
    memory_base = 0; ReadByte = rb; WriteByte = wb; ReadWord = rw; WriteWord = ww;
    mcl68k_step_hook = hook; mcl68k_iack_vector = iack;
    if (setjmp(jb) == 0) run_mcl68k(0);
    snapshot();
    if (getenv("AM_CHIP")) { FILE *o = fopen(getenv("AM_CHIP"), "wb"); if (o) { fwrite(chip, 1, sizeof chip, o); fclose(o); } }
    if (getenv("AM_DUMP")) { uint32_t a = strtoul(getenv("AM_DUMP"), 0, 16); for (uint32_t r = 0; r < 8; r++) { fprintf(stderr, "%06X:", a + 16 * r); for (int k = 0; k < 16; k++) fprintf(stderr, " %02X", chip[(a + 16 * r + k) & (CHIP - 1)]); fprintf(stderr, "\n"); } }
    fprintf(stderr, "oracle: %llu instructions, %llu hardware accesses\n", (unsigned long long)icount, (unsigned long long)am.n_access);
    return 0;
}
