/// @file smoke_bcd_exec.c
/// @brief ABCD/SBCD/NBCD: exhaustive correctness for the
///        pjit/handle_bcd.s helpers, plus real end-to-end opcode-path
///        coverage for every addressing mode each instruction allows.
///
/// The helpers are called directly, the same way tests/qemu/smoke_div.c calls
/// handle_DIVU()/handle_DIVS() -- they're ordinary-AAPCS-callable naked
/// functions (r0/r1 in, flags via cpu->sr), so no JIT dispatch is needed
/// to exercise them exhaustively. The oracle here is deliberately *not*
/// the same "+0x66 / bc / dc" bit-trick the helpers themselves use --
/// it's plain decimal digit arithmetic, independent of that formula, so a
/// bug shared between the oracle and the implementation can't hide.
///
/// V is not checked: real 68000 documentation calls it undefined for
/// these three instructions, and the digit-arithmetic oracle has no
/// independent way to define it either.
#include <stdint.h>
#include "pjit.h"
#include "pjit_compat.h"

extern void qemu_exit(void);
extern void emit_opcode_table(void);

extern void abcd(uint8_t src, uint8_t *dst);
extern void sbcd(uint8_t src, uint8_t *dst);
extern void nbcd(uint8_t *dst);

// abcd()/sbcd()/nbcd() take old Z from the *live* ARM flags and leave the
// result's N/Z/V/C there; X lives in the GE flags (GE[3:0] = 1111 when X is clear). These
// shims load a chosen CPSR (flag nibble and GE) first and hand back the CPSR the function left.
extern uint32_t call_abcd(uint8_t src, uint8_t *dst, uint32_t cpsr_in);
extern uint32_t call_sbcd(uint8_t src, uint8_t *dst, uint32_t cpsr_in);
extern uint32_t call_nbcd(uint8_t *dst, uint32_t cpsr_in);
__asm__(
    ".arm\n"
    ".global call_abcd\n call_abcd:\n push {lr}\n msr cpsr_fs, r2\n bl abcd\n mrs r0, cpsr\n pop {pc}\n"
    ".global call_sbcd\n call_sbcd:\n push {lr}\n msr cpsr_fs, r2\n bl sbcd\n mrs r0, cpsr\n pop {pc}\n"
    ".global call_nbcd\n call_nbcd:\n push {lr}\n msr cpsr_fs, r1\n bl nbcd\n mrs r0, cpsr\n pop {pc}\n"
);
static uint32_t cpsr_out;
static int cur_x;

static int pass, fail;

static uint8_t dec_to_bcd(int d) { d %= 100; if (d < 0) d += 100; return (uint8_t)(((d / 10) << 4) | (d % 10)); }

/// @brief Sets cpu->sr's CCR to exactly X=x, Z=old_z, everything else 0
///        (matches what a real 68K program would have live before the op
///        -- N/V/C from a *previous* instruction are irrelevant here,
///        only X and the sticky Z accumulator matter going in).
static void prime(int x, int old_z)
{
    (void)old_z;                       // old Z and X go in via CPSR, see cpsr_in()
    cur_x = x;
    pjit_set_sr((uint16_t)(x ? 0x0010 : 0));
}
static uint32_t cpsr_in(int old_z) { return (old_z ? 0x40000000u : 0) | (cur_x ? 0 : 0x000F0000u); }

static void check(const char *op, uint8_t a, uint8_t b, int x, int old_z,
                   uint8_t got, uint8_t want, int want_c)
{
    uint16_t sr = pjit_get_sr();
    int got_c = sr & 1, got_x = !((cpsr_out >> 16) & 1), got_n = (sr >> 3) & 1, got_z = (sr >> 2) & 1;
    int want_n = (want >> 7) & 1;
    int want_z = old_z && (want == 0);
    // The live ARM flags must agree with the CCR byte saved to cpu->sr.
    int flags_agree = (((cpsr_out >> 31) & 1) == got_n) && (((cpsr_out >> 30) & 1) == got_z) &&
                      (((cpsr_out >> 29) & 1) == got_c) &&
                      (((cpsr_out >> 28) & 1) == ((sr >> 1) & 1));
    int ok = flags_agree && (got == want) && (got_c == want_c) && (got_x == want_c) &&
             (got_n == want_n) && (got_z == want_z);
    if (ok) { pass++; return; }
    if (fail++ < 20)
        printf("[FAIL] %s a=%02X b=%02X X=%d oldZ=%d -> got=%02X SR=%04X, want=%02X C/X=%d N=%d Z=%d\n",
               op, a, b, x, old_z, got, sr, want, want_c, want_n, want_z);
}

static void exhaustive(void)
{
    for (int d = 0; d < 100; d++) {
        for (int s = 0; s < 100; s++) {
            for (int x = 0; x <= 1; x++) {
                for (int oz = 0; oz <= 1; oz++) {
                    uint8_t sb = dec_to_bcd(s), db;

                    int sum = s + d + x;
                    db = dec_to_bcd(d);
                    prime(x, oz);
                    cpsr_out = call_abcd(sb, &db, cpsr_in(oz));
                    check("ABCD", sb, dec_to_bcd(d), x, oz, db, dec_to_bcd(sum % 100), sum >= 100);

                    int diff = d - s - x;
                    db = dec_to_bcd(d);
                    prime(x, oz);
                    cpsr_out = call_sbcd(sb, &db, cpsr_in(oz));
                    check("SBCD", sb, dec_to_bcd(d), x, oz, db, dec_to_bcd(diff), diff < 0);
                }
            }
        }
    }
    for (int d = 0; d < 100; d++) {
        for (int x = 0; x <= 1; x++) {
            for (int oz = 0; oz <= 1; oz++) {
                int diff = -d - x;
                uint8_t db = dec_to_bcd(d);
                prime(x, oz);
                cpsr_out = call_nbcd(&db, cpsr_in(oz));
                check("NBCD", dec_to_bcd(d), 0, x, oz, db, dec_to_bcd(diff), diff < 0);
            }
        }
    }
}

/// ---- Real end-to-end opcode-path coverage, one case per addressing mode ----

static uint8_t  ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint16_t progmem[32768u/2] __attribute__((aligned(32768u)));
static uint8_t  mem_buf[64] __attribute__((aligned(4)));

static uint32_t off, base, np;
static int opass, ofail;

static void wbe(uint16_t *a, uint16_t v) { ((uint8_t*)a)[0] = v >> 8; ((uint8_t*)a)[1] = v; }
static void w(uint16_t v)   { wbe(&progmem[base + np++], v); }
static void w32(uint32_t v) { w(v >> 16); w(v); }
// 32 68K words/case: the largest case (two movb_abs() setup writes plus
// two MOVEA.L and the opcode itself) needs 17; a case that outgrew its
// 32 words would overwrite the next case's first word in progmem.
static void start(void)     { base = off; np = 0; off += 32; }
static void movea(uint8_t an, uint32_t v) { w(0x207C | (an << 9)); w32(v); }

// Primes the CCR through D7 -- MOVEQ #v,D7 ; MOVE D7,CCR -- the register-source form of MOVE to CCR.
static void ccr(uint8_t v) { w(0x7E00 | v); w(0x44C7); }

// MOVE.B #v,addr.L -- sets up a byte of mem_buf at *execution* time. Cases
// are built (and any mem_buf[i]=... a case's own C code writes) all run
// before exec_all() runs any of them, so a plain C-side write is stale by
// the time the case it was "for" actually executes -- every later case's
// own setup has already overwritten it. Doing the write as 68K code run
// right before the case's real instructions sidesteps that entirely.
static void movb_abs(uint32_t addr, uint8_t v) { w(0x13FC); w(v); w32(addr); }

static struct {
    const char *name;
    uint32_t base, np;
    uint8_t want;      // expected low byte of D0 (or *check_addr)
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
        if (!e) { printf("[FAIL] %s: dispatch returned 0\n", cases[i].name); ofail++; continue; }

        uint8_t *stop = (uint8_t *)(e + cases[i].np * 2);   // TRAP #15 after the case: the old stop slot
        stop[0] = 0x4E; stop[1] = 0x4F;
        (void)pjit_compat_run((uint16_t *)e);

        uint8_t got = cases[i].check_addr ? *cases[i].check_addr : (uint8_t)pjit_get_dn(0);
        if (got == cases[i].want) opass++;
        else { printf("[FAIL] %-16s got=%02X want=%02X\n", cases[i].name, got, cases[i].want); ofail++; }
    }
}

int main(void)
{
    printf("=== PJIT smoke test: ABCD/SBCD/NBCD ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();

    printf("[RUN] exhaustive ABCD/SBCD/NBCD (40200 cases)\n");
    exhaustive();
    printf("[TEST] direct-call: %d passed, %d failed\n", pass, fail);

    for (uint32_t i = 0; i < sizeof(progmem)/2; i++) wbe(&progmem[i], 0x4E71); // NOP fill
    for (unsigned i = 0; i < sizeof(mem_buf); i++) mem_buf[i] = 0;

    // ABCD Dx,Dy (D1 -> D0): 0x35 + 0x27 + X=1 = 63 -> BCD 0x63.
    start();
    w(0x203C); w32(0x00000035u);           // MOVE.L #$35,D0
    w(0x223C); w32(0x00000027u);           // MOVE.L #$27,D1
    ccr(0x10);                              // X=1
    w(0xC101);                             // ABCD D1,D0
    def("ABCD Dx,Dy", 0x63, 0);

    // ABCD -(A1),-(A0): opcode 0xC109 has sReg=1 (A1, source, bits 2-0), dReg=0
    // (A0, dest, bits 11-9).
    // A1=&mem_buf[21] predecrements to the source at mem_buf[20]=0x09;
    // A0=&mem_buf[5] predecrements to the dest at mem_buf[4]=0x01, where
    // the result (9+1=10) lands.
    start();
    movb_abs((uint32_t)&mem_buf[20], 0x09);
    movb_abs((uint32_t)&mem_buf[4],  0x01);
    movea(0, (uint32_t)&mem_buf[5]);
    movea(1, (uint32_t)&mem_buf[21]);
    ccr(0x00);                              // X=0
    w(0xC109);                             // ABCD -(A1),-(A0)
    def("ABCD -(An),-(An)", 0x10, &mem_buf[4]);

    // SBCD Dx,Dy (D1 -> D0): 0x50 - 0x01 - X(0) = 49 -> BCD 0x49.
    start();
    w(0x203C); w32(0x00000050u);
    w(0x223C); w32(0x00000001u);
    ccr(0x00);
    w(0x8101);                             // SBCD D1,D0
    def("SBCD Dx,Dy", 0x49, 0);

    // SBCD -(A1),-(A0): same field mapping as the ABCD case above --
    // source at mem_buf[20]=0x01, dest at mem_buf[4]=0x10, X=1 ->
    // 10-1-1=8, result lands at the dest, mem_buf[4].
    start();
    movb_abs((uint32_t)&mem_buf[20], 0x01);
    movb_abs((uint32_t)&mem_buf[4],  0x10);
    movea(0, (uint32_t)&mem_buf[5]);
    movea(1, (uint32_t)&mem_buf[21]);
    ccr(0x10);
    w(0x8109);                             // SBCD -(A1),-(A0)
    def("SBCD -(An),-(An)", 0x08, &mem_buf[4]);

    // NBCD Dn: D2 = 0x45, X=0 -> 0-45 -> -45+100=55 -> 0x55.
    start();
    w(0x243C); w32(0x00000045u);           // MOVE.L #$45,D2
    ccr(0x00);
    w(0x4802);                             // NBCD D2
    w(0x2002);                             // MOVE.L D2,D0
    def("NBCD Dn", 0x55, 0);

    // NBCD (A0): mem_buf[0]=0x20, X=0 -> 0x80.
    start();
    movb_abs((uint32_t)&mem_buf[0], 0x20);
    movea(0, (uint32_t)&mem_buf[0]);
    ccr(0x00);
    w(0x4810);                             // NBCD (A0)
    def("NBCD (An)", 0x80, &mem_buf[0]);

    // NBCD (A0)+: same as above, and A0 must land one past.
    start();
    movb_abs((uint32_t)&mem_buf[8], 0x01);
    movea(0, (uint32_t)&mem_buf[8]);
    ccr(0x00);
    w(0x4818);                             // NBCD (A0)+
    def("NBCD (An)+", 0x99, &mem_buf[8]);

    // NBCD -(A0): predecrement from A0=+9 lands on mem_buf[8].
    start();
    movb_abs((uint32_t)&mem_buf[8], 0x50);
    movea(0, (uint32_t)&mem_buf[9]);
    ccr(0x10);                              // X=1
    w(0x4820);                             // NBCD -(A0)
    def("NBCD -(An)", 0x49, &mem_buf[8]);  // 0-50-1 -> -51+100=49

    // NBCD (d16,A0): displacement 8 from A0.
    start();
    movb_abs((uint32_t)&mem_buf[24], 0x33);
    movea(0, (uint32_t)&mem_buf[16]);
    ccr(0x00);
    w(0x4828); w(0x0008);                  // NBCD 8(A0)
    def("NBCD (d16,An)", 0x67, &mem_buf[24]); // 0-33 -> -33+100=67

    // NBCD abs.L -- a non-trivial operand (not 0-0=0): if the abs.L
    // address ever lands in the wrong register again, this fails loudly
    // instead of silently writing 0 back over whatever garbage address it
    // used, the same way it did before the (d16,An) fix above was found.
    start();
    movb_abs((uint32_t)&mem_buf[40], 0x12);
    ccr(0x00);
    w(0x4839); w32((uint32_t)&mem_buf[40]); // NBCD abs.L
    def("NBCD abs.L", 0x88, &mem_buf[40]);  // 0-12 -> -12+100=88

    exec_all();

    printf("[TEST] opcode-path: %d passed, %d failed\n", opass, ofail);

    int total_pass = pass + opass, total_fail = fail + ofail;
    printf("[TEST] %d passed, %d failed\n", total_pass, total_fail);
    printf(total_fail ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return 0;
}
