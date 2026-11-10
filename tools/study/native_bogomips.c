/* The delay loop of tests/bbb/bbb_bogomips.c in native ARM, under Linux, as the baseline on the same core.
   gcc -O1 -march=armv7-a tools/study/native_bogomips.c -o native_bogomips   (on the board) */
#include <stdio.h>
#include <stdint.h>
#include <time.h>
static double now(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return t.tv_sec + t.tv_nsec * 1e-9; }
static void __attribute__((noinline)) delay(uint32_t n) { __asm__ volatile("1: subs %0, %0, #1\n bne 1b" : "+r"(n) : : "cc"); }
int main(void)
{
    uint32_t n = 1 << 20; double d;
    do { n *= 2; double t = now(); delay(n); d = now() - t; } while (d < 1.0);
    printf("native: %u iterations in %.3f s = %.2f BogoMIPS (2 instructions per iteration)\n", n, d, 2.0 * n / d / 1e6);
    return 0;
}
