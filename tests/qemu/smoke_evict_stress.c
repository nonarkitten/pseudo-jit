/// @file smoke_evict_stress.c
/// @brief Many small routines scattered over 2 MB (so that four of them share every cache line index: the slot cache spans 512 KB), linked to each other by static JSR / JMP and called
///        in a pseudo-random order through a table (JSR (A0)). The checksum they leave (D1, D2) must be the same through the interpreter and through the cache, however often the
///        lines evict each other and the direct links between them are invalidated.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"

extern void emit_opcode_table(void);

#define R      3000                     // routines
#define CALLS  6000                   // calls from the main loop
static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t  big[0x200000 + 0x1000] __attribute__((aligned(0x80000)));
static uint8_t  stk[32768] __attribute__((aligned(16)));
static uint32_t rng = 12345;
static uint32_t rnd(void) { rng = rng * 1103515245u + 12345u; return rng >> 8; }
static void w16(uint8_t *p, uint16_t v) { p[0] = v >> 8; p[1] = (uint8_t)v; }
static void w32(uint8_t *p, uint32_t v) { w16(p, v >> 16); w16(p + 2, v); }
static uint32_t cell[R];

static void run(uint32_t entry, uint32_t *d1, uint32_t *d2)
{
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, (uint32_t)stk + 32000);
    cpu->ssp = cpu->usp = __builtin_bswap32((uint32_t)stk + 32000);
    pjit_set_sr(0x2700);
    pjit_run((uint16_t *)entry);
    *d1 = pjit_get_dn(1); *d2 = pjit_get_dn(2);
}

int main(void)
{
    printf("=== PJIT smoke test: eviction and links under a scattered call graph ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    // R routines in distinct 64-byte cells of a 2 MB area, chosen at random
    static uint8_t used[0x200000 / 64];
    for (uint32_t c = 0x100000 / 64; c < 0x101400 / 64; c++) used[c] = 1;      // the jump table's area and the two main loops' cells are not leaves
    for (uint32_t c = 0x1FE000 / 64; c < 0x200000 / 64; c++) used[c] = 1;
    for (int i = 0; i < R; i++) { uint32_t c; do c = rnd() % (0x200000 / 64); while (used[c]); used[c] = 1; cell[i] = (uint32_t)big + c * 64; }
    for (int i = 0; i < R; i++) {
        uint8_t *p = (uint8_t *)cell[i];
        w16(p, 0x0681); w32(p + 2, rnd()); p += 6;                       // ADDI.L #k,D1
        w16(p, 0xE799); p += 2;                                          // ROL.L #3,D1
        w16(p, 0xB581); p += 2;                                          // EOR.L D2,D1
        w16(p, 0x5282); p += 2;                                          // ADDQ.L #1,D2
        uint32_t t = rnd();
        if (i < R - 1 && (t & 3) != 0) {                                 // a static call to a later routine (the graph is acyclic)
            uint32_t j = i + 1 + rnd() % (R - 1 - i < 40 ? R - 1 - i : 40);
            w16(p, 0x4EB9); w32(p + 2, cell[j]); p += 6;                 // JSR routine_j
            w16(p, 0x0681); w32(p + 2, rnd()); p += 6;                   // ADDI.L #k2,D1
        }
        if (i < R - 1 && (t & 12) == 4) {                                // a tail call (static JMP)
            uint32_t j = i + 1 + rnd() % (R - 1 - i < 40 ? R - 1 - i : 40);
            w16(p, 0x4EF9); w32(p + 2, cell[j]);
        } else w16(p, 0x4E75);                                           // RTS
    }
    // the call table and the main loop (its own cell)
    static uint32_t table[CALLS];
    for (int i = 0; i < CALLS; i++) table[i] = cell[rnd() % R];
    uint32_t M = (uint32_t)big + 0x1FF000;
    uint8_t *p = (uint8_t *)M;
    w16(p, 0x227C); w32(p + 2, (uint32_t)table); p += 6;                 // MOVEA.L #table,A1
    w16(p, 0x263C); w32(p + 2, CALLS - 1); p += 6;                       // MOVE.L #CALLS-1,D3
    uint8_t *loop = p;
    w16(p, 0x2059); p += 2;                                              // loop: MOVEA.L (A1)+,A0
    w16(p, 0x4E90); p += 2;                                              //       JSR (A0)
    w16(p, 0x5383); p += 2;                                              //       SUBQ.L #1,D3
    w16(p, 0x6A00); w16(p + 2, (uint16_t)(loop - (p + 2))); p += 4;      //       BPL.W loop
    w16(p, 0x4E4F);                                                      // TRAP #15
    // big-endian table for the guest
    for (int i = 0; i < CALLS; i++) table[i] = __builtin_bswap32(table[i]);

    uint32_t a1, a2, b1, b2;
    cpu->dr_threshold = 0xFFFFFFFFu;                                     // the interpreter
    run(M, &a1, &a2);
    cpu->dr_threshold = DR_THRESHOLD_DEFAULT;                            // the cache
    pjit_cache_flush();
    run(M, &b1, &b2);
    int bad = a1 != b1 || a2 != b2;
    printf("interpreter D1 %08X D2 %08X; cache D1 %08X D2 %08X: %s\n", a1, a2, b1, b2, bad ? "FAIL" : "PASS");
    for (int pass = 0; pass < 2; pass++) {                               // again with the cache as it is, and again after a flush
        if (pass) pjit_cache_flush();
        run(M, &b1, &b2);
        if (a1 != b1 || a2 != b2) { bad = 1; printf("pass %d: cache D1 %08X D2 %08X: FAIL\n", pass, b1, b2); }
    }
    // ---- a library-style jump table: N packed 6-byte JMP abs.L vectors (2-aligned, straddling 16-byte groups and 128-byte lines), reached by JSR 0(A6,D4.W) with D4 = -6 * (k + 1), the leaves
    //      scattered over the 2 MB (Kickstart's RAM code is mostly this: the slot fill's spill and link handling run on every entry) ----
    {
        enum { NV = 700, NC = 12000 };
        for (int round = 0; round < 6; round++) {
            rng = 777 + round * 31;
            uint32_t base = (uint32_t)big + 0x100000 + 6 * NV + 2 * (rnd() % 64) + 0x40;          // A6 = the library base; the table lies below it
            for (int i = 0; i < R; i++) { uint8_t *q = (uint8_t *)cell[i]; w16(q, 0x0681); w32(q + 2, rnd()); w16(q + 6, 0xE799); w16(q + 8, 0xB581); w16(q + 10, 0x5282); w16(q + 12, 0x4E75); }   // leaves
            for (int k = 0; k < NV; k++) { uint8_t *e = (uint8_t *)(base - 6 * (k + 1)); w16(e, 0x4EF9); w32(e + 2, cell[rnd() % R]); }
            static uint16_t calls[NC];
            for (int i = 0; i < NC; i++) calls[i] = __builtin_bswap16((uint16_t)(0 - 6 * (rnd() % NV + 1)));
            uint32_t M2 = (uint32_t)big + 0x1FE000;
            uint8_t *q = (uint8_t *)M2;
            w16(q, 0x227C); w32(q + 2, (uint32_t)calls); q += 6;               // MOVEA.L #calls,A1
            w16(q, 0x2C7C); w32(q + 2, base); q += 6;                          // MOVEA.L #base,A6
            w16(q, 0x263C); w32(q + 2, NC - 1); q += 6;                        // MOVE.L #NC-1,D3
            uint8_t *lp = q;
            w16(q, 0x3819); q += 2;                                            // loop: MOVE.W (A1)+,D4
            w16(q, 0x4EB6); w16(q + 2, 0x4000); q += 4;                        //       JSR 0(A6,D4.W)
            w16(q, 0x5383); q += 2;                                            //       SUBQ.L #1,D3
            w16(q, 0x6A00); w16(q + 2, (uint16_t)(lp - (q + 2))); q += 4;      //       BPL.W loop
            w16(q, 0x4E4F);                                                    // TRAP #15
            uint32_t j1, j2, k1, k2;
            pjit_cache_flush(); cpu->dr_threshold = 0xFFFFFFFFu; run(M2, &j1, &j2); printf("  round %d: interpreter done D1 %08X\n", round, j1);
            cpu->dr_threshold = DR_THRESHOLD_DEFAULT; pjit_cache_flush(); printf("  round %d: cache run...\n", round); run(M2, &k1, &k2);
            int b2 = j1 != k1 || j2 != k2;
            run(M2, &k1, &k2); b2 |= j1 != k1 || j2 != k2;                     // and again with the cache as it is
            printf("jump table round %d: interpreter D1 %08X D2 %08X; cache D1 %08X D2 %08X: %s\n", round, j1, j2, k1, k2, b2 ? "FAIL" : "PASS");
            bad |= b2;
        }
    }
    printf("%s\n", bad ? "FAIL" : "PASS");
    return 0;
}
