/// @file smoke_loops.c
/// @brief Loop compaction (pjit_fill, compact_try): small loops -- DBF, Bcc and BRA closing, an internal forward branch, a break, nested loops, string copy and compare -- run
///        interpretively and out of the DR cache (which turns the loops into blocks after their first trip), registers and memory compared; the number of blocks made is shown.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t code[16 * 1024] __attribute__((aligned(1024)));
static uint8_t buf0[256] __attribute__((aligned(16))), buf1[256] __attribute__((aligned(16)));
static uint8_t stack_mem[256] __attribute__((aligned(16)));
static int failures;
static void put(uint8_t *at, int hw, uint16_t v) { at[hw * 2] = v >> 8; at[hw * 2 + 1] = (uint8_t)v; }

typedef struct { const char *name; uint16_t w[24]; int n; const char *src; } prog_t;
#define T 0x4E4F
static const prog_t progs[] = {
    { "CLR.L (A0)+ ; DBF",                       { 0x700F, 0x4298, 0x51C8, 0xFFFC, T }, 5, "" },
    { "ADD.L D0,D1 ; DBF",                       { 0x7200, 0x303C, 0x0009, 0xD280, 0x51C8, 0xFFFC, T }, 7, "" },
    { "MOVE.L (A0)+,(A1)+ ; DBF",                { 0x303C, 0x000F, 0x22D8, 0x51C8, 0xFFFC, T }, 6, "pattern" },
    { "ADD.L D0,D1 ; SUBQ.L #1,D0 ; BNE",        { 0x700A, 0x7200, 0xD280, 0x5380, 0x66FA, T }, 6, "" },
    { "an internal forward branch",              { 0x7008, 0x7200, 0x7400, 0x5281, 0x2601, 0x0283, 0x0000, 0x0003, 0x6602, 0x5282, 0x5380, 0x66EE, T }, 13, "" },
    { "a break: MOVE.B (A0)+,D1 ; BEQ ; ADD ; BRA", { 0x7400, 0x1218, 0x6704, 0xD481, 0x60F8, T }, 6, "bytes" },
    { "nested DBF inside BNE",                   { 0x7203, 0x7000, 0x303C, 0x0003, 0x5282, 0x51C8, 0xFFFC, 0x5381, 0x66F2, T }, 10, "" },
    { "strcpy: MOVE.B (A1)+,(A0)+ ; BNE",        { 0x10D9, 0x66FC, T }, 3, "string" },
    { "strcmp: MOVE.B ; CMP.B ; BNE ; TST ; BNE", { 0x1018, 0xB019, 0x6604, 0x4A00, 0x66F6, T }, 6, "string" },
    { "ADD.W (A0)+,D1 ; DBF",                    { 0x7200, 0x303C, 0x0007, 0xD258, 0x51C8, 0xFFFC, T }, 7, "pattern" },
    { "MULU in a loop",                          { 0x7005, 0x7201, 0xC2C0, 0x5380, 0x66FA, T }, 6, "" },
    { "LSL.L #2 ; EOR ; DBF",                    { 0x303C, 0x0009, 0x7201, 0xE589, 0xB380, 0x51C8, 0xFFF8, T }, 8, "" },
    { "BSR inside the loop (not compacted)",     { 0x700B, 0x6104, 0x5380, 0x66FA, T, 0x5281, 0x4E75 }, 7, "" },
};

typedef struct { uint32_t d[8], a[8], sr; uint8_t mem0[256], mem1[256]; } state_t;

static void run(const prog_t *p, int slot, state_t *s, uint32_t thr)
{
    uint8_t *at = code + slot * 1024;
    for (int i = 0; i < p->n; i++) put(at, i, p->w[i]);
    for (int i = 0; i < 256; i++) { buf0[i] = 0xA0 + i * 7; buf1[i] = 0x10 + i * 3; }
    if (p->src[0] == 'b') { static const uint8_t b[] = { 3, 1, 4, 1, 5, 9, 2, 6, 0 }; for (unsigned i = 0; i < sizeof b; i++) buf0[i] = b[i]; }
    if (p->src[0] == 's') { for (int i = 0; i < 20; i++) { buf0[i] = 'a' + i % 7; buf1[i] = 'a' + i % 7; } buf0[19] = 0; buf1[19] = 0; buf1[11] = 'z'; }
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0x11110000u + i * 3); pjit_set_an(i, 0); }
    pjit_set_an(0, (uint32_t)buf0); pjit_set_an(1, (uint32_t)buf1);
    uint32_t sp = (uint32_t)stack_mem + sizeof stack_mem - 16;
    pjit_set_an(7, sp); cpu->ssp = cpu->usp = __builtin_bswap32(sp);
    pjit_set_sr(0x2700);
    cpu->dr_threshold = thr;
    pjit_cache_flush();
    pjit_run((uint16_t *)at);
    for (int i = 0; i < 8; i++) { s->d[i] = pjit_get_dn(i); s->a[i] = pjit_get_an(i); }
    s->sr = pjit_get_sr() & 0x1F;
    for (int i = 0; i < 256; i++) { s->mem0[i] = buf0[i]; s->mem1[i] = buf1[i]; }
}

int main(void)
{
    printf("=== PJIT smoke test: loop compaction ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    for (unsigned k = 0; k < sizeof(progs) / sizeof(progs[0]); k++) {
        static state_t a, b;
        run(&progs[k], k, &a, 0xFFFFFFFFu);
        uint32_t made0 = pjit_arena.comp[127];
        run(&progs[k], k, &b, DR_THRESHOLD_DEFAULT);
        int bad = 0;
        for (int i = 0; i < 8; i++) { if (a.d[i] != b.d[i]) { printf("[TEST] %s: D%d interp %08X cached %08X FAIL\n", progs[k].name, i, a.d[i], b.d[i]); bad = 1; }
                                      if (i < 7 && a.a[i] != b.a[i] - 0 && !(i < 2)) { printf("[TEST] %s: A%d interp %08X cached %08X FAIL\n", progs[k].name, i, a.a[i], b.a[i]); bad = 1; } }
        if (a.a[0] != b.a[0] || a.a[1] != b.a[1]) { printf("[TEST] %s: A0/A1 interp %08X/%08X cached %08X/%08X FAIL\n", progs[k].name, a.a[0], a.a[1], b.a[0], b.a[1]); bad = 1; }
        if (a.sr != b.sr) { printf("[TEST] %s: CCR interp %02X cached %02X FAIL\n", progs[k].name, a.sr, b.sr); bad = 1; }
        for (int i = 0; i < 256; i++) if (a.mem0[i] != b.mem0[i] || a.mem1[i] != b.mem1[i]) { printf("[TEST] %s: memory differs at %d FAIL\n", progs[k].name, i); bad = 1; break; }
        failures += bad;
        printf("[DBG] last c_words failure site %u\n", (unsigned)pjit_arena.comp[118]); pjit_arena.comp[118] = 0;
        if (pjit_arena.comp[127] != made0) { const uint32_t *blk = (const uint32_t *)pjit_arena.comp[1]; const uint32_t *end = (const uint32_t *)pjit_arena.comp[123]; printf("[BLOCK] %s:", progs[k].name); for (const uint32_t *q = blk; q < end; q++) printf(" %08X", (unsigned)*q); printf("\n"); }
        printf("[LOOP] %-46s %s: %u block(s) made\n", progs[k].name, bad ? "FAIL" : "ok  ", (unsigned)(pjit_arena.comp[127] - made0));
    }
    printf("[TOTAL] %u blocks made\n", (unsigned)pjit_arena.comp[127]);
    printf(failures ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return failures != 0;
}
