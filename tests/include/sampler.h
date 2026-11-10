/// @file sampler.h
/// @brief A PMU sampling profiler for the BeagleBone tests: counter 1 counts SAMPLE_EVENT (default: mispredicted branches; 0x11 = CPU cycles) and raises INTC line 3 every
///        SAMPLE_N events; the IRQ handler counts the interrupted PC in a table. sample_begin() / sample_end() bracket the guest (IRQs are unmasked in between; pjit_run
///        leaves them masked after each TRAP exit, so a harness that calls pjit_run repeatedly calls sample_begin() before each); sample_report() prints the top PCs with
///        the words around them (disassemble with `objdump -D -b binary -m arm`; PCs in the translated blocks are also shown as ring offsets).
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "blk.h"
#ifndef SAMPLE_N
#define SAMPLE_N 8
#endif
#ifndef SAMPLE_EVENT
#define SAMPLE_EVENT 0x10                   /* 0x10 branch mispredicted, 0x11 cycles */
#endif
#define INTC 0x48200000u
#define R32(a) (*(volatile uint32_t *)(a))
#define SAMPLE_STR(x) #x
#define SAMPLE_XSTR(x) SAMPLE_STR(x)
extern void Entry(void);
extern void sample_irq(void);
#ifndef SAMPLE_SLOTS
#define SAMPLE_SLOTS 2048
#endif
uint32_t sample_tab[SAMPLE_SLOTS * 2];                      // {pc, count}, open addressing
uint32_t sample_reload = (uint32_t)-SAMPLE_N;
uint32_t sample_total;
int sample_enable;

__asm__(".text\n.arm\n.balign 32\n.global sample_vectors\nsample_vectors:\n  b Entry\n  b Entry+4\n  b Entry+8\n  b Entry+12\n  b Entry+16\n  b Entry+20\n  b sample_irq\n  b Entry+28\n"
        "sample_irq:\n  sub lr, lr, #4\n  push {r0-r3, r12}\n"
        "  mov r0, #2\n  mcr p15, 0, r0, c9, c12, 3\n"                                   // PMOVSR: clear counter 1's overflow
        "  mov r0, #1\n  mcr p15, 0, r0, c9, c12, 5\n"                                   // PMSELR = 1
        "  ldr r1, =sample_reload\n  ldr r1, [r1]\n  mcr p15, 0, r1, c9, c13, 2\n"      // PMXEVCNTR = -N
        "  ldr r2, =sample_tab\n  lsr r0, lr, #2\n  ldr r3, =" SAMPLE_XSTR(SAMPLE_SLOTS - 1) "\n  and r0, r0, r3\n"
        "  mov r12, #0\n"
        "1:\n  ldr r1, [r2, r0, lsl #3]\n  cmp r1, lr\n  beq 2f\n  cmp r1, #0\n  beq 3f\n  add r0, r0, #1\n  and r0, r0, r3\n  add r12, r12, #1\n  cmp r12, r3\n  bls 1b\n  b 4f\n"      // (a full table drops the sample instead of looping forever)
        "3:\n  str lr, [r2, r0, lsl #3]\n"
        "2:\n  add r1, r2, r0, lsl #3\n  ldr r3, [r1, #4]\n  add r3, r3, #1\n  str r3, [r1, #4]\n"
        "4:\n"
        "  ldr r1, =sample_total\n  ldr r3, [r1]\n  add r3, r3, #1\n  str r3, [r1]\n"
        "  ldr r0, =0x48200000\n  mov r1, #1\n  str r1, [r0, #0x48]\n  dsb\n"            // INTC: NEWIRQAGR
        "  pop {r0-r3, r12}\n  movs pc, lr\n  .ltorg\n");
extern char sample_vectors[];
static uint32_t vbar_old; static int vbar_saved;

static void sample_begin(void)
{
    if (!vbar_saved) { __asm__ volatile("mrc p15, 0, %0, c12, c0, 0" : "=r"(vbar_old)); vbar_saved = 1; }
    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0\n isb" :: "r"(sample_vectors));
    for (int i = 0; i < 4; i++) R32(INTC + 0x8C + 0x20 * i) = 0xFFFFFFFFu;
    R32(INTC + 0x68) = 0xFF; R32(INTC + 0x100 + 4 * 3) = 0; R32(INTC + 0x88) = 1u << 3;   // line 3 (the performance monitor)
    __asm__ volatile("mcr p15, 0, %0, c9, c12, 5" :: "r"(1));
    __asm__ volatile("mcr p15, 0, %0, c9, c13, 1" :: "r"((uint32_t)SAMPLE_EVENT));          // PMXEVTYPER: counter 1 counts this
    __asm__ volatile("mcr p15, 0, %0, c9, c13, 2" :: "r"(sample_reload));
    __asm__ volatile("mcr p15, 0, %0, c9, c12, 3" :: "r"(0xFFFFFFFFu));
    __asm__ volatile("mcr p15, 0, %0, c9, c14, 1" :: "r"(2));                       // PMINTENSET: counter 1
    __asm__ volatile("cpsie i");
}
static void sample_end(void)
{
    __asm__ volatile("cpsid i");
    __asm__ volatile("mcr p15, 0, %0, c9, c14, 2" :: "r"(2));                       // PMINTENCLR
    R32(INTC + 0x8C) = 1u << 3;
    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0\n isb" :: "r"(vbar_old));
}
static void sample_report(void)
{
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    uint32_t ring = pjit_blk.bc.ring, ring_end = ring + pjit_blk.bc.mask + 1;
#else
    uint32_t ring = 0, ring_end = 0;                                                // (the slot design: every sample is "text", whose symbol the host looks up)
#endif
    printf("[SAMPLE] %u samples (one per %d events), ring %08X-%08X\n", sample_total, SAMPLE_N, ring, ring_end);
#if defined(PJIT_BLK) || defined(PJIT_TIER)
    for (uint32_t i = 0; i <= pjit_blk.bc.set_mask * 4 + 3; i++) {                  // the live blocks: guest address -> code address
        uint32_t *t = (uint32_t *)pjit_blk.bc.tags + 2 * i;
        if (t[0] && (pjit_blk.bc.hw - t[1]) <= pjit_blk.bc.mask) printf("[T] %08X %08X\n", t[0], ring + (t[1] & pjit_blk.bc.mask));
    }
#endif
    for (int i = 0; i < SAMPLE_SLOTS; i++) if (sample_tab[2 * i + 1]) printf("[S] %08X %u\n", sample_tab[2 * i], sample_tab[2 * i + 1]);     // everything, for offline aggregation
    for (int k = 0; k < 28; k++) {
        int best = -1; uint32_t bc = 0;
        for (int i = 0; i < SAMPLE_SLOTS; i++) if (sample_tab[2 * i + 1] > bc) { bc = sample_tab[2 * i + 1]; best = i; }
        if (best < 0) break;
        uint32_t pc = sample_tab[2 * best];
        printf("[SAMPLE] %08X %5u %2u%% %s", pc, bc, bc * 100 / (sample_total ? sample_total : 1), (pc >= ring && pc < ring_end) ? "ring" : "text");
        if (pc >= ring && pc < ring_end) printf("+%X", pc - ring);
        printf("  ");
        for (int w = -6; w <= 1; w++) printf("%08X ", ((uint32_t *)pc)[w]);
        printf("\n");
        sample_tab[2 * best + 1] = 0;
    }
}
