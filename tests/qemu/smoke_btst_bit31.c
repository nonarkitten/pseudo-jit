/// @file smoke_btst_bit31.c
/// @brief BCHG/BCLR/BSET/BTST against a Dn-direct destination, bit
///        numbers >= 8. A Dn-direct destination is a full longword for
///        these four opcodes (a memory destination is always a byte), so
///        the whole register is reachable: `btst.l #31,d0` tests the
///        sign bit.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t progmem[512] __attribute__((aligned(4)));
static uint32_t saved_sp;

static void wbe(uint16_t *a, uint16_t v) { ((uint8_t *)a)[0] = v >> 8; ((uint8_t *)a)[1] = v; }

typedef struct {
    const char *name;
    uint16_t    opcode;    // #n,D0 static form (opcode | bit-number ext word follows)
    uint16_t    bit;
    uint32_t    d0_before;
    uint32_t    d0_want;
    int         z_want;    // Z before the op (BCHG/BCLR/BSET) or Z as the test result (BTST)
} case_t;

// #imm,Dn static forms: BTST=0x0800, BCHG=0x0840, BCLR=0x0880, BSET=0x08C0 | Dn(0).
static const case_t cases[] = {
    {"BTST.L #31,D0 (sign bit set)",    0x0800, 31, 0x80000001u, 0x80000001u, 0},
    {"BTST.L #8,D0 (above the low byte, clear)", 0x0800, 8,  0x00000001u, 0x00000001u, 1},
    {"BCHG.L #16,D0 (0 -> 1)",           0x0840, 16, 0x00000000u, 0x00010000u, 1},
    {"BCLR.L #20,D0 (1 -> 0)",           0x0880, 20, 0x00100000u, 0x00000000u, 0},
    {"BSET.L #24,D0 (0 -> 1)",           0x08C0, 24, 0x00000000u, 0x01000000u, 1},
};
#define NCASES (sizeof(cases) / sizeof(cases[0]))
#define WORDS_PER_CASE 24

static unsigned failed;
static unsigned case_idx;

static void run_case(uint32_t e, uint32_t stk)
{
    uint8_t *stop = (uint8_t *)(e + 8 * 2);   // 8 68K words reserved (well over the 4 used)
    stop[0] = 0x4E; stop[1] = 0x4F;           // TRAP #15
    pjit_set_an(7, stk);
    (void)pjit_compat_run((uint16_t *)e);
}

int main(void)
{
    printf("=== PJIT smoke test: BCHG/BCLR/BSET/BTST #n,Dn, n >= 8 ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    static uint32_t stk[64] __attribute__((aligned(8)));
    uint32_t stk_top = (uint32_t)&stk[64];

    for (unsigned i = 0; i < NCASES; i++) wbe(&progmem[i * WORDS_PER_CASE], 0x4E71); // NOP-fill each slot

    for (unsigned i = 0; i < NCASES; i++) {
        uint16_t *p = &progmem[i * WORDS_PER_CASE];
        // MOVE.L #d0_before,D0 ; <op> #bit,D0 ; MOVE SR,D2
        wbe(&p[0], 0x203C);
        wbe(&p[1], (uint16_t)(cases[i].d0_before >> 16));
        wbe(&p[2], (uint16_t)cases[i].d0_before);
        wbe(&p[3], cases[i].opcode);
        wbe(&p[4], cases[i].bit);
        wbe(&p[5], 0x40C2);
    }

    for (unsigned i = 0; i < NCASES; i++) {
        case_idx = i;
        uint16_t *p = &progmem[i * WORDS_PER_CASE];
        uint32_t e = pjit_cache_dispatch((uint32_t)p);
        if (!e) { printf("case %2u: %-32s dispatch returned 0\n", i, cases[i].name); failed++; continue; }

        pjit_set_sr(0x2700);
        pjit_set_dn(0, 0xDEADBEEFu);
        pjit_set_dn(2, 0);

        run_case(e, stk_top);

        uint32_t d0 = pjit_get_dn(0);
        uint16_t sr = (uint16_t)pjit_get_dn(2);
        int z = (sr >> 2) & 1;
        int ok = (d0 == cases[i].d0_want) && (z == cases[i].z_want);
        printf("case %2u: %-32s D0=%08X (want %08X) Z=%d (want %d) %s\n",
               i, cases[i].name, d0, cases[i].d0_want, z, cases[i].z_want, ok ? "ok" : "MISMATCH");
        if (!ok) failed++;
    }

    printf("[TEST] %u/%u passed\n", (unsigned)NCASES - failed, (unsigned)NCASES);
    printf(failed == 0 ? "PASS\n" : "FAIL\n");
    qemu_exit();
    return failed ? 1 : 0;
}
