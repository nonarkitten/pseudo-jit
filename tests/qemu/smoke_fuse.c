/// @file smoke_fuse.c
/// @brief The native string loops (pjit_fill, fill_fuse_try): MOVE.x (An)+,(Am)+ followed by DBF Dk back to it (any size), or (bytes) by BNE.S back to it, run as a tight ARM
///        loop. Checked against a plain C model: the copied bytes, both address registers, the counter (low word 0xFFFF, upper word kept), the flags
///        (N and Z from the last byte, V and C clear, X untouched), a jump straight into the DBF. The results must be the same whatever translates the loop.
#include <stdint.h>
#include "pjit.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t program[16] __attribute__((aligned(512)));
static uint8_t src[2048] __attribute__((aligned(16)));
static uint8_t dst[2048] __attribute__((aligned(16)));
static uint8_t guest_stack[1024] __attribute__((aligned(8)));
static int failures;

static void check(const char *what, uint32_t got, uint32_t want)
{
    int ok = (got == want);
    if (!ok) { printf("[TEST] %-60s got 0x%08X want 0x%08X FAIL\n", what, got, want); failures++; }
}
static void put(int i, uint16_t w) { uint8_t *b = (uint8_t *)&program[i]; b[0] = w >> 8; b[1] = (uint8_t)w; }
static int fused(int idx)       // is the MOVE slot (guest halfword idx of the program) a `b` into the fuse area?
{
    const uint32_t *q = (const uint32_t *)((uint32_t)cpu->cache_data + ((((uint32_t)program + 2 * idx) & 0x7FFFFu) << 3));
    if ((q[0] >> 24) != 0xEA) return 0;
    uint32_t t = (uint32_t)q + 8 + ((uint32_t)((int32_t)(q[0] << 8) >> 8) << 2);
    return t >= (uint32_t)pjit_arena.fuse && t < (uint32_t)pjit_arena.fuse + sizeof(pjit_arena.fuse);
}

/// size: 1 = .B, 2 = .L, 3 = .W (the opcode's size nibble); kind 0: DBF Dk (n + 1 elements); kind 1: BNE.S (bytes, until the NUL); kind 2: DBF entered by
/// jumping straight to it (one element fewer)
static void run(const char *name, int kind, int size, int n, int m, int k, int count, int want_fused)
{
    int e = size == 1 ? 1 : size == 2 ? 4 : 2;                      // bytes per element
    int so = e == 1 ? 5 : 8, dop = e == 1 ? 9 : 16;                 // where in src[] / dst[] the copy starts (aligned for .W / .L)
    uint16_t move = (size << 12) | 0xD8 | (m << 9) | n;
    int i = 0;
    if (kind == 2) put(i++, 0x6002);                                // BRA.S over the MOVE, straight to the DBF
    put(i++, move);
    if (kind == 1) put(i++, 0x66FC);                                // BNE.S back to the MOVE
    else { put(i++, 0x51C8 | k); put(i++, 0xFFFC); }                // DBF Dk, back to the MOVE
    put(i++, 0x4E4F);                                               // TRAP #15
    for (int j = 0; j < 8; j++) { pjit_set_dn(j, 0x11110000u + j); pjit_set_an(j, 0xA0000000u + j); }
    for (int j = 0; j < 2048; j++) { src[j] = (uint8_t)(j * 7 + 3); dst[j] = 0xEE; }
    int len = count + 1;                                            // elements moved
    if (kind == 2) len = count;                                     // (entered at the DBF: it decrements first)
    if (kind == 1) { len = count; for (int j = 0; j < len - 1; j++) if (!src[so + j]) src[so + j] = 1; src[so + len - 1] = 0; }
    uint32_t a_src = (uint32_t)src + so, a_dst = (uint32_t)dst + dop;
    pjit_set_an(n, a_src); pjit_set_an(m, a_dst);
    if (kind != 1) pjit_set_dn(k, 0xABCD0000u | (uint32_t)count);
    pjit_set_an(7, (uint32_t)(guest_stack + sizeof(guest_stack)));
    pjit_set_sr(0x2710);                                            // supervisor, X set, the rest clear
    pjit_cache_flush();
    pjit_run(program);
    for (int j = 0; j < len * e; j++) if (dst[dop + j] != src[so + j]) { printf("[TEST] %s: dst[%d] = %02X want %02X FAIL\n", name, j, dst[dop + j], src[so + j]); failures++; break; }
    for (int j = 0; j < dop; j++) if (dst[j] != 0xEE) { printf("[TEST] %s: dst before the copy was touched FAIL\n", name); failures++; break; }
    if (dst[dop + len * e] != 0xEE) { printf("[TEST] %s: dst past the copy was touched FAIL\n", name); failures++; }
    check(name, pjit_get_an(n), a_src + len * e);
    check(name, pjit_get_an(m), a_dst + len * e);
    if (kind != 1) check(name, pjit_get_dn(k), 0xABCD0000u | 0xFFFF);
    uint32_t last = 0;                                              // the last element, big-endian
    for (int j = 0; j < e; j++) last = last << 8 | src[so + (len - 1) * e + j];
    uint32_t top = 1u << (8 * e - 1);
    uint32_t ccr = len == 0 ? 0 : ((last & top) ? 8 : 0) | (last == 0 ? 4 : 0);    // (no MOVE ran: the flags are as they were)
    check(name, pjit_get_sr() & 0x1F, 0x10 | ccr);                  // X kept, N / Z from the last element, V and C clear
    int f = fused(kind == 2 ? 1 : 0);
    (void)want_fused;                                           // (the idiom recogniser is off: the loop compactor turns these into blocks, which smoke_loops checks)
    printf("[INFO] %-50s %d elements, fused %d\n", name, len, f);
}


/// The strcmp loop: MOVE.B (An)+,Dx ; MOVE.B (Am)+,Dy ; CMP.B Dx,Dy ; BNE.B exit ; TST.B Dx ; BNE.B back. The two exits are two TRAP #15s (nothing that would
/// touch the flags); which one ran shows in the saved PC.
static void run_cmp(const char *name, const uint8_t *sa, const uint8_t *sb, int n, int m, int x, int y)
{
    int i = 0;
    put(i++, 0x1018 | (x << 9) | n);                                // MOVE.B (An)+,Dx
    put(i++, 0x1018 | (y << 9) | m);                                // MOVE.B (Am)+,Dy
    put(i++, 0xB000 | (y << 9) | x);                                // CMP.B Dx,Dy
    put(i++, 0x6606);                                               // BNE.B +6: the mismatch exit
    put(i++, 0x4A00 | x);                                           // TST.B Dx
    put(i++, 0x66F4);                                               // BNE.B -12: back to the first MOVE
    put(i++, 0x4E4F);                                               // equal and finished: TRAP #15 (program[6])
    put(i++, 0x4E4F);                                               // the mismatch exit: TRAP #15 (program[7])
    for (int j = 0; j < 8; j++) { pjit_set_dn(j, 0x22220000u + 0x100 * j + j); pjit_set_an(j, 0xA0000000u + j); }
    for (int j = 0; j < 64; j++) { src[j] = sa[j]; dst[j] = sb[j]; }
    uint32_t a_a = (uint32_t)src, a_b = (uint32_t)dst;
    pjit_set_an(n, a_a); pjit_set_an(m, a_b);
    pjit_set_an(7, (uint32_t)(guest_stack + sizeof(guest_stack)));
    pjit_set_sr(0x2710);
    pjit_cache_flush();
    pjit_run(program);
    // the model
    uint32_t k = 0, b0 = 0, b1 = 0, res = 0; int mism = 0;
    for (;;) { b0 = sa[k]; b1 = sb[k]; k++; if (b1 != b0) { mism = 1; break; } if (b0 == 0) break; }
    uint32_t ccr;
    if (mism) { res = (b1 - b0) & 0xFF; ccr = (res & 0x80 ? 8 : 0) | (res == 0 ? 4 : 0) | (((b1 ^ b0) & (b1 ^ res) & 0x80) ? 2 : 0) | (b1 < b0 ? 1 : 0); }
    else ccr = 4;                                                   // TST.B of 0
    check(name, (uint32_t)cpu->pc, (uint32_t)&program[mism ? 8 : 7]);
    check(name, pjit_get_dn(x), (0x22220000u + 0x100 * x + x) & ~0xFFu | b0);
    check(name, pjit_get_dn(y), (0x22220000u + 0x100 * y + y) & ~0xFFu | b1);
    check(name, pjit_get_an(n), a_a + k);
    check(name, pjit_get_an(m), a_b + k);
    check(name, pjit_get_sr() & 0x1F, 0x10 | ccr);
    int f = fused(0);
    printf("[INFO] %-50s %s after %u bytes, fused %d\n", name, mism ? "mismatch" : "equal", (unsigned)k, f);
}

int main(void)
{
    printf("=== PJIT smoke test: native string loops (MOVE.B (An)+,(Am)+ ; DBF / BNE) ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    run("DBF.B, 1 byte (count 0)", 0, 1, 1, 0, 0, 0, 1);
    run("DBF.B, 5 bytes", 0, 1, 1, 0, 0, 4, 1);
    run("DBF.B, 300 bytes (count over 255)", 0, 1, 1, 0, 3, 299, 1);
    run("DBF.B, A6 -> A5, D7", 0, 1, 6, 5, 7, 40, 1);
    run("DBF.B, A2 -> A3, D2", 0, 1, 2, 3, 2, 17, 1);
    run("BNE.B until the NUL, 12 bytes", 1, 1, 1, 0, 0, 12, 1);
    run("BNE.B until the NUL, 1 byte (an empty string)", 1, 1, 1, 0, 0, 1, 1);
    run("DBF.B entered at the DBF", 2, 1, 1, 0, 0, 6, 1);
    run("DBF.B entered at the DBF, count 0 (no MOVE runs)", 2, 1, 1, 0, 0, 0, -1);
    run("DBF.B entered at the DBF, count 1", 2, 1, 1, 0, 0, 1, 1);
    run("DBF.L, 1 longword", 0, 2, 1, 0, 0, 0, 1);
    run("DBF.L, 60 longwords", 0, 2, 1, 0, 0, 59, 1);
    run("DBF.L, A4 -> A3, D6", 0, 2, 4, 3, 6, 20, 1);
    run("DBF.W, 1 word", 0, 3, 1, 0, 0, 0, 1);
    run("DBF.W, 90 words", 0, 3, 1, 0, 3, 89, 1);
    run("DBF.L entered at the DBF", 2, 2, 1, 0, 0, 9, 1);
    {
        static const uint8_t eq[64] = "hello, world", ne3[64] = "helmo, world", e0[64] = "", hi[64] = { 0x41, 0x80, 0x33 }, lo[64] = { 0x41, 0x7F, 0x33 };
        static const uint8_t long1[64] = "DHRYSTONE PROGRAM, 2'ND STRING", long2[64] = "DHRYSTONE PROGRAM, 2'ND STRINGX";
        run_cmp("strcmp equal", eq, eq, 1, 0, 0, 1);
        run_cmp("strcmp mismatch at 4 (n > m)", eq, ne3, 1, 0, 0, 1);
        run_cmp("strcmp mismatch at 4 (n < m)", ne3, eq, 1, 0, 0, 1);
        run_cmp("strcmp both empty", e0, e0, 1, 0, 0, 1);
        run_cmp("strcmp 0x80 vs 0x7F (signed overflow)", hi, lo, 1, 0, 0, 1);
        run_cmp("strcmp 0x7F vs 0x80", lo, hi, 1, 0, 0, 1);
        run_cmp("strcmp one string longer", long1, long2, 1, 0, 0, 1);
        run_cmp("strcmp A5 / A3, D3 / D6", long1, long1, 5, 3, 3, 6);
        run_cmp("strcmp A6 / A0, D7 / D2", long1, long2, 6, 0, 7, 2);
    }
    printf(failures ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return failures != 0;
}
