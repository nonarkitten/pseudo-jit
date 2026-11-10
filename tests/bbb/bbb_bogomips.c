/// @file bbb_bogomips.c
/// @brief The 68K delay loop (SUBQ.L #1,Dn / BNE.S) on a BeagleBone, interpretive and out of the DR cache, timed with the ARM cycle
///        counter: cycles per loop iteration (2 68K instructions) from the difference of two loop counts. Compare with the native
///        loop on the same core under Linux (tools/study/native_bogomips.c). `make bbb-bbb_bogomips`.
#include <stdint.h>
#include "pjit.h"
#include "mem_be.h"
#include "bbb.h"

extern void emit_opcode_table(void);

#define N_A 1000000u
#define N_B 3000000u

#define REG(a) (*(volatile uint32_t *)(a))
// MPU DPLL (AM335x TRM, CM_WKUP at 0x44E00400): CLKOUT = M / (N+1) * 24 MHz / M2.  DMTIMER2 counts the 24 MHz oscillator (CLKSEL_TIMER2 = 1),
// through the prescaler if TCLR has it on (PRE, bit 5: divide by 2^(PTV+1), PTV = bits 4:2).
static void clock_report(void)
{
    uint32_t sel = REG(0x44E0042Cu), m2 = REG(0x44E004A8u) & 0x1F, mode = REG(0x44E00488u) & 7;
    uint32_t M = (sel >> 8) & 0x7FF, N = sel & 0x7F;
    uint32_t tclr = REG(0x48040038u), div = (tclr & 0x20) ? 2u << ((tclr >> 2) & 7) : 1u;
    printf("[CLK] DPLL_MPU mode %u (4 = MN bypass, 7 = locked), M=%u N=%u M2=%u -> %u MHz\n", mode, M, N, m2, m2 ? M * 24 / (N + 1) / m2 : 0);
    uint32_t c0 = bbb_pmu_cycles(), t0 = REG(0x4804003Cu);
    while (REG(0x4804003Cu) - t0 < 2400000u / div) ;
    uint32_t c1 = bbb_pmu_cycles(), t1 = REG(0x4804003Cu);
    printf("[CLK] %u PMU cycles in %u DMTIMER2 ticks (TCLR %02X, 24 MHz / %u): %u MHz measured\n", c1 - c0, t1 - t0, tclr, div,
           (uint32_t)(((uint64_t)(c1 - c0) * 24 / div) / (t1 - t0)));
}

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t code_mem[64] __attribute__((aligned(256)));
static uint8_t stack_mem[256] __attribute__((aligned(16)));

static void put16(int off, uint16_t v) { code_mem[off] = v >> 8; code_mem[off + 1] = v; }

// MOVE.L #n,D5 ; loop: SUBQ.L #1,D5 ; BNE.S loop ; TRAP #15
static void load(uint32_t n)
{
    put16(0, 0x2A3C); put16(2, n >> 16); put16(4, n & 0xFFFF);
    put16(6, 0x5385); put16(8, 0x66FC); put16(10, 0x4E4F);
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    uint32_t sp = (uint32_t)stack_mem + sizeof(stack_mem) - 16;
    pjit_set_an(7, sp);
    cpu->ssp = cpu->usp = __builtin_bswap32(sp);
    pjit_set_sr(0x2700);
    pjit_cache_flush();
}

static uint32_t timed(uint32_t n)
{
    load(n);
    uint32_t t0 = bbb_pmu_cycles();
    pjit_run((uint16_t *)code_mem);
    uint32_t t1 = bbb_pmu_cycles();
    if (pjit_get_dn(5) != 0) printf("[ERR] D5 = %u at exit\n", pjit_get_dn(5));
    return t1 - t0;
}

static void report(const char *tag)
{
    uint32_t a = timed(N_A), b = timed(N_B);
    uint32_t per_x100 = (uint32_t)(((uint64_t)(b - a) * 100) / (N_B - N_A));
    printf("[%s] %u -> %u cycles for %u -> %u iterations: %u.%02u cycles per iteration, %u.%02u per 68K instruction\n", tag, a, b,
           N_A, N_B, per_x100 / 100, per_x100 % 100, per_x100 / 200, (per_x100 / 2) % 100);
    printf("[%s] BogoMIPS (2 68K instructions per iteration) at 1000 MHz: %u.%02u\n", tag, 200000u / per_x100, (200000u * 100 / per_x100) % 100);
}

int main(void)
{
    printf("=== PJIT bogomips on the BeagleBone (PMU cycles) ===\n");
    clock_report();
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    cpu->dr_threshold = 0xFFFFFFFFu;
    report("interp");
    cpu->dr_threshold = DR_THRESHOLD_DEFAULT;
    report("cached");
    printf("PASS\n");
    return 0;
}
