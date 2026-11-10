/// @file smoke_kcmp.c
/// @brief How far is PJIT's output from an ideal translation? The kernels of tests/kcmp/kernels.c (strlen, strcpy, memcpy, CRC, sort ...) are compiled twice from the same C: for the
///        68000 (tools/data/kcmp.py -> tests/data/kcmp_data.h, run through PJIT) and for the ARM (native, here). Counts executed ARM instructions per run (QEMU `-icount shift=0`: build with
///        EXTRA_CFLAGS=-DICOUNT) for: the native build, the interpreter, and the tiered / cached runtime once warm (150 runs, so hot loops are in blocks). The checksums must agree.
///        Run: make VARIANT=kcmp EXTRA_CFLAGS=-DICOUNT qemu-smoke_kcmp; qemu-system-arm -M virt -cpu cortex-a15 -m 128M -nographic -semihosting -icount shift=0,sleep=off -kernel build/kcmp/qemu/smoke_kcmp.elf
#include <stdint.h>
#include "pjit_libc.h"
#include "pjit.h"
#include "mem_be.h"
#include "cp15.h"
#include "kcmp_data.h"
#include "kcmp/kernels.c"

extern void qemu_exit(void);
extern void emit_opcode_table(void);
extern uint32_t read_cntpct_lo(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t low_mem[1024 * 1024] __attribute__((aligned(1024 * 1024)));
static uint32_t mmu_pagetable[4096] __attribute__((aligned(16384)));
static void map_low(void)
{
    for (uint32_t i = 0; i < 4096; i++) mmu_pagetable[i] = ((i << 20) & 0xFFF00000u) | 0xC0Eu;
    mmu_pagetable[0] = ((uint32_t)low_mem & 0xFFF00000u) | 0xC0Eu;
    CP15TtbCtlTtb0Config(); CP15Ttb0Set((uint32_t)mmu_pagetable); CP15DomainAccessClientSet(); CP15TlbInvalidate(); CP15MMUEnable();
}
#define ORG   0x1000u
#define STACK 0xF7FFCu
static void put_be32(uint32_t a, uint32_t v) { low_mem[a] = v >> 24; low_mem[a + 1] = v >> 16; low_mem[a + 2] = v >> 8; low_mem[a + 3] = (uint8_t)v; }

static uint8_t nat_d[0x1000] __attribute__((aligned(8))), nat_s[0x1000] __attribute__((aligned(8)));     /* the native run's dst / first array and src / second array */
static uint32_t rnd(uint32_t i) { uint32_t x = i * 2654435761u + 12345u; x ^= x >> 15; x *= 2246822519u; x ^= x >> 13; return x; }

/// the same data for both runs: bytes = a string of 255 letters + NUL; halfwords / longs = pseudo-random values (stored big-endian for the 68000, native for the ARM)
#ifndef STRL
#define STRL 255
#endif
static void fill(const kcmp_t *k)
{
    uint32_t sa = 0x30000, sb = k->kind == 'B' ? 0x20000 : 0x28000;
    if (k->kind == 'B') {
        for (uint32_t i = 0; i < STRL; i++) { uint8_t c = 'A' + (rnd(i) >> 8) % 26; low_mem[sa + i] = low_mem[sb + i] = nat_d[i] = nat_s[i] = c; }
        low_mem[sa + STRL] = low_mem[sb + STRL] = nat_d[STRL] = nat_s[STRL] = 0;
    } else if (k->kind == 'W') {
        for (uint32_t i = 0; i < 256; i++) {
            uint16_t va = rnd(i) >> 4, vb = rnd(i + 1000) >> 4;
            low_mem[sa + 2 * i] = va >> 8; low_mem[sa + 2 * i + 1] = (uint8_t)va; low_mem[sb + 2 * i] = vb >> 8; low_mem[sb + 2 * i + 1] = (uint8_t)vb;
            ((uint16_t *)nat_d)[i] = va; ((uint16_t *)nat_s)[i] = vb;
        }
    } else {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t va = rnd(i), vb = rnd(i + 1000);
            put_be32(sa + 4 * i, va); put_be32(sb + 4 * i, vb);
            ((uint32_t *)nat_d)[i] = va; ((uint32_t *)nat_s)[i] = vb;
        }
    }
}
static void load(const kcmp_t *k)
{
    memcpy(low_mem + ORG, k->img, k->size);
    memset(low_mem + ORG + k->size, 0, 0x1000);
    low_mem[0x100] = 0x4E; low_mem[0x101] = 0x4F;                 // TRAP #15: where the entry's RTS lands
    put_be32(STACK, 0x100);
    put_be32(0x10000, 0);
    fill(k);
}
static uint32_t run(uint32_t threshold)
{
    cpu->dr_threshold = threshold;
    cpu->trap_exit_mask = 0xFFFF;
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, STACK);
    pjit_set_sr(0x2700);
    return pjit_run((uint16_t *)ORG);
}
static uint32_t result68(void) { return (uint32_t)low_mem[0x10000] << 24 | low_mem[0x10001] << 16 | low_mem[0x10002] << 8 | low_mem[0x10003]; }

typedef u32 (*kfn_t)(void *, void *, u32);
#define TAB(name, fn, n, kind) { #name, (kfn_t)fn },
static const struct { const char *name; kfn_t fn; } natives[] = { KERNELS(TAB) };

int main(void)
{
    printf("=== PJIT vs native ARM, same C (%d kernels) ===\n", KCMP_N);
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();
    map_low();

#ifdef PROBE
    {   /* jsr $1010 ; move.l d0,$10000 ; rts ; (1010:) moveq #0,d0 ; tst.b $30000 ; beq.s L ; moveq #7,d0 ; rts ; (m NOPs) ; L: moveq #9,d0 ; rts   (the byte is non-zero: want 7) */
        for (int variant = 1; variant <= 3; variant += 2)
        for (int m = 0; m <= 0; m += 4) {
            static uint8_t img[128]; unsigned n = 0;
            memset(img, 0, sizeof img);
            #define B(...) do { const uint8_t t[] = { __VA_ARGS__ }; memcpy(img + n, t, sizeof t); n += sizeof t; } while (0)
            B(0x4E,0xB9,0x00,0x00,0x10,0x10); B(0x23,0xC0,0x00,0x01,0x00,0x00); B(0x4E,0x75);
            n = 0x10;
            if (variant == 0) { B(0x70,0x00); B(0x4A,0x39,0x00,0x03,0x00,0x00); }           /* moveq ; tst.b abs */
            if (variant == 1) { B(0x4A,0x39,0x00,0x03,0x00,0x00); }                        /* tst.b abs first (as gcc's strlen) */
            if (variant == 3) { B(0x4E,0x71); B(0x4A,0x39,0x00,0x03,0x00,0x00); }           /* nop ; tst.b abs */
            if (variant == 4) { B(0x4A,0xB9,0x00,0x03,0x00,0x00); }                        /* tst.l abs (bytes 0x56..: non-zero) */
            if (variant == 5) { B(0x12,0x39,0x00,0x03,0x00,0x00); }                        /* move.b abs,d1 */
            if (variant == 6) { B(0x0C,0x39,0x00,0x00,0x00,0x03,0x00,0x00); }              /* cmpi.b #0,abs */
            if (variant == 7) { B(0x4A,0x38,0x10,0x00); }                                  /* tst.b $1000.w (the first opcode byte: non-zero) */
            if (variant == 2) { B(0x70,0x00); B(0x20,0x7C,0x00,0x03,0x00,0x00); B(0x4A,0x10); }   /* moveq ; movea.l #$30000,a0 ; tst.b (a0) */
            uint8_t disp = 2 + 2 + 2 * m; if (variant == 4) { /* tst.l abs: long at $30000 is non-zero */ } B(0x67, disp);
            B(0x70,0x07); B(0x4E,0x75);
            for (int i = 0; i < m; i++) B(0x4E,0x71);
            B(0x70,0x09); B(0x4E,0x75);
            kcmp_t p = { "probe", img, n, 0, 'B' };
            uint32_t r[2];
            for (int mode = 0; mode < 2; mode++) { pjit_cache_flush(); load(&p); run(mode ? 0u : 0xFFFFFFFFu); r[mode] = result68(); }
            { const uint32_t *c = cpu->cache_data; for (int w = 0; w < 96; w++) printf("SLOT v%d %04X.%d %08X\n", variant, 0x1000 + (w / 4) * 2, w % 4, (unsigned)c[(0x1000 * 8 / 4) + w]); }
            printf("[PROBE] variant %d nops %2d: interp %u cached %u %s\n", variant, m, (unsigned)r[0], (unsigned)r[1], r[0] == r[1] ? "" : "<-- DIFFERS");
        }
    }
#endif
    int failures = 0;
    printf("[KCMP] kernel       n   68K bytes   native  interp   tiered   tiered/native  (ARM instructions per run)\n");
    for (int i = 0; i < KCMP_N; i++) {
        const kcmp_t *k = &kcmp[i];
#ifdef ONLY
        if (i != ONLY) continue;
#endif
        uint32_t nat = natives[i].fn(nat_d, nat_s, k->n);          /* (placeholder call: buffers are filled below) */
        (void)nat;
        pjit_cache_flush();
        load(k);
        nat = natives[i].fn(nat_d, nat_s, k->n);
        run(0xFFFFFFFFu);
        uint32_t r_int = result68();
        pjit_cache_flush();
        load(k); run(0u);
        uint32_t r_jit = result68();
#ifdef DBG
        printf("[DBG] cached: d0 %08X a0 %08X a7 %08X sr %04X result %08X\n", (unsigned)pjit_get_dn(0), (unsigned)pjit_get_an(0), (unsigned)pjit_get_an(7), (unsigned)pjit_get_sr(), (unsigned)r_jit);
#endif
        if (r_int != nat || r_jit != nat) { printf("[KCMP] %-10s MISMATCH native %08X interp %08X cached %08X\n", k->name, (unsigned)nat, (unsigned)r_int, (unsigned)r_jit); failures++; continue; }
        uint32_t a, t_nat, t_int, t_jit, t_ref, t_rel;
        a = read_cntpct_lo(); for (int r = 0; r < 20; r++) { fill(k); natives[i].fn(nat_d, nat_s, k->n); } t_nat = (read_cntpct_lo() - a) / 20;
        a = read_cntpct_lo(); for (int r = 0; r < 20; r++) fill(k); t_ref = (read_cntpct_lo() - a) / 20;
        t_nat -= t_ref;
        a = read_cntpct_lo(); for (int r = 0; r < 4; r++) { load(k); run(0xFFFFFFFFu); } t_int = (read_cntpct_lo() - a) / 4;
        a = read_cntpct_lo(); for (int r = 0; r < 4; r++) load(k); t_rel = (read_cntpct_lo() - a) / 4;
        t_int -= t_rel;
        for (int r = 0; r < 200; r++) { load(k); run(0u); }                                  // warm: hot loops reach the block tier
        a = read_cntpct_lo(); for (int r = 0; r < 20; r++) { load(k); run(0u); } t_jit = (read_cntpct_lo() - a) / 20;
        a = read_cntpct_lo(); for (int r = 0; r < 20; r++) load(k); t_rel = (read_cntpct_lo() - a) / 20;
        t_jit -= t_rel;
        printf("[KCMP] %-10s %4u %6u %9u %7u %8u   %5u.%02u x\n", k->name, k->n, k->size, (unsigned)t_nat, (unsigned)t_int, (unsigned)t_jit, (unsigned)(t_jit / (t_nat ? t_nat : 1)), (unsigned)((t_jit * 100 / (t_nat ? t_nat : 1)) % 100));
    }
    printf(failures ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return failures;
}
