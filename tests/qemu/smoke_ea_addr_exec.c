/// @file smoke_ea_addr_exec.c
/// @brief Real execution of LEA and PEA across (An), (d16,An) and (d8,An,Xn), plus LEA d16(PC), whose extension word is relative to the 68K PC
///        (which the translated code must know, not the ARM address it runs at).
///
/// LEA is self-verifying without any extra bookkeeping: "LEA <ea>,A1 ;
/// JSR (A1)" reads the computed address straight back through JSR's
/// already-proven (An) form (smoke_call_exec.c) -- a wrong address lands
/// nowhere sensible instead of quietly passing. PEA needs one level of
/// BSR wrapping: "PEA <ea> ; RTS" is a classic computed-jump idiom (RTS
/// pops what PEA just pushed and jumps there), but for the landing's own
/// RTS to have something valid to return through afterward, the whole
/// pair has to run under a real call frame.
///
/// (d8,PC,Xn) never fits a slot-cache slot: it is single-stepped by the interpreter, and execution re-enters the cache at the next instruction. The code after it, in the same
/// page, must still run translated: the case checks that the skipped slots are not taken for translated ones.
///
/// Each landing ORs a distinct bit into D1 (0x01-0x80); a correct final D1
/// (0xFF) proves every address was computed correctly.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"
#include "mem_be.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t progmem[96] __attribute__((aligned(4)));
static uint32_t sup_stack[256] __attribute__((aligned(8)));

// Word indices: main (33 words) first, then landings, then wrappers
// (LEA/JSR + RTS for d8(PC,Xn); LEA + PEA + RTS apiece for the PEA modes).
enum {
    MAIN = 0,
    LAND_LEA_AN = 38, LAND_LEA_D16AN = 40, LAND_LEA_D8ANXN = 42,
    LAND_PEA_AN = 44, LAND_PEA_D16AN = 46, LAND_PEA_D8ANXN = 48,
    LAND_LEA_D16PC = 50,
    WRAP_PEA_AN = 52, WRAP_PEA_D16AN = 57, WRAP_PEA_D8ANXN = 63,
    END = 69,
    LAND_LEA_D8PCXN = 69,
};

static void wbe(uint16_t *a, uint16_t v) { ((uint8_t*)a)[0] = v >> 8; ((uint8_t*)a)[1] = v; }
static void w32(uint16_t *a, uint32_t v) { wbe(a, v >> 16); wbe(a + 1, v); }

/// @brief OR.L D2,D1 ; RTS -- the body shared by most landings.
static void def_land(int idx)
{
    wbe(&progmem[idx],     0x8282);  // OR.L D2,D1
    wbe(&progmem[idx + 1], 0x4E75);  // RTS
}

static uint16_t lea_an(int dst, int src)      { return 0x41D0 | (dst << 9) | src; }
static uint16_t lea_d16an(int dst, int src)   { return 0x41E8 | (dst << 9) | src; }
static uint16_t lea_d8anxn(int dst, int src)  { return 0x41F0 | (dst << 9) | src; }
static uint16_t lea_absl(int dst)             { return 0x41F9 | (dst << 9); }
static uint16_t lea_d16pc(int dst)            { return 0x41FA | (dst << 9); }
static uint16_t pea_an(int src)               { return 0x4850 | src; }
static uint16_t pea_d16an(int src)            { return 0x4868 | src; }
static uint16_t pea_d8anxn(int src)           { return 0x4870 | src; }
static uint16_t jsr_an(int src)               { return 0x4E90 | src; }
static uint16_t moveq(int dn, uint8_t imm)    { return 0x7000 | (dn << 9) | imm; }
static uint16_t briefext(int idx_dn, int8_t disp8) { return (uint16_t)(idx_dn << 12) | (uint8_t)disp8; }

/// @brief BSR.S: 1 word, disp8 relative to the word after the opcode.
static void emit_bsr_s(int at, int target)
{
    int32_t disp = target * 2 - (at * 2 + 2);
    wbe(&progmem[at], 0x6100 | ((uint8_t)disp));
}

/// @brief LEA d16(PC),Ax: 2 words, disp16 relative to the extension
/// word's own address -- 68K's PC-relative reference point (same
/// convention as Bcc/BSR: opcode_addr + 2).
static void emit_lea_d16pc(int at, int dst, int target)
{
    int32_t disp = target * 2 - (at * 2 + 2);
    wbe(&progmem[at], lea_d16pc(dst));
    wbe(&progmem[at + 1], (uint16_t)disp);
}

// A1 = An, A2 = An -- registers 6+1 and 6+2 in this codebase's map.
#define A1 1
#define A2 2

int main(void)
{
    printf("=== PJIT smoke test: LEA/PEA -- (An), (d16,An), (d8,An,Xn), (d16,PC), (d8,PC,Xn) ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    int i = MAIN;
    wbe(&progmem[i++], 0x7200);                     // MOVEQ #0,D1

    // LEA (An),A1 ; JSR (A1) -- JSR's own (An) form is already proven
    // (smoke_call_exec.c), so a wrong LEA lands nowhere sensible.
    wbe(&progmem[i++], moveq(2, 1));                // MOVEQ #1,D2
    wbe(&progmem[i], lea_absl(A2));
    w32(&progmem[i + 1], (uint32_t)&progmem[LAND_LEA_AN]); i += 3;
    wbe(&progmem[i++], lea_an(A1, A2));             // LEA (A2),A1
    wbe(&progmem[i++], jsr_an(A1));                 // JSR (A1) -> D1 |= 1

    // LEA 40(An),A1 ; JSR (A1)
    wbe(&progmem[i++], moveq(2, 2));                // MOVEQ #2,D2
    wbe(&progmem[i], lea_absl(A2));
    w32(&progmem[i + 1], (uint32_t)&progmem[LAND_LEA_D16AN] - 40); i += 3;
    wbe(&progmem[i], lea_d16an(A1, A2));
    wbe(&progmem[i + 1], 40); i += 2;               // LEA 40(A2),A1
    wbe(&progmem[i++], jsr_an(A1));                 // JSR (A1) -> D1 |= 2

    // LEA 8(An,D0.W),A1 ; JSR (A1) -- D0 is 0 (never touched above), so
    // the index register contributes nothing; this isolates the base +
    // displacement path without also needing to verify Xn scaling.
    wbe(&progmem[i++], moveq(2, 4));                // MOVEQ #4,D2
    wbe(&progmem[i], lea_absl(A2));
    w32(&progmem[i + 1], (uint32_t)&progmem[LAND_LEA_D8ANXN] - 8); i += 3;
    wbe(&progmem[i], lea_d8anxn(A1, A2));
    wbe(&progmem[i + 1], briefext(0, 8)); i += 2;   // LEA 8(A2,D0.W),A1
    wbe(&progmem[i++], jsr_an(A1));                 // JSR (A1) -> D1 |= 4

    // PEA (An) -- via BSR-wrapped "PEA <ea> ; RTS" computed jump.
    wbe(&progmem[i++], moveq(2, 8));                // MOVEQ #8,D2
    emit_bsr_s(i, WRAP_PEA_AN); i += 1;             // -> D1 |= 8

    // PEA 40(An)
    wbe(&progmem[i++], moveq(2, 0x10));             // MOVEQ #16,D2
    emit_bsr_s(i, WRAP_PEA_D16AN); i += 1;          // -> D1 |= 0x10

    // PEA 8(An,D0.W)
    wbe(&progmem[i++], moveq(2, 0x20));             // MOVEQ #32,D2
    emit_bsr_s(i, WRAP_PEA_D8ANXN); i += 1;         // -> D1 |= 0x20

    // LEA d16(PC),A1 ; JSR (A1) -- no base register at all; the address
    // comes straight from where this instruction sits. Fits in a cache
    // slot fine (no index register to add), so it runs inline like the
    // (An)-relative cases above.
    wbe(&progmem[i++], moveq(2, 0x40));             // MOVEQ #64,D2
    emit_lea_d16pc(i, A1, LAND_LEA_D16PC); i += 2;  // LEA d16(PC),A1
    wbe(&progmem[i++], jsr_an(A1));                 // JSR (A1) -> D1 |= 0x40

    // LEA d8(PC,D0.W),A1 ; JSR (A1) -- the mode that never fits a slot.
    // MOVEQ can't reach bit 7, so this marker is a MOVE.L #$80,D2.
    wbe(&progmem[i++], 0x243C); w32(&progmem[i], 0x80); i += 2;       // MOVE.L #$80,D2
    {
        int ext = i + 1;                            // the brief extension word
        int32_t d8 = LAND_LEA_D8PCXN * 2 - ext * 2; // measured from the extension word
        wbe(&progmem[i], 0x41FB | (A1 << 9));       // LEA (d8,PC,Xn),A1
        wbe(&progmem[i + 1], briefext(0, d8));      // D0.W index (D0 = 0), d8
        i += 2;
    }
    wbe(&progmem[i++], jsr_an(A1));                 // JSR (A1) -> D1 |= 0x80

    wbe(&progmem[i++], 0x4E4F);                     // TRAP #15 (test-only stop)

    if (i != LAND_LEA_AN) {
        printf("[FAIL] main is %d words, expected %d (fix the layout constants)\nFAIL\n",
               i, LAND_LEA_AN);
        qemu_exit();
        return 0;
    }

    def_land(LAND_LEA_AN);    def_land(LAND_LEA_D16AN); def_land(LAND_LEA_D8ANXN);
    def_land(LAND_PEA_AN);    def_land(LAND_PEA_D16AN);  def_land(LAND_PEA_D8ANXN);
    def_land(LAND_LEA_D16PC);
    def_land(LAND_LEA_D8PCXN);

    i = WRAP_PEA_AN;
    wbe(&progmem[i], lea_absl(A2));
    w32(&progmem[i + 1], (uint32_t)&progmem[LAND_PEA_AN]); i += 3;
    wbe(&progmem[i++], pea_an(A2));
    wbe(&progmem[i++], 0x4E75);                     // RTS
    if (i != WRAP_PEA_D16AN)
        printf("[FAIL] WRAP_PEA_AN is the wrong length\n");

    i = WRAP_PEA_D16AN;
    wbe(&progmem[i], lea_absl(A2));
    w32(&progmem[i + 1], (uint32_t)&progmem[LAND_PEA_D16AN] - 40); i += 3;
    wbe(&progmem[i], pea_d16an(A2));
    wbe(&progmem[i + 1], 40); i += 2;
    wbe(&progmem[i++], 0x4E75);                     // RTS
    if (i != WRAP_PEA_D8ANXN)
        printf("[FAIL] WRAP_PEA_D16AN is the wrong length\n");

    i = WRAP_PEA_D8ANXN;
    wbe(&progmem[i], lea_absl(A2));
    w32(&progmem[i + 1], (uint32_t)&progmem[LAND_PEA_D8ANXN] - 8); i += 3;
    wbe(&progmem[i], pea_d8anxn(A2));
    wbe(&progmem[i + 1], briefext(0, 8)); i += 2;
    wbe(&progmem[i++], 0x4E75);                     // RTS
    if (i != END)
        printf("[FAIL] WRAP_PEA_D8ANXN is the wrong length\n");

    uint32_t stk = (uint32_t)&sup_stack[256];
    pjit_set_sr(0x2000);
    PJIT_SET_STACKS(stk);

    uint32_t arm_entry = pjit_cache_dispatch((uint32_t)&progmem[MAIN]);
    if (!arm_entry) {
        printf("[FAIL] dispatch returned 0\nFAIL\n");
        qemu_exit();
        return 0;
    }

    pjit_set_an(7, stk);
    PJIT_TEST_RUN(arm_entry);

    // Unreachable: pjit_test_exit() below halts via qemu_exit().
    printf("[TEST] unexpectedly returned from JIT'd code\nFAIL\n");
    qemu_exit();
    return 0;
}

/// @brief TRAP #15 handler, a strong override of the platform's weak
///        default (platform/qemu_virt/semihost_exit.c). Never returns.
void pjit_test_exit(uint32_t d0)
{
    (void)d0;
    uint32_t d1 = pjit_get_dn(1);
    printf("[TEST] D1 = 0x%08X (want 0x000000FF)\n", d1);
    printf(d1 == 0xFFu ? "PASS\n" : "FAIL\n");
    qemu_exit();
}
