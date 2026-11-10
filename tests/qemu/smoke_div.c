/// @file smoke_div.c
/// @brief Direct unit test for the DIVU.W/DIVS.W helpers in
///        pjit/handle_divw.s, including 68K's overflow reporting.
///
/// Calls the helpers as ordinary functions rather than through emitted
/// code, so it runs without building the opcode table. The oracle is
/// plain C: the compiler is free to use hardware divide here, which is
/// exactly the thing the helpers exist to avoid on the real target.
#include <stdint.h>
#include "pjit.h"

extern void qemu_exit(void);

// r0 = (remainder << 16) | quotient, r1 = overflow -- an AAPCS 64-bit
// return, low word first.
extern uint64_t handle_DIVU(uint32_t num, uint32_t den);
extern uint64_t handle_DIVS(int32_t num, int32_t den);

static int pass, fail;

static void check(const char *kind, uint32_t num, uint32_t den,
                  uint64_t got, uint32_t want_res, uint32_t want_ovf)
{
    uint32_t res = (uint32_t)got, ovf = (uint32_t)(got >> 32);
    if (res == want_res && ovf == want_ovf) { pass++; return; }
    if (fail++ < 12)
        printf("[FAIL] %s %08X / %08X -> %08X ovf=%u, want %08X ovf=%u\n",
               kind, num, den, res, ovf, want_res, want_ovf);
}

static void do_u(uint32_t num, uint32_t den)
{
    uint32_t want_res, want_ovf;
    if (den == 0 || num / den > 0xFFFFu) { want_res = num; want_ovf = 1; }
    else { want_res = ((num % den) << 16) | (num / den); want_ovf = 0; }
    check("DIVU", num, den, handle_DIVU(num, den), want_res, want_ovf);
}

static void do_s(int32_t num, int32_t den)
{
    uint32_t want_res, want_ovf;
    // INT32_MIN / -1 would trap the oracle itself, so it is screened here
    // rather than after the divide.
    if (den == 0 || (num == (int32_t)0x80000000 && den == -1)) {
        want_res = (uint32_t)num; want_ovf = 1;
    } else {
        int32_t q = num / den;
        if (q > 32767 || q < -32768) { want_res = (uint32_t)num; want_ovf = 1; }
        else { want_res = ((uint32_t)(num % den) << 16) | ((uint32_t)q & 0xFFFFu);
               want_ovf = 0; }
    }
    check("DIVS", (uint32_t)num, (uint32_t)den, handle_DIVS(num, den),
          want_res, want_ovf);
}

static uint32_t seed = 0x13579BDFu;
static uint32_t rnd(void) { seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5; return seed; }

int main(void)
{
    printf("=== PJIT smoke test: DIVU.W/DIVS.W helpers ===\n");

    static const uint32_t edge_n[] = {
        0, 1, 2, 0xFFFF, 0x10000, 0x7FFF, 0x8000, 0xFFFFFFFF, 0x80000000,
        0x7FFFFFFF, 0xFFFF0000, 0x0000FFFF, 100, 0xFFFF8000
    };
    static const uint32_t edge_d[] = {
        0, 1, 2, 7, 0x7FFF, 0x8000, 0xFFFF, 0xFFFFFFFF, 0xFFFF8000, 10
    };

    for (unsigned i = 0; i < sizeof(edge_n)/4; i++)
        for (unsigned j = 0; j < sizeof(edge_d)/4; j++) {
            do_u(edge_n[i], edge_d[j] & 0xFFFFu);
            // The emitter sign-extends the divisor word before the call.
            do_s((int32_t)edge_n[i], (int32_t)(int16_t)edge_d[j]);
        }

    for (int i = 0; i < 200000; i++) {
        uint32_t n = rnd(), d = rnd();
        do_u(n, d & 0xFFFFu);
        do_s((int32_t)n, (int32_t)(int16_t)d);
        // Bias towards small dividends, where quotients actually fit.
        do_u(n & 0xFFFFF, (d & 0xFF) + 1);
        do_s((int32_t)(n & 0xFFFFF) - 0x7FFFF, (int32_t)(int16_t)((d & 0xFF) + 1));
    }

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    qemu_exit();
    return 0;
}
