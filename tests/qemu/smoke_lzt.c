/// @file smoke_lzt.c
/// @brief The table image decoder (pjit/lzt.s, format tools/pack/lzt.py): (1) lz4_block against liblz4's output for a vector with every length / offset case, (2) lz4_block fuzzed
///        against a small C compressor (random buffers with runs and repeats at offsets 1..300, unaligned source and destination, the slack bytes untouched past 16),
///        (3) the NEON 16x16 untranspose against a C model, (4) lzt_unpack on the vector, a bad magic.
#include <stdint.h>
#include "pjit_libc.h"
#include "cp15.h"
#include "lzt_vec.h"
#include "lzt.h"


static uint32_t mmu_pagetable[4096] __attribute__((aligned(16384)));
static uint8_t  in[8192] __attribute__((aligned(16))), cmp[16384] __attribute__((aligned(16))), out[16384 + 64] __attribute__((aligned(16)));
static uint8_t  planes[16384 + 64] __attribute__((aligned(16))), rec[16384 + 64] __attribute__((aligned(16)));
static int pass, fail;
static void check(const char *what, uint32_t got, uint32_t want)
{
    if (got == want) pass++;
    else { if (fail < 20) printf("[FAIL] %s: got 0x%08X want 0x%08X\n", what, got, want); fail++; }
}
static uint32_t rng = 12345;
static uint32_t rnd(void) { rng = rng * 1664525u + 1013904223u; return rng >> 8; }

static uint32_t rd32(const uint8_t *p) { return p[0] | p[1] << 8 | p[2] << 16 | (uint32_t)p[3] << 24; }
/// a greedy LZ4 block compressor (the spec's end rules: the last match starts 12+ bytes before the end and ends 5+ bytes before it)
static uint32_t compress(const uint8_t *s, uint32_t n, uint8_t *d)
{
    static uint32_t tab[4096];
    uint8_t *o = d; uint32_t anchor = 0, i = 0;
    for (int k = 0; k < 4096; k++) tab[k] = 0;
    while (n >= 13 && i + 12 <= n) {
        uint32_t h = (rd32(s + i) * 2654435761u) >> 20, cand = tab[h]; tab[h] = i + 1;
        if (cand && i - (cand - 1) <= 65535 && rd32(s + cand - 1) == rd32(s + i)) {
            uint32_t m = cand - 1, len = 4, lit = i - anchor;
            while (i + len < n - 5 && s[m + len] == s[i + len]) len++;
            *o++ = (uint8_t)((lit < 15 ? lit : 15) << 4 | (len - 4 < 15 ? len - 4 : 15));
            if (lit >= 15) { uint32_t x = lit - 15; while (x >= 255) { *o++ = 255; x -= 255; } *o++ = (uint8_t)x; }
            for (uint32_t k = 0; k < lit; k++) *o++ = s[anchor + k];
            *o++ = (uint8_t)(i - m); *o++ = (uint8_t)((i - m) >> 8);
            if (len - 4 >= 15) { uint32_t x = len - 4 - 15; while (x >= 255) { *o++ = 255; x -= 255; } *o++ = (uint8_t)x; }
            i += len; anchor = i;
        } else i++;
    }
    uint32_t lit = n - anchor;
    *o++ = (uint8_t)((lit < 15 ? lit : 15) << 4);
    if (lit >= 15) { uint32_t x = lit - 15; while (x >= 255) { *o++ = 255; x -= 255; } *o++ = (uint8_t)x; }
    for (uint32_t k = 0; k < lit; k++) *o++ = s[anchor + k];
    return (uint32_t)(o - d);
}

int main(void)
{
    for (uint32_t i = 0; i < 4096; i++) mmu_pagetable[i] = ((i << 20) & 0xFFF00000u) | 0xC0Eu;     // QEMU faults unaligned accesses with the MMU off
    CP15TtbCtlTtb0Config(); CP15Ttb0Set((uint32_t)mmu_pagetable); CP15DomainAccessClientSet();
    CP15TlbInvalidate(); CP15MMUEnable();
    lzt_neon_enable();
    printf("=== PJIT smoke test: table image decoder (LZ4 + NEON untranspose) ===\n");

    // (4) the reference compressor's image of the vector
    uint32_t raw_n = sizeof lzt_raw;
    check("magic", lzt_unpack((const uint8_t *)lzt_img, out, planes), raw_n);
    int bad = 0; for (uint32_t i = 0; i < raw_n; i++) bad += out[i] != lzt_raw[i];
    check("vector round trip (bytes wrong)", bad, 0);
    for (uint32_t i = raw_n; i < (raw_n + 255) / 256 * 256; i++) bad += out[i] != 0;
    check("padding is zero", bad, 0);
    static uint8_t img2[sizeof lzt_img + 16] __attribute__((aligned(16)));
    for (uint32_t i = 0; i < sizeof lzt_img; i++) img2[i] = lzt_img[i];
    img2[0] ^= 1; check("bad magic", lzt_unpack(img2, out, planes), 0);

    // (5) a bundle: the same data through plain / 4 / 8 / 16 byte planes and the best choice, ascending offsets
    static uint8_t region[LZTB_REGION + 16] __attribute__((aligned(16)));
    for (uint32_t i = 0; i < sizeof region; i++) region[i] = 0xCC;
    check("bundle", (uint32_t)lztb_load(lztb_img, region, planes), 0);
    uint32_t part = LZTB_REGION / 5; bad = 0;
    for (int k = 0; k < 5; k++) for (uint32_t i = 0; i < raw_n; i++) bad += region[k * part + i] != lzt_raw[i];
    check("bundle parts (bytes wrong)", bad, 0);
    for (uint32_t i = 0; i < part - raw_n; i++) bad += region[part - 1 - i] != 0;
    check("bundle padding", bad, 0);
    { static uint8_t t[sizeof lztb_img] __attribute__((aligned(16))); for (uint32_t i = 0; i < sizeof t; i++) t[i] = lztb_img[i]; t[1] ^= 1; check("bundle bad magic", (uint32_t)lztb_load(t, region, planes), (uint32_t)-1); }

    // (3) the untranspose against the model
    for (int round = 0; round < 8; round++) {
        uint32_t records = 16 * (1 + round * 5);
        for (uint32_t i = 0; i < records * 16; i++) planes[i] = (uint8_t)rnd();
        lzt_untranspose(planes, rec, records);
        bad = 0; for (uint32_t i = 0; i < records; i++) for (int k = 0; k < 16; k++) bad += rec[i * 16 + k] != planes[k * records + i];
        check("untranspose", bad, 0);
        lzt_untranspose8(planes, rec, records * 2);                                       // 8-byte records: 8 planes of 2*records
        bad = 0; for (uint32_t i = 0; i < 2 * records; i++) for (int k = 0; k < 8; k++) bad += rec[i * 8 + k] != planes[k * 2 * records + i];
        check("untranspose8", bad, 0);
        lzt_untranspose4(planes, rec, records * 4);
        bad = 0; for (uint32_t i = 0; i < 4 * records; i++) for (int k = 0; k < 4; k++) bad += rec[i * 4 + k] != planes[k * 4 * records + i];
        check("untranspose4", bad, 0);
    }

    // (2) lz4_block fuzz
    static const uint16_t offs[] = { 1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 100, 300 };
    for (int t = 0; t < 400; t++) {
        uint32_t n = 1 + rnd() % 3000, len = 0;
        while (len < n) {
            uint32_t kind = rnd() % 4, l = 1 + rnd() % (kind == 0 ? 40 : 700);
            if (len + l > n) l = n - len;
            if (kind == 0 || len < 300) for (uint32_t k = 0; k < l; k++) in[len++] = (uint8_t)rnd();
            else if (kind == 1) { uint8_t b = (uint8_t)rnd(); for (uint32_t k = 0; k < l; k++) in[len++] = b; }
            else { uint32_t o = offs[rnd() % 13]; for (uint32_t k = 0; k < l; k++, len++) in[len] = in[len - o]; }
        }
        uint32_t cn = compress(in, n, cmp);
        uint32_t so = rnd() & 3, dof = rnd() & 3;                                         // unaligned source and destination
        for (uint32_t k = cn; k-- > 0;) cmp[so + k] = cmp[k];
        for (uint32_t k = 0; k < n + 32; k++) out[dof + k] = 0x5A;
        uint32_t got = lz4_block(cmp + so, cn, out + dof);
        bad = got != n; for (uint32_t k = 0; k < n; k++) bad += out[dof + k] != in[k];
        for (uint32_t k = n + 16; k < n + 32; k++) bad += out[dof + k] != 0x5A;            // nothing beyond the 16 bytes of slack
        check("fuzz", bad, 0);
    }
    printf("[RUN] %d checks\n", pass + fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    return fail != 0;
}
