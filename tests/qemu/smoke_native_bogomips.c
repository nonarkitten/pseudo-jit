/// @file smoke_native_bogomips.c
/// @brief The same delay loop as smoke_bogomips.c written in native ARM,
///        as the baseline the JIT's number is measured against.
#include <stdint.h>
#include "pjit.h"

extern void qemu_exit(void);
extern uint32_t read_cntfrq(void);
extern uint32_t read_cntpct_lo(void);
extern void bogo_delay(uint32_t loops);

static uint32_t g_loops;
static uint32_t g_elapsed;

__attribute__((noinline)) double measure(void)
{
    uint32_t cntfrq = read_cntfrq();
    g_loops = 1;

    do {
        g_loops *= 2;
        uint32_t start = read_cntpct_lo();
        bogo_delay(g_loops);
        uint32_t end = read_cntpct_lo();
        g_elapsed = end - start;
    } while (g_elapsed < cntfrq);

    double duration = (double)g_elapsed / (double)cntfrq;
    return 2.0 * (double)g_loops / duration / 1.0e6;
}

int main(void)
{
    printf("=== PJIT smoke test: native ARM bogomips -- JIT comparison baseline ===\n");

    printf("[TEST] warming up (untimed pass, lets QEMU's TCG JIT settle)\n");
    measure();

    printf("[TEST] measuring\n");
    double bogomips = measure();

    printf("[TEST] loops = %u, elapsed ticks = %u, CNTFRQ = %u\n", g_loops, g_elapsed, read_cntfrq());
    printf("[TEST] Native ARM BogoMIPS %0.2f\n", bogomips);

    printf((bogomips > 0.0 && bogomips < 100000.0) ? "PASS\n" : "FAIL\n");
    qemu_exit();
    return 0;
}
