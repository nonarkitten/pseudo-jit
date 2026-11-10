/// @file bbb_irq.c
/// @brief Interrupts on the BeagleBone through the block runtime (docs/design/BLOCK_RUNTIME.md section 7): the ARM performance monitor's cycle counter overflows every PERIOD
///        cycles and raises INTC line 3, which stands in for a level-7 (edge) device. The 68K guest spins in loops that never leave chained blocks (and one through calls);
///        its level-7 handler (autovector 31) reads the cycle counter with TRAP #14, subtracts the time the host latch saw the interrupt, and counts. Reported: interrupts
///        raised, delivered (levels that coalesce while one is pending count once), the latency from latch to handler (cycles: mean and worst), and what the interrupts cost the loop.
///        `make bbb-bbb_irq` (BLK=1).
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "mem_be.h"

extern void emit_opcode_table(void);
#define INTC 0x48200000u
#define R32(a) (*(volatile uint32_t *)(a))
#ifndef PERIOD
#define PERIOD 100000u                                   /* cycles between interrupts: 100 us (-DPERIOD=1000000 for a 1 ms tick) */
#endif

static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static uint8_t code_mem[2048] __attribute__((aligned(256)));
static uint8_t stack_mem[1024] __attribute__((aligned(16)));
static uint32_t vectors[64] __attribute__((aligned(4)));
static volatile uint32_t v_count, v_sum, v_max;         /* the guest's (big-endian) */
static volatile uint32_t t_latch;                       /* the latch's cycle count, big-endian for the guest */
static volatile uint32_t n_latch;                       /* host */
static volatile int armed;

/// the platform hook, called by the IRQ latch in IRQ mode: acknowledge the monitor, reload the counter, say "level 7" (an edge: the 68000's non-maskable level, delivered once; a lower level would stay posted until the device dropped it, and nothing here can)
#ifdef ON_BUFFEE
extern unsigned buffee_irq_c(void);     // the Buffee board's own handler: the UART (ESC x3 back to the bootloader), the Amiga's IPL and RESET lines
#endif
unsigned pjit_platform_irq(void)
{
#ifdef ON_BUFFEE
    if ((R32(INTC + 0x40) & 0x7F) != 3) return buffee_irq_c();     // another line than the performance monitor's
#endif
    uint32_t c; __asm__ volatile("mrc p15, 0, %0, c9, c13, 0" : "=r"(c));
    t_latch = __builtin_bswap32(c); n_latch++;
#ifdef IRQ_DEBUG
    if (n_latch == 1 || n_latch == 2 || n_latch == 3 || n_latch == 10 || n_latch == 100 || n_latch == 1000) printf("[irq %u] cycle %u flag %u level %u handled %u\n", (unsigned)n_latch, (unsigned)c, (unsigned)cpu->irq_flag, (unsigned)cpu->irq_level, (unsigned)__builtin_bswap32(v_count));
#endif
    __asm__ volatile("mcr p15, 0, %0, c9, c12, 3" :: "r"(2));                       /* PMOVSR: counter 1's overflow */
    __asm__ volatile("mcr p15, 0, %0, c9, c12, 5" :: "r"(1));                       /* PMSELR = 1 */
    __asm__ volatile("mcr p15, 0, %0, c9, c13, 2" :: "r"(armed ? (uint32_t)-PERIOD : 0));   /* PMXEVCNTR */
    R32(INTC + 0x48) = 1;                                                           /* NEWIRQAGR */
    __asm__ volatile("dsb");
    return armed ? 7 : 0;
}

static int hw;
static void w(uint16_t v) { code_mem[hw * 2] = v >> 8; code_mem[hw * 2 + 1] = (uint8_t)v; hw++; }
static void wl(uint32_t v) { w(v >> 16); w(v & 0xFFFF); }
static void at(int off) { hw = off / 2; }

static void handler(void)                               /* at code_mem + 0x400 */
{
    at(0x400);
    w(0x2F00); w(0x2F01);                               /* MOVE.L D0,-(A7) ; MOVE.L D1,-(A7) */
    w(0x2239); wl((uint32_t)&t_latch);                  /* MOVE.L t_latch.L,D1 (first: a later latch overwrites it) */
    w(0x4E4E);                                          /* TRAP #14: D0 = the cycle counter */
    w(0x9081);                                          /* SUB.L D1,D0 */
    w(0xD1B9); wl((uint32_t)&v_sum);                    /* ADD.L D0,sum.L */
    w(0xB0B9); wl((uint32_t)&v_max);                    /* CMP.L max.L,D0 */
    w(0x6306);                                          /* BLS.S +6 */
    w(0x23C0); wl((uint32_t)&v_max);                    /* MOVE.L D0,max.L */
    w(0x52B9); wl((uint32_t)&v_count);                  /* ADDQ.L #1,count.L */
#ifdef BURN
    w(0x2F02); w(0x343C); w(BURN - 1); w(0x51CA); w(0xFFFE); w(0x241F);   /* a context switch's worth of work: MOVE.L D2,-(A7) ; MOVE.W #BURN-1,D2 ; DBF D2,* ; MOVE.L (A7)+,D2 -> BURN more instructions */
#endif
    w(0x221F); w(0x201F);                               /* MOVE.L (A7)+,D1 ; MOVE.L (A7)+,D0 */
    w(0x4E73);                                          /* RTE */
}

static void arm_pmu(void)
{
    for (int i = 0; i < 4; i++) R32(INTC + 0x8C + 0x20 * i) = 0xFFFFFFFFu;          /* mask every line ... */
    R32(INTC + 0x68) = 0xFF; R32(INTC + 0x100 + 4 * 3) = 0; R32(INTC + 0x88) = 1u << 3;   /* ... but line 3 (the performance monitor) */
#ifdef ON_BUFFEE
    R32(INTC + 0x88 + 0x20 * 2) = 1u << (72 - 64);                                   /* and the UART's (line 72), so that ESC x3 still works */
#endif
    __asm__ volatile("mcr p15, 0, %0, c9, c12, 5" :: "r"(1));
    __asm__ volatile("mcr p15, 0, %0, c9, c13, 1" :: "r"(0x11u));                   /* counter 1 counts cycles */
    __asm__ volatile("mcr p15, 0, %0, c9, c13, 2" :: "r"((uint32_t)-PERIOD));
    __asm__ volatile("mcr p15, 0, %0, c9, c12, 3" :: "r"(0xFFFFFFFFu));
    __asm__ volatile("mcr p15, 0, %0, c9, c14, 1" :: "r"(2));                       /* PMINTENSET: counter 1 */
}
static void disarm_pmu(void) { __asm__ volatile("mcr p15, 0, %0, c9, c14, 2" :: "r"(2)); R32(INTC + 0x8C) = 1u << 3; }

static uint32_t run(const char *name, int variant, uint32_t n, int interrupts, uint32_t *delivered, uint32_t *lat_mean, uint32_t *lat_max)
{
    hw = 0;
    w(0x7C00);                                          /* MOVEQ #0,D6 */
    w(0x2A3C); wl(n);                                   /* MOVE.L #n,D5 */
    int loop = hw * 2;
    if (variant == 0) { w(0x5286); w(0xBC85); w(0x65FA); w(0x4E4F); }                   /* L: ADDQ.L #1,D6 ; CMP.L D5,D6 ; BCS.S L ; TRAP #15 */
    else { w(0x6106); w(0xBC85); w(0x65FA); w(0x4E4F); w(0x5286); w(0x4E75); }          /* L: BSR.S sub ; CMP.L D5,D6 ; BCS.S L ; TRAP #15 ; sub: ADDQ.L #1,D6 ; RTS */
    (void)loop;
    handler();
    for (int i = 0; i < 8; i++) { pjit_set_dn(i, 0); pjit_set_an(i, 0); }
    pjit_set_an(7, (uint32_t)(stack_mem + sizeof stack_mem));
    cpu->ssp = cpu->usp = __builtin_bswap32((uint32_t)(stack_mem + sizeof stack_mem));
    pjit_set_sr(0x2000);
    cpu->irq_level = 0; cpu->irq_flag = 0;
    v_count = v_sum = v_max = 0; n_latch = 0;
    pjit_cache_flush();
    if (interrupts) { armed = 1; arm_pmu(); }
    uint32_t t0 = bbb_pmu_cycles();
    pjit_run((uint16_t *)code_mem);
    uint32_t cyc = bbb_pmu_cycles() - t0;
    if (interrupts) { armed = 0; disarm_pmu(); }
    *delivered = __builtin_bswap32(v_count);
    *lat_mean = *delivered ? __builtin_bswap32(v_sum) / *delivered : 0;
    *lat_max = __builtin_bswap32(v_max);
    printf("[IRQ] %-28s %s: D6 = %u, %u cycles\n", name, interrupts ? "interrupts on " : "interrupts off", pjit_get_dn(6), cyc);
    return cyc;
}

int main(void)
{
    printf("=== interrupts through the block runtime on the BeagleBone ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    cpu->dr_threshold = DR_THRESHOLD_DEFAULT;
    pjit_irq_enable(1);
    cpu->vbr = (uint32_t)vectors;
    for (int v = 0; v < 64; v++) write_BE_long((uint32_t)&vectors[v], 0);
    write_BE_long((uint32_t)&vectors[31], (uint32_t)code_mem + 0x400);
    int bad = 0;
    for (int variant = 0; variant < 2; variant++) {
        uint32_t n = variant ? 1000000 : 2000000, d, mean, mx, d0, m0, x0;
        uint32_t base = run(variant ? "BSR / RTS loop" : "ADDQ ; CMP ; BCS loop", variant, n, 0, &d0, &m0, &x0);
        uint32_t with = run(variant ? "BSR / RTS loop" : "ADDQ ; CMP ; BCS loop", variant, n, 1, &d, &mean, &mx);
        uint32_t raised = n_latch;
        printf("[IRQ]   %u raised, %u delivered; latency latch -> handler: mean %u, worst %u cycles; the loop took %d cycles more (%d per interrupt)\n", raised, d, mean, mx,
               (int)(with - base), d ? (int)(with - base) / (int)d : 0);
        if (d == 0 || pjit_get_dn(6) != n) { printf("[FAIL] %s\n", d == 0 ? "no interrupt was delivered" : "the loop did not finish"); bad++; }
        if (d + 2 < raised / 2) { printf("[FAIL] fewer than half of the interrupts were delivered\n"); bad++; }
    }
    printf(bad ? "FAIL\n" : "PASS\n");
    return bad != 0;
}
