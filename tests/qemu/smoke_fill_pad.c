/// @file smoke_fill_pad.c
/// @brief A function that follows `RTS ; dc.w 0` inside the same 16-byte translation group. The fill that translates the first function reads on past its RTS: `0000` decodes as ORI.B #imm,D0 and
///        swallows the NEXT function's first word (a LINK) as its immediate, leaving the leftovers of the ORI in the slot where the real function starts. A later call to that function must run
///        the function, not the leftovers.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"

extern void emit_opcode_table(void);
static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t  code[0x1000] __attribute__((aligned(0x1000)));
static uint8_t  stk[1024] __attribute__((aligned(16)));
static void w16(uint8_t *p, uint16_t v) { p[0] = v >> 8; p[1] = (uint8_t)v; }
static void w32(uint8_t *p, uint32_t v) { w16(p, v >> 16); w16(p + 2, v); }
int main(void)
{
    printf("=== PJIT smoke test: a function after RTS + padding in the same translation group ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    int bad = 0;
    for (int variant = 0; variant < 4; variant++) {
        uint8_t *f1 = code + 0x100 + variant * 2 * 16;                    // 16-byte aligned groups: F1 at +0
        uint32_t F1 = (uint32_t)f1, F2 = F1 + 6, M = (uint32_t)code + 0x800;
        w16(f1 + 0, 0x7001);                                              // F1: MOVEQ #1,D0
        w16(f1 + 2, 0x4E75);                                              //     RTS
        w16(f1 + 4, 0x0000);                                              //     dc.w 0   (ORI.B #imm,D0 to a decoder that reads on)
        w16(f1 + 6, 0x4E55); w16(f1 + 8, 0xFFF4);                         // F2: LINK A5,#-12
        w16(f1 + 10, 0x7207);                                             //     MOVEQ #7,D1
        w16(f1 + 12, 0x4E5D);                                             //     UNLK A5
        w16(f1 + 14, 0x4E75);                                             //     RTS
        uint8_t *p = (uint8_t *)M;
        w16(p, 0x4EB9); w32(p + 2, F1); p += 6;                           // JSR F1 (translates the group from F1: reads on past the RTS)
        if (variant & 1) { w16(p, 0x207C); w32(p + 2, F2); p += 6; w16(p, 0x4E90); p += 2; }      // MOVEA.L #F2,A0 ; JSR (A0)
        else { w16(p, 0x4EB9); w32(p + 2, F2); p += 6; }                  // JSR F2
        w16(p, 0x4E4F);                                                   // TRAP #15
        for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
        pjit_set_an(7, (uint32_t)stk + 960); cpu->ssp = cpu->usp = __builtin_bswap32((uint32_t)stk + 960);
        pjit_set_sr(0x2700);
        pjit_cache_flush();
        pjit_run((uint16_t *)M);
        int ok = pjit_get_dn(0) == 1 && pjit_get_dn(1) == 7 && pjit_get_an(7) == (uint32_t)stk + 960;
        printf("variant %d: D0 %08X (1) D1 %08X (7) A7 %08X: %s\n", variant, pjit_get_dn(0), pjit_get_dn(1), pjit_get_an(7), ok ? "PASS" : "FAIL");
        bad |= !ok;
    }
    printf("%s\n", bad ? "FAIL" : "PASS");
    return 0;
}
