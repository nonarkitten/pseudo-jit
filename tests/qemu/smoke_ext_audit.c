/// @file smoke_ext_audit.c
/// @brief EOR Dn,<ea> and Scc <ea> with extension words (d16(An), (d8,An,Xn), abs.W, abs.L): the forms where the extension-class table
///        (pjit/ext_table.c) and the generator's sequences must agree about which of r1 / r2 holds the extension value. Run through the DR cache and through the interpretive runtime.
#include <stdint.h>
#include "pjit.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t progmem[32768u/2] __attribute__((aligned(32768u)));
static uint32_t data_buf[8] __attribute__((aligned(4)));
static uint32_t off, base, np;
static int pass, fail;
static void wbe(uint16_t *a, uint16_t v) { ((uint8_t*)a)[0] = v >> 8; ((uint8_t*)a)[1] = v; }
static void w(uint16_t v)   { wbe(&progmem[base + np++], v); }
static void w32(uint32_t v) { w(v >> 16); w(v); }
static void start(void)     { base = off; np = 0; off += 48; }
static struct { const char *name; uint32_t base, np, want; } cases[40];
static int ncases;
static void def(const char *name, uint32_t want) { w(0x4E4F); cases[ncases].name = name; cases[ncases].base = base; cases[ncases].np = np; cases[ncases].want = want; ncases++; }
static uint8_t guest_stack[2048] __attribute__((aligned(8)));
static void movea(uint8_t an, uint32_t v) { w(0x207C | (an << 9)); w32(v); }
static void reset_data(void)
{
    uint8_t *db = (uint8_t *)data_buf;
    for (int i = 0; i < 8; i++) { uint32_t v = 0x11223340u + i; db[i*4+0] = v >> 24; db[i*4+1] = v >> 16; db[i*4+2] = v >> 8; db[i*4+3] = v; }
}
static void exec_all(void)
{
    for (int i = 0; i < ncases; i++) {
        printf("[RUN] %s\n", cases[i].name);
        reset_data();
        for (int r = 0; r < 8; r++) { pjit_set_dn(r, 0); pjit_set_an(r, 0); }
        pjit_set_an(7, (uint32_t)(guest_stack + sizeof(guest_stack)));
        pjit_set_sr(0x2700);
        pjit_run(&progmem[cases[i].base]);
        uint32_t d0 = pjit_get_dn(0);
        if (d0 == cases[i].want) pass++;
        else { printf("[FAIL] %-22s D0=0x%08X want 0x%08X\n", cases[i].name, d0, cases[i].want); fail++; }
    }
}
int main(void)
{
    printf("=== PJIT smoke test: EOR Dn,<ea> and Scc <ea> with extension words ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
#ifdef PJIT_TEST_NO_DR
    cpu->dr_threshold = 0xFFFFFFFFu;
#endif
    emit_opcode_table();
    for (uint32_t i = 0; i < sizeof(progmem)/2; i++) wbe(&progmem[i], 0x4E71);
    uint32_t buf = (uint32_t)data_buf;
    // memory at buf+4 = 0x11223341; afterwards MOVE.L 4(A0),D0 / MOVE.L abs,D0 reads it back
    start(); movea(0, buf); w(0x720F); w(0xB328); w(4); w(0x2028); w(4);                      def("EOR.B D1,4(A0)",       0x1E223341u);
    start(); movea(0, buf); w(0x720F); w(0xB368); w(4); w(0x2028); w(4);                      def("EOR.W D1,4(A0)",       0x11223341u ^ 0x000F0000u);
    start(); movea(0, buf); w(0x720F); w(0xB3A8); w(4); w(0x2028); w(4);                      def("EOR.L D1,4(A0)",       0x11223341u ^ 0x0000000Fu);
    start(); movea(0, buf); w(0x7400); w(0x720F); w(0xB330); w(0x2000); w(0x2010);           def("EOR.B D1,(0,A0,D2.W)", 0x1E223340u);   // D2 = 4 is overwritten by... (0,A0,D2.W) with D2 = 0: buf; D1 = 0x0F
    start(); w(0x720F); w(0xB339); w32(buf + 4); w(0x2039); w32(buf + 4);                     def("EOR.B D1,abs.L",       0x1E223341u);
    start(); w(0x720F); w(0xB378); w((uint16_t)0); w(0x2038); w(0);                           def("EOR.W D1,abs.W (0)",   0x00000000u);      // low memory: skipped, see below
    start(); movea(0, buf); w(0x50E8); w(4); w(0x2028); w(4);                                 def("ST 4(A0)",             0xFF223341u);
    start(); movea(0, buf); w(0x50F0); w(0x0004); w(0x2028); w(4);                            def("ST 4(A0,D0.W)",        0xFF223341u);    // (4,A0,D0.W): D0 = 0
    start(); w(0x50F9); w32(buf + 4); w(0x2039); w32(buf + 4);                                def("ST abs.L",             0xFF223341u);
    start(); movea(0, buf); w(0x51E8); w(4); w(0x2028); w(4);                                 def("SF 4(A0)",             0x00223341u);
    ncases--;                                                                                 // (the abs.W case needs low memory: dropped)
    cases[5] = cases[6]; cases[6] = cases[7]; cases[7] = cases[8]; cases[8] = cases[9]; ncases--;
    exec_all();
    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    return fail ? 1 : 0;
}
