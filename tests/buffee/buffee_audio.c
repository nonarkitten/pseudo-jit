/// @file buffee_audio.c
/// @brief Does Paula's audio keep time? Without Kickstart: channel k plays a ramp from chip RAM by DMA, LEN[k] words at a period of PER[k] colour
///        clocks, and its interrupts (one per pass of the sample, INTREQ bits 7-10) are timed against the Cortex's cycle counter for SECONDS
///        (default 4). By the HRM a pass is LEN x 2 samples x PER colour clocks, so channel k's interrupts come every 4 x LEN x PER / bus clock
///        seconds (the bus clock from the boot info, else a PAL 7.09379 MHz). Then CPU-driven output with DMA off: a write to AUD0DAT gives an
///        interrupt at once and, while the CPU leaves the data register alone, one every 2 x PER colour clocks. VOL (default 0) is the volume.
///        `AUDIO` lines; `AUDIO RESULT` the worst channel's error in ppm and the interrupts missing or extra.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

#ifndef SECONDS
#define SECONDS 4
#endif
#ifndef VOL
#define VOL 0
#endif

#define CW(off, v) write_BE_word(0xDFF000u + (off), (v))
#define CR(off)    read_BE_word(0xDFF000u + (off))
#define INTREQR    0x01E
#define INTREQ     0x09C
#define INTENA     0x09A
#define DMACON     0x096
#define AUD(k, r)  (0x0A0 + 16 * (k) + (r))     /* r: 0 LCH, 2 LCL, 4 LEN, 6 PER, 8 VOL, A DAT */
#define SAMPLES    0x020000u                     /* chip RAM: channel k's ramp at SAMPLES + 0x1000 k */

static const uint32_t LEN[4] = { 100, 77, 50, 33 }, PER[4] = { 200, 301, 450, 997 };

static uint32_t bus_hz(void)
{
    const volatile uint8_t *bi = BUFFEE_BOOTINFO;
    uint32_t hz = *(const volatile uint32_t *)bi == 0x544F4F42u ? *(const volatile uint32_t *)(bi + 32) : 0;
    return hz >= 4000000u && hz <= 17000000u ? hz : 7093790u;
}

int main(void)
{
    uint32_t hz = bus_hz(), first[4] = { 0 }, last[4] = { 0 }, count[4] = { 0 }, t0, worst = 0, odd = 0;

    printf("AUDIO bus clock %u Hz (%s), volume %u, %u s\n", (unsigned)hz, hz == 7093790u ? "PAL nominal" : "from the boot info", VOL, SECONDS);
    CW(DMACON, 0x000F);                                     /* audio DMA off */
    CW(INTENA, 0x0780);                                     /* audio interrupts off: polled */
    CW(INTREQ, 0x0780);
    for (int k = 0; k < 4; k++) {
        uint32_t a = SAMPLES + 0x1000u * (uint32_t)k;
        for (uint32_t i = 0; i < LEN[k]; i++) write_BE_word(a + 2 * i, (uint16_t)((i * 0x0402u) ^ 0x8080u));
        CW(AUD(k, 0), (uint16_t)(a >> 16)); CW(AUD(k, 2), (uint16_t)a);
        CW(AUD(k, 4), (uint16_t)LEN[k]); CW(AUD(k, 6), (uint16_t)PER[k]); CW(AUD(k, 8), VOL);
    }
    CW(DMACON, 0x8200 | 0x000F);                            /* DMAEN and AUD0-3 */

    t0 = bbb_pmu_cycles();
    while (bbb_pmu_cycles() - t0 < SECONDS * 1000000000u) {
        uint16_t r = CR(INTREQR);
        if (!(r & 0x0780)) continue;
        uint32_t now = bbb_pmu_cycles();
        for (int k = 0; k < 4; k++)
            if (r & (0x0080u << k)) {
                if (!count[k]) first[k] = now;
                last[k] = now;
                count[k]++;
            }
        CW(INTREQ, r & 0x0780);
    }
    CW(DMACON, 0x000F);

    for (int k = 0; k < 4; k++) {
        uint64_t want_ns = 4ull * LEN[k] * PER[k] * 1000000000ull / hz;
        uint64_t span = (uint64_t)(last[k] - first[k]);                     /* ns (the counter runs at 1 GHz) */
        uint32_t n = count[k] > 1 ? count[k] - 1 : 0;
        uint64_t got_ns = n ? span / n : 0;
        uint32_t expect = (uint32_t)((uint64_t)SECONDS * 1000000000ull / want_ns);
        int32_t ppm = n ? (int32_t)(((int64_t)got_ns - (int64_t)want_ns) * 1000000 / (int64_t)want_ns) : 999999;
        uint32_t a = ppm < 0 ? (uint32_t)-ppm : (uint32_t)ppm;
        if (a > worst) worst = a;
        if (count[k] + 2 < expect || count[k] > expect + 2) odd++;
        printf("AUDIO channel %d: length %u, period %u: %u interrupts (about %u expected), every %u.%03u ms (HRM %u.%03u ms), %c%u ppm\n", k, (unsigned)LEN[k],
               (unsigned)PER[k], (unsigned)count[k], (unsigned)expect, (unsigned)(got_ns / 1000000), (unsigned)(got_ns / 1000 % 1000), (unsigned)(want_ns / 1000000),
               (unsigned)(want_ns / 1000 % 1000), ppm < 0 ? '-' : '+', (unsigned)a);
    }

    /* CPU-driven output: DMA off, the CPU writes the data register */
    {
        uint32_t per = 500, t[4], n = 0, want_ns = (uint32_t)(4ull * per * 1000000000ull / hz);     /* 2 samples x per colour clocks */
        CW(INTREQ, 0x0080);
        CW(AUD(0, 6), (uint16_t)per);
        CW(AUD(0, 10), 0x4000);
        t0 = bbb_pmu_cycles();
        while (n < 4 && bbb_pmu_cycles() - t0 < 100000000u)
            if (CR(INTREQR) & 0x0080) { t[n++] = bbb_pmu_cycles(); CW(INTREQ, 0x0080); }
        printf("AUDIO CPU-driven: %u interrupts in 100 ms", (unsigned)n);
        if (n) printf(", the first %u us after the write", (unsigned)((t[0] - t0) / 1000));
        for (uint32_t i = 1; i < n; i++) printf(", then %u us", (unsigned)((t[i] - t[i - 1]) / 1000));
        printf(" (HRM: at once, then every %u us)\n", (unsigned)(want_ns / 1000));
        if (n < 4) odd++;
        CW(INTREQ, 0x8080);                                  /* pending: at the end of its period the channel returns to idle */
        t0 = bbb_pmu_cycles();
        while (bbb_pmu_cycles() - t0 < 2000000u) ;
        CW(INTREQ, 0x0080);
    }
    for (int k = 0; k < 4; k++) CW(AUD(k, 8), 0);
    printf("AUDIO RESULT: the worst channel is %u ppm off the HRM's interval; %u counts off\n", (unsigned)worst, (unsigned)odd);
    return 0;
}
