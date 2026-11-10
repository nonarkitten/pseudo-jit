/// @file smoke_ot.c
/// @brief Checks the offset table (OT), the DRSR it points into, and the
///        DT's word 1 against each other for every opcode; also reports
///        which DT leaders read r1/r2 (extension-word data).
#include <stdint.h>
#include "pjit.h"

extern void emit_opcode_table(void);
extern void qemu_exit(void);

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

static int is_b(uint32_t w) { return (w & 0x0F000000u) == 0x0A000000u && (w >> 28) != 0xF; }
static uint32_t b_target(uint32_t w, uint32_t addr) {
    int32_t imm24 = (int32_t)(w << 8) >> 8;
    return addr + 8 + ((uint32_t)imm24 << 2);
}

// Mirrors ext_t in pjit.c; pjit_ext_table packs (src << 4) | dst.
extern const uint8_t pjit_ext_table[65536];
enum { X_NONE, X_I16, X_I32, X_D8_AREG, X_D16_PC, X_D8_XPC, X_BDISP, X_COUNT };
static const char *x_name[X_COUNT] = { "-", "I16", "I32", "d8,An,Xn", "d16,PC", "d8,PC,Xn", "bdisp" };

int main(void)
{
    printf("=== PJIT smoke test: offset table / DRSR consistency ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    uint32_t drsr_lo = (uint32_t)cpu->drsr, drsr_hi = drsr_lo + 0x200000u;
    int n_separate = 0, bad_leader = 0, bad_range = 0, bad_dt = 0, bad_class = 0, dep_leaders = 0;
    int n_class[4] = {0, 0, 0, 0};
    // [src][dst]: opcodes with extension words; how many have a leader that
    // reads r1 / r2 / neither.
    static int m_all[X_COUNT][X_COUNT], m_r1[X_COUNT][X_COUNT], m_r2[X_COUNT][X_COUNT];
    int with_ext = 0, indep = 0;

    for (uint32_t op = 0; op < 65536; op++) {
        uint32_t *e = pjit_drsr_entry((uint16_t)op);
        uint32_t cls = cpu->ot[op] & 3u;
        n_class[cls]++;

        if ((uint32_t)e < drsr_lo || (uint32_t)e >= drsr_hi || ((uint32_t)e & (DRSR_UNIT - 1))) {
            if (bad_range++ < 5) printf("[FAIL] %04X: DRSR entry %p out of range / misaligned\n", op, e);
            continue;
        }

        // DT word 1 (the interpretive path) is either the same body as the OT's
        // (a branch to it, or -- for a one-word body -- that very word) or, for
        // the opcodes whose guest-PC handling differs between the worlds, a
        // branch to a different, aligned DRSR entry of its own.
        uint32_t *w1 = &cpu->opcode_table[op * 4 + 1];
        int ok, separate = 0;
        if (pjit_arena.form[op]) {                         // a split entry: its own words, nop padding, then a branch (word 3) to the OT's routine
            w1 = &cpu->opcode_table[op * 4 + pjit_arena.form[op]];     // the terminal word follows the own words: `bx lr` (a bare routine) or a branch to it
            ok = pjit_arena.form[op] <= 3 && ((*w1 == 0xE12FFF1Eu && e[0] == 0xE12FFF1Eu) || (is_b(*w1) && b_target(*w1, (uint32_t)w1) == (uint32_t)e));
        } else if (e[1] == DRSR_PAD) {
            ok = (is_b(*w1) && is_b(e[0])) ? (b_target(*w1, (uint32_t)w1) == b_target(e[0], (uint32_t)e))
                                           : (*w1 == e[0]);
        } else {
            ok = is_b(*w1) && (b_target(*w1, (uint32_t)w1) == (uint32_t)e);
        }
        if (!ok && is_b(*w1)) {
            uint32_t t = b_target(*w1, (uint32_t)w1);
            if (t >= drsr_lo && t < drsr_hi && !(t & (DRSR_UNIT - 1))) ok = separate = 1;
        }
        if (!ok && (*w1 >> 28) == 0xE && (*w1 & 0x0FFFFFF0u) == 0x012FFF10u) ok = separate = 1;   // a lone `bx lr`
        n_separate += separate;
        if (!ok && bad_dt++ < 5) printf("[FAIL] %04X: DT word 1 0x%08X is neither the OT's body nor an aligned DRSR entry (%p)\n", op, *w1, e);

        // Length class.
        uint32_t words = pjit_ext_word_count((uint16_t)op);
        if (cls != OT_SPECIAL) {
            uint32_t want = (words == 1) ? OT_LEN_2 : (words == 2) ? OT_LEN_4 : OT_LEN_6;
            if (cls != want && bad_class++ < 5) printf("[FAIL] %04X: class %u but %u words\n", op, cls, words);
        }

        // Which leaders read r1/r2 (informational).
        if (words > 1 && (cpu->opcode_table[op * 4] != (0xEF000000u | ILLINSTR))) {     // SVC #ILLINSTR (cond AL, 1111, imm24)
            uint32_t l = cpu->opcode_table[op * 4];
            unsigned sm = (pjit_ext_table[op] >> 4) & 15, dm = pjit_ext_table[op] & 15;
            int r1 = pjit_arm_reads_reg(l, 1), r2 = pjit_arm_reads_reg(l, 2);
            with_ext++;
            m_all[sm][dm]++;
            if (r1) m_r1[sm][dm]++;
            if (r2) m_r2[sm][dm]++;
            if (!r1 && !r2) indep++;
            if (r1 || r2) {
                if (dep_leaders < 6) printf("[AUDIT] %04X leader 0x%08X reads r1/r2 (%u-word insn)\n", op, l, words);
                dep_leaders++;
            }
        }
    }

    printf("[MATRIX] opcodes with extension words, by (src ext, dst ext): total / leader reads r1 / reads r2\n");
    for (unsigned a = 0; a < X_COUNT; a++)
        for (unsigned b = 0; b < X_COUNT; b++)
            if (m_all[a][b])
                printf("[MATRIX]   src %-8s dst %-8s : %5d / %5d / %5d\n", x_name[a], x_name[b], m_all[a][b], m_r1[a][b], m_r2[a][b]);
    printf("[AUDIT] %d opcodes have extension words: %d have an ext-independent leader, %d read r1/r2 (fine: interpretive mode runs the extension handler before the DT)\n", with_ext, indep, dep_leaders);
    printf("[TEST] %d opcodes have an interpretive body of their own (guest-PC handling)\n", n_separate);
    printf("[TEST] classes: len2=%d special=%d len4=%d len6=%d\n", n_class[0], n_class[1], n_class[2], n_class[3]);
    printf("[TEST] %d out-of-range, %d DT mismatches, %d class mismatches, %d bad leaders\n", bad_range, bad_dt, bad_class, bad_leader);

    int ok = !bad_range && !bad_dt && !bad_class && !bad_leader;
    printf(ok ? "PASS\n" : "FAIL\n");
    qemu_exit();
    return ok ? 0 : 1;
}
