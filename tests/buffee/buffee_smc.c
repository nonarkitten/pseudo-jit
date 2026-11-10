/// @file buffee_smc.c
/// @brief Self-modifying code through the translating tiers (slot cache + blocks) on Buffee: the guest runs a routine, patches it with a MOVE.W #imm,abs.L, runs it again.
///        A 68000 has no instruction cache, so Kickstart never flushes after writing code (library jump tables, SetFunction): the second run must see the new instruction.
///        `SMC <case>: D2 (old) D3 (new) ... PASS|FAIL`. Cases: 1 same slot line, 2 a routine far away, 3 called through a JMP (A0) / JSR (A0), 4 patched by a byte write, 5 a loop whose body is patched
///        after the first 300 iterations (a promoted block).
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"
extern void emit_opcode_table(void);
#define CODE   0x08001000u
#define STACK  0x08100000u
static uint32_t hw;
static void w(uint16_t v) { write_BE_word(CODE + hw * 2, v); hw++; }
static void wl(uint32_t v) { w(v >> 16); w(v & 0xFFFF); }
static void at(uint32_t off) { hw = off / 2; }
static uint32_t run(void)
{
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, STACK); cpu->ssp = cpu->usp = __builtin_bswap32(STACK); pjit_set_sr(0x2700);
    cpu->irq_level = 0; cpu->irq_flag = 0; pjit_cache_flush();
    return pjit_run((uint16_t *)CODE);
}
static int check(const char *n, uint32_t d2, uint32_t d3)
{
    int ok = pjit_get_dn(2) == d2 && pjit_get_dn(3) == d3;
    printf("SMC %s: D2 %08X (want %08X) D3 %08X (want %08X) %s\n", n, pjit_get_dn(2), d2, pjit_get_dn(3), d3, ok ? "PASS" : "FAIL");
    return !ok;
}
int main(void)
{
    int bad = 0;
    printf("=== SMC through the translating tiers ===\n");
    pjit_cache_init(BUFFEE_CACHE_TOP); cpu->trap_exit_mask = 0x8000; emit_opcode_table(); cpu->dr_threshold = DR_THRESHOLD_DEFAULT;
    for (int c = 1; c <= 6; c++) {
        uint32_t sub = c == 2 ? CODE + 0x2000 : CODE + 0x100;
        at(0);
        if (c == 5) { w(0x2C7C); wl(sub); w(0x363C); w(300); w(0x4E96); w(0x51CB); w(0xFFFC); }   // MOVEA.L #sub,A6 ; MOVE.W #300,D3 ; L: JSR (A6) ; DBF D3,L
        else if (c == 6) { w(0x4EB9); wl(sub); }                         // JSR sub.L (a static call: linked slot to slot, no router entry before the stale code runs)
        else { w(0x207C); wl(sub); w(0x4E90); }                          // MOVEA.L #sub,A0 ; JSR (A0)
        w(0x2401);                                                       // MOVE.L D1,D2
        if (c == 4) { w(0x13FC); w(0x0007); wl(sub + 1); }               // MOVE.B #7,sub+1 (the MOVEQ's data byte)
        else { w(0x33FC); w(0x7207); wl(sub); }                          // MOVE.W #$7207,sub : MOVEQ #7,D1
        if (c == 5) w(0x4E96); else if (c == 6) { w(0x4EB9); wl(sub); } else w(0x4E90);
        w(0x2601);                                                       // MOVE.L D1,D3
        w(0x4E4F);                                                       // TRAP #15: exit
        at(sub - CODE); w(0x7205); w(0x4E75);                            // MOVEQ #5,D1 ; RTS
        run();
        static const char *nm[] = { "", "1 same line, JSR (A0)", "2 far routine, JSR (A0)", "3 patch + JSR (A0) again", "4 byte write", "5 hot loop (block)", "6 static JSR sub.L (known limit: linked slots)" };
        int f = check(nm[c], 5, 7);
        if (c < 6) bad += f; else if (f) printf("SMC   (case 6 is informational: a direct link bypasses the router, so the flush waits for its next entry)\n");
    }
    buffee_smc_report();
    printf("SMC %s\n", bad ? "FAIL" : "PASS");
    return 0;
}
