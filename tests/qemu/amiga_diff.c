/// @file amiga_diff.c
/// @brief PJIT's interpreter running a Kickstart ROM on the deterministic Amiga model (tools/amiga/amiga.c), in QEMU, for comparison with the golden core
///        (tools/amiga/oracle_host.c, same model, same snapshots). Guest memory: chip RAM 1 MB (mirrored at $100000), the ROM at $F80000 (and over the first 512 KB while
///        the overlay is on, read-only: writes land in the chip RAM underneath), everything else from $200000 to $F7FFFF is the model's (no-access sections: every access
///        faults and is emulated by amiga_abort()). One snapshot every SNAP instructions: tools/amiga/compare.py finds the first that differs.
///        qemu-system-arm -M virt -cpu cortex-a15 -m 512M -nographic -semihosting -kernel build/amiga_diff.elf -device loader,file=Kickstart3.1.rom,addr=0x50000000,force-raw=on
#include <stdint.h>
#include "pjit.h"
#include "cp15.h"
#include "mem_be.h"
#include "amiga.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);
extern void pjit_prefetch_diff(void);

#define ROM_PHYS   0x50000000u
#define CHIP_PHYS  0x51000000u
#define ROMSEC_PHYS 0x52000000u
#define CHIP_SIZE  0x100000u
#ifndef LIMIT
#define LIMIT      20000000ull

#endif
#ifndef SNAP
#define SNAP       100000ull
#endif

volatile uint32_t diff_state[2];                      /* BIG-endian: instructions completed, the deadline (tests/qemu/amiga_hook.S) */
static am_t am;
#ifndef DENSE_FROM
#define DENSE_FROM 0ull
#endif
static uint64_t next_snap = SNAP, model_deadline;
static uint64_t step_for(uint64_t at) { return (DENSE_FROM && at >= DENSE_FROM) ? 1 : SNAP; }
static int overlay_state = -1;
static uint32_t l1[4096] __attribute__((aligned(16384)));
static uint32_t l2[256] __attribute__((aligned(1024)));

static inline uint32_t cnt_get(void) { return __builtin_bswap32(diff_state[0]); }
static inline void cnt_set(uint32_t v) { diff_state[0] = __builtin_bswap32(v); }
static inline void deadline_set(uint32_t v) { diff_state[1] = __builtin_bswap32(v); }

#define SEC_RAM   0xC0Eu                              /* AP=11, B, C */
#define SEC_ROM   0x8C0Eu                             /* read-only */
#define SEC_NONE  0x2u                                /* no access */
static void map_overlay(int on)
{
    for (uint32_t p = 0; p < 256; p++)
        l2[p] = (on && p < 0x80) ? ((ROM_PHYS + p * 4096) | 0x23E) : ((CHIP_PHYS + p * 4096) | 0x3E);   /* small page: AP=11 (+AP[2]=1 for read-only), B, C */
    CP15DCacheCleanFlushBuff((uint32_t)l2, sizeof l2);
    CP15TlbInvalidate();
    overlay_state = on;
}
static void mmu_setup(void)
{
    for (uint32_t i = 0; i < 4096; i++) l1[i] = (i << 20) | SEC_RAM;                  /* the harness itself and QEMU's devices: identity */
    for (uint32_t s = 0; s < 0x20; s++) {                                            /* the 24-bit address space, mirrored: a 68000 has no A24-A31 */
        uint32_t g = s & 0xF;
        if (g == 0) l1[s] = ((uint32_t)l2 & 0xFFFFFC00u) | 0x1;                      /* a coarse table: the overlay maps 128 pages of ROM */
        else if (g == 1) l1[s] = CHIP_PHYS | SEC_RAM;                                /* chip RAM mirrored at $100000 */
        else if (g == 0xF) l1[s] = ROMSEC_PHYS | SEC_ROM;
        else l1[s] = SEC_NONE;                                                       /* the model's: every access faults */
    }
    map_overlay(1);
    CP15DCacheCleanFlushBuff((uint32_t)l1, sizeof l1);
    CP15TtbCtlTtb0Config(); CP15Ttb0Set((uint32_t)l1); CP15DomainAccessClientSet(); CP15TlbInvalidate(); CP15MMUEnable();
}

/* ---- the abort handler: model accesses and writes into read-only pages ---- */
static inline uint32_t gr(uint32_t *f, unsigned n) { return n < 13 ? f[2 + n] : n == 13 ? f[0] : f[1]; }
static inline void sr_(uint32_t *f, unsigned n, uint32_t v) { if (n < 13) f[2 + n] = v; else if (n == 13) f[0] = v; else f[1] = v; }
static void chip_store(uint32_t a, int size, uint32_t v)
{
    uint8_t *c = (uint8_t *)CHIP_PHYS;
    for (int i = 0; i < size; i++) c[(a + i) & (CHIP_SIZE - 1)] = (uint8_t)(v >> (8 * (size - 1 - i)));
}
#ifndef TRACE_FROM
#define TRACE_FROM 0xFFFFFFFFu
#define TRACE_TO 0
#endif
static void hw_store(uint32_t a, int size, uint32_t v)
{
    uint32_t t = cnt_get();
    if (t >= TRACE_FROM && t <= TRACE_TO) printf("%u W%d %06X <- %X\n", (unsigned)t, size, (unsigned)a, (unsigned)v);
    if (size == 4) { am_write(&am, a, 2, v >> 16, t); am_write(&am, a + 2, 2, v & 0xFFFF, t); } else am_write(&am, a, size, v, t);
    if (am_overlay(&am) != overlay_state) map_overlay(am_overlay(&am));
    deadline_set(t);                                      /* the interrupt level may have changed: look at the next boundary */
}
static uint32_t hw_load(uint32_t a, int size)
{
    uint32_t t = cnt_get();
    if (t >= TRACE_FROM && t <= TRACE_TO) printf("%u R%d %06X\n", (unsigned)t, size, (unsigned)a);
    if (size == 4) { uint32_t hi = am_read(&am, a, 2, t), lo = am_read(&am, a + 2, 2, t); return hi << 16 | lo; }
    return am_read(&am, a, size, t);
}
int amiga_abort(uint32_t *f, uint32_t spsr, uint32_t dfsr, uint32_t dfar)
{
    uint32_t st = dfsr & 0x40F;
    if (st != 0xD && st != 0xF) return 0;
    uint32_t ins = *(volatile uint32_t *)(f[15] - 8);
    uint32_t n = spsr >> 31, z = spsr >> 30 & 1, c = spsr >> 29 & 1, v = spsr >> 28 & 1; int ok;
    switch (ins >> 28) {
    case 0: ok = z; break; case 1: ok = !z; break; case 2: ok = c; break; case 3: ok = !c; break; case 4: ok = n; break; case 5: ok = !n; break;
    case 6: ok = v; break; case 7: ok = !v; break; case 8: ok = c && !z; break; case 9: ok = !c || z; break; case 10: ok = n == v; break; case 11: ok = n != v; break;
    case 12: ok = !z && n == v; break; case 13: ok = z || n != v; break; default: ok = 1; break;
    }
    unsigned rn = ins >> 16 & 15, rt = ins >> 12 & 15; int load = (ins >> 20) & 1, size, sgn = 0; uint32_t off;
    if (((ins >> 26) & 3) == 1) {                                          /* LDR / STR / LDRB / STRB */
        size = (ins & (1u << 22)) ? 1 : 4;
        if (ins & (1u << 25)) {
            uint32_t m = gr(f, ins & 15), sh = ins >> 7 & 31;
            switch (ins >> 5 & 3) { case 0: off = m << sh; break; case 1: off = sh ? m >> sh : 0; break; case 2: off = sh ? (uint32_t)((int32_t)m >> sh) : (uint32_t)((int32_t)m >> 31); break; default: return 0; }
        } else off = ins & 0xFFF;
    } else if (((ins >> 25) & 7) == 0 && (ins & 0x90) == 0x90 && (ins & 0x60)) {   /* halfword / signed byte / signed halfword */
        unsigned sh = ins >> 5 & 3; size = sh == 2 ? 1 : 2; sgn = sh != 1;
        off = (ins & (1u << 22)) ? ((ins >> 4 & 0xF0) | (ins & 0xF)) : gr(f, ins & 15);
    } else return 0;
    if (rn == 15 || rt == 15) return 0;
    if (!ok) return 1;
    uint32_t base = gr(f, rn), nb = (ins & (1u << 23)) ? base + off : base - off;
    int pre = (ins & (1u << 24)) != 0, wb = !pre || (ins & (1u << 21));
    uint32_t a = pre ? nb : base;
    if (dfar - a > 3) return 0;
    uint32_t g = a & 0xFFFFFF;
    if (load) {
        uint32_t val = hw_load(g, size);
        if (sgn) val = size == 1 ? (uint32_t)(int32_t)(int8_t)val : (uint32_t)(int32_t)(int16_t)val;
        sr_(f, rt, val);
    } else {
        uint32_t val = gr(f, rt); if (size == 1) val &= 0xFF; else if (size == 2) val &= 0xFFFF;
        if (g < 0x80000 && overlay_state) chip_store(g, size, val);               /* a write under the overlay lands in the chip RAM */
        else if ((g >> 20) == 0xF) { }                                           /* ROM: ignored */
        else hw_store(g, size, val);
    }
    if (wb) sr_(f, rn, nb);
    return 1;
}

/* ---- the machine ---- */
static void snapshot(void)
{
    printf("S %u %08x%08x %06X", (unsigned)cnt_get(), (unsigned)(am.hash >> 32), (unsigned)am.hash, (unsigned)(uint32_t)cpu->pc & 0xFFFFFF);
    for (int i = 0; i < 8; i++) printf(" %08X", (unsigned)pjit_get_dn(i));
    for (int i = 0; i < 8; i++) printf(" %08X", (unsigned)pjit_get_an(i));
    printf(" %04X", (unsigned)(pjit_get_sr() & 0xA71F));
    {   uint32_t sp = pjit_get_an(7) & 0xFFFFE;                                               /* the 16 bytes at the stack pointer */
        for (int i = 0; i < 8; i++) { const uint8_t *c = (const uint8_t *)CHIP_PHYS; uint32_t a = (sp + 2 * i) & 0xFFFFF; printf(" %02X%02X", c[a], c[(a + 1) & 0xFFFFF]); } }
    printf("\n");
}
static void deliver_irq(int level)
{
    uint16_t sr = pjit_get_sr(); uint32_t pc = (uint32_t)cpu->pc, a7 = pjit_get_an(7);
    if (!(sr & 0x2000)) { cpu->usp = __builtin_bswap32(a7); a7 = __builtin_bswap32(cpu->ssp); }
    a7 -= 2; write_BE_word(a7, pc & 0xFFFF); a7 -= 2; write_BE_word(a7, pc >> 16); a7 -= 2; write_BE_word(a7, sr);
    pjit_set_an(7, a7);
    pjit_set_sr((uint16_t)(((sr | 0x2000) & ~0x8000 & ~0x0700) | (level << 8)));
    cpu->pc = (uint16_t *)(uintptr_t)(((uint32_t)read_BE_word((24 + level) * 4) << 16) | read_BE_word((24 + level) * 4 + 2));
}
static void set_deadline(uint32_t c)
{
    uint64_t d = am_deadline(&am);
    if (d <= c) d = c + 1;
    if (d > next_snap) d = next_snap;
    if (am_ipl(&am)) d = c + 1;                                /* an interrupt level is up: look at every boundary (the resumed instruction runs first) */
    deadline_set((uint32_t)d);
}

int main(void)
{
    uint8_t *rom = (uint8_t *)ROM_PHYS;
    pjit_cache_init(0x5F000000u);
    cpu->trap_exit_mask = 0;
    emit_opcode_table();
    cpu->dr_threshold = 0xFFFFFFFFu;                          /* the interpreter: the hook runs before every instruction */
    pjit_irq_enable(0);
    cpu->prefetch_hook = (uint32_t)pjit_prefetch_diff;
    {   uint8_t *romsec = (uint8_t *)ROMSEC_PHYS;
        for (uint32_t i = 0; i < 0x80000; i++) romsec[i] = 0xFF;
        for (uint32_t i = 0; i < 0x80000; i++) romsec[0x80000 + i] = rom[i]; }
    for (uint32_t i = 0; i < CHIP_SIZE; i++) ((uint8_t *)CHIP_PHYS)[i] = 0;
    CP15DCacheCleanFlushBuff(ROMSEC_PHYS, 0x100000); CP15DCacheCleanFlushBuff(CHIP_PHYS, 0x100000);
    am_init(&am, rom);
    mmu_setup();
    printf("=== PJIT interpreter on the Amiga model ===\n");
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    uint32_t ssp = ((uint32_t)rom[0] << 24) | rom[1] << 16 | rom[2] << 8 | rom[3], pc = ((uint32_t)rom[4] << 24) | rom[5] << 16 | rom[6] << 8 | rom[7];
    pjit_set_an(7, ssp); cpu->ssp = cpu->usp = __builtin_bswap32(ssp);
    pjit_set_sr(0x2700); cpu->irq_level = 0; cpu->irq_flag = 0; cpu->vbr = 0;
    cnt_set(0); set_deadline(0);
    for (;;) {
        uint32_t r = pjit_run((uint16_t *)pc);
        uint32_t c;
        if (r == 0x100) {                                      /* STOP: let model time pass until a level above the mask is up (as the oracle's GetIPLx does) */
            c = cnt_get(); pc = (uint32_t)cpu->pc;
            for (;;) {
                uint64_t d = am_deadline(&am);
                if (d <= c) d = c + 1;
                if (d > next_snap) d = next_snap;
                c = (uint32_t)d; cnt_set(c);
                if (c >= LIMIT) break;
                am_events(&am, c);
                if (c == next_snap) { snapshot(); next_snap += step_for(next_snap); }
                {   int ipl = am_ipl(&am), mask = (pjit_get_sr() >> 8) & 7;
                    if (ipl > mask || ipl == 7) break; }
            }
        } else if (r != 0x104) { printf("pjit_run returned 0x%X at %08X\n", (unsigned)r, (unsigned)(uint32_t)cpu->pc); snapshot(); break; }
        else { c = cnt_get(); pc = (uint32_t)cpu->pc; }
        if (c >= LIMIT) { snapshot(); break; }
        if (c >= model_deadline || c >= am_deadline(&am)) { am_events(&am, c); }
        if (c == next_snap) { snapshot(); next_snap += step_for(next_snap); }
        {   int ipl = am_ipl(&am), mask = (pjit_get_sr() >> 8) & 7;
            if (ipl > mask || ipl == 7) { cpu->pc = (uint16_t *)(uintptr_t)pc; deliver_irq(ipl); pc = (uint32_t)cpu->pc; } }
        set_deadline(c);                                          /* the hook counts the resumed instruction itself */
    }
    printf("amiga_diff: %u hardware accesses\n", (unsigned)am.n_access);
    qemu_exit();
    return 0;
}
