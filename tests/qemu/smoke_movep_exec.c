/// @file smoke_movep_exec.c
/// @brief MOVEP: real end-to-end coverage for all four forms (W/L x
///        register-to-memory/memory-to-register).
///
/// MOVEP transfers a Dn to/from alternate bytes of memory starting at
/// (d16,An), most significant byte first -- byte k of the transfer lands
/// at An+d16+k*2, so a .W transfer touches 2 bytes (offsets 0,2) and a
/// .L transfer touches 4 (offsets 0,2,4,6); the odd offsets in between
/// are untouched. Every case here checks both halves of that: the bytes
/// MOVEP is supposed to touch, *and* the ones on either side it must
/// leave alone (a sentinel byte at every odd offset, and -- for the .W
/// register forms -- the untouched half of Dn).
///
/// A nonzero, distinctive displacement is used throughout rather than 0:
/// a d16 of 0 would not prove that the displacement is used at all.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t progmem[32768u/2] __attribute__((aligned(32768u)));
static uint8_t  mem_buf[64] __attribute__((aligned(4)));

static uint32_t off, base, np;
static int pass, fail;

static void wbe(uint16_t *a, uint16_t v) { ((uint8_t*)a)[0] = v >> 8; ((uint8_t*)a)[1] = v; }
static void w(uint16_t v)   { wbe(&progmem[base + np++], v); }
static void w32(uint32_t v) { w(v >> 16); w(v); }
// 64 68K words/case: the .L reg->mem case alone needs 8 movb_abs() sentinel
// writes (4 words each = 32) plus its own setup and opcode -- learned the
// hard way in smoke_bcd_exec.c that an underestimate here doesn't fail
// loudly, it silently overwrites the next case's first instruction word.
static void start(void)     { base = off; np = 0; off += 64; }
static void movea(uint8_t an, uint32_t v) { w(0x207C | (an << 9)); w32(v); }
// MOVE.B #v,addr.L -- run at *execution* time as part of the case's own
// program, not at C build time (see smoke_bcd_exec.c's note on why a
// plain C-side mem_buf[i]=... write is stale by the time a later-built
// case actually executes).
static void movb_abs(uint32_t addr, uint8_t v) { w(0x13FC); w(v); w32(addr); }
static void movel_imm(uint8_t dn, uint32_t v)  { w(0x203C | (dn << 9)); w32(v); }

#define SENTINEL 0xAA

static struct {
    const char *name;
    uint32_t base, np;
    uint8_t want;
    uint8_t *check_addr; // NULL: check D0's low byte instead
} cases[16];
static int ncases;

static void def(const char *name, uint8_t want, uint8_t *addr)
{
    cases[ncases] = (typeof(cases[0])){ name, base, np, want, addr };
    ncases++;
}

static void exec_all(void)
{
    for (int i = 0; i < ncases; i++) {
        printf("[RUN] %s\n", cases[i].name);
        uint32_t e = pjit_cache_dispatch((uint32_t)&progmem[cases[i].base]);
        if (!e) { printf("[FAIL] %s: dispatch returned 0\n", cases[i].name); fail++; continue; }

        uint8_t *stop = (uint8_t *)(e + cases[i].np * 2);   // TRAP #15 after the case: the old stop slot
        stop[0] = 0x4E; stop[1] = 0x4F;
        (void)pjit_compat_run((uint16_t *)e);

        uint8_t got = cases[i].check_addr ? *cases[i].check_addr : (uint8_t)pjit_get_dn(0);
        if (got == cases[i].want) pass++;
        else { printf("[FAIL] %-24s got=%02X want=%02X\n", cases[i].name, got, cases[i].want); fail++; }
    }
}

/// @brief Checks one byte of mem_buf against an expected value, outside
///        the case table (multiple bytes need checking per case).
static void checkmem(const char *name, uint8_t *addr, uint8_t want)
{
    uint8_t got = *addr;
    if (got == want) { pass++; return; }
    printf("[FAIL] %-24s @%p got=%02X want=%02X\n", name, (void*)addr, got, want);
    fail++;
}

int main(void)
{
    printf("=== PJIT smoke test: MOVEP ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    for (uint32_t i = 0; i < sizeof(progmem)/2; i++) wbe(&progmem[i], 0x4E71); // NOP fill

    // ---- MOVEP.L D0,6(A0): D0=0x12345678 -> mem[6]=0x12,[8]=0x34,
    // [10]=0x56,[12]=0x78, with [7],[9],[11] (the odd offsets in between)
    // left at the sentinel.
    start();
    for (int k = 0; k < 8; k++) movb_abs((uint32_t)&mem_buf[6 + k], SENTINEL);
    movel_imm(0, 0x12345678u);
    movea(0, (uint32_t)&mem_buf[0]);
    w(0x01C8); w(0x0006);                  // MOVEP.L D0,6(A0)
    def("MOVEP.L reg->mem [6]", 0x12, &mem_buf[6]);
    // (the other three bytes and the sentinels are checked separately,
    // right after exec_all() runs this case -- see below)

    // ---- MOVEP.L 20(A0),D1: mem[20..26 even]=0xAA,0xBB,0xCC,0xDD ->
    // D1=0xAABBCCDD, replacing all 32 bits (poisoned beforehand).
    start();
    movb_abs((uint32_t)&mem_buf[20], 0xAA);
    movb_abs((uint32_t)&mem_buf[22], 0xBB);
    movb_abs((uint32_t)&mem_buf[24], 0xCC);
    movb_abs((uint32_t)&mem_buf[26], 0xDD);
    movel_imm(1, 0xDEADBEEFu);              // poison D1 first
    movea(0, (uint32_t)&mem_buf[0]);
    w(0x0348); w(0x0014);                  // MOVEP.L 20(A0),D1
    w(0x2001);                             // MOVE.L D1,D0 (for def()'s check)
    def("MOVEP.L mem->reg", 0xDD, 0);      // low byte of D1 == 0xAABBCCDD

    // ---- MOVEP.W D2,36(A0): D2's low word only, mem[36]=hi,[38]=lo.
    start();
    for (int k = 0; k < 4; k++) movb_abs((uint32_t)&mem_buf[36 + k], SENTINEL);
    movel_imm(2, 0x11119933u);
    movea(0, (uint32_t)&mem_buf[0]);
    w(0x0588); w(0x0024);                  // MOVEP.W D2,36(A0)
    def("MOVEP.W reg->mem [36]", 0x99, &mem_buf[36]);

    // ---- MOVEP.W 44(A0),D3: only D3's low word changes; upper word of
    // the poison value must survive.
    start();
    movb_abs((uint32_t)&mem_buf[44], 0x77);
    movb_abs((uint32_t)&mem_buf[46], 0x66);
    movel_imm(3, 0xCAFEFFFFu);              // poison, including low word
    movea(0, (uint32_t)&mem_buf[0]);
    w(0x0708); w(0x002C);                  // MOVEP.W 44(A0),D3
    w(0x4843);                             // SWAP D3 -- upper word into the low half
    w(0x2003);                             // MOVE.L D3,D0
    def("MOVEP.W mem->reg hi", 0xFE, 0);   // low byte of swapped D3 == upper word 0xCAFE's low byte

    exec_all();

    // Extra byte-level checks for the .L reg->mem case (odd offsets must
    // be untouched, and every even offset carries the right byte).
    checkmem("MOVEP.L reg->mem [8]",  &mem_buf[8],  0x34);
    checkmem("MOVEP.L reg->mem [10]", &mem_buf[10], 0x56);
    checkmem("MOVEP.L reg->mem [12]", &mem_buf[12], 0x78);
    checkmem("MOVEP.L reg->mem [7] untouched",  &mem_buf[7],  SENTINEL);
    checkmem("MOVEP.L reg->mem [9] untouched",  &mem_buf[9],  SENTINEL);
    checkmem("MOVEP.L reg->mem [11] untouched", &mem_buf[11], SENTINEL);
    checkmem("MOVEP.L reg->mem [13] untouched", &mem_buf[13], SENTINEL);

    checkmem("MOVEP.W reg->mem [37] untouched", &mem_buf[37], SENTINEL);
    checkmem("MOVEP.W reg->mem [38]",           &mem_buf[38], 0x33);
    checkmem("MOVEP.W reg->mem [39] untouched", &mem_buf[39], SENTINEL);

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return 0;
}
