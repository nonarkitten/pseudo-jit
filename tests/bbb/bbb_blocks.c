/// @file bbb_blocks.c
/// @brief Times real generated code on the Cortex-A8: each straight-line segment of tests/data/blocks_data.h (the hottest runs of plain instructions of translated Dhrystone /
///        Tiny BASIC blocks, tools/study/blockbench.py) runs as a loop, cycles per iteration by differencing two lengths, so that the pipeline model in tools/study/cyclemodel.py
///        can be fitted to the hardware on the code the translator really emits. `make bbb-bbb_blocks`.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#ifndef SEGS_HEADER
#define SEGS_HEADER "blocks_data.h"
#endif
#include SEGS_HEADER

extern void arm_flush_cache(uintptr_t addr, uint32_t length);
static uint32_t code[1600] __attribute__((aligned(64)));
static uint8_t low_mem[1024 * 1024] __attribute__((aligned(1024 * 1024)));      // guest addresses 0..0xFFFFF (Tiny BASIC's absolute addresses)
static uint8_t scratch[0x80000] __attribute__((aligned(4096)));
uint32_t saved_sp;
extern uint32_t run_seg(uint32_t *code, uint32_t k, uint8_t *base);
__asm__(".text\n.arm\n.balign 64\n.global run_seg\nrun_seg:\n  push {r4-r11, lr}\n  ldr r3, =saved_sp\n  str sp, [r3]\n  mov r4, r1\n  mov r5, r2\n"
        "  add r6, r2, #0x4000\n  add r7, r2, #0x8000\n  add r8, r2, #0xC000\n  add r9, r2, #0x10000\n  add r10, r2, #0x14000\n  add r11, r2, #0x18000\n  add r12, r2, #0x1C000\n"
        "  add sp, r2, #0x40000\n  blx r0\n  ldr r3, =saved_sp\n  ldr sp, [r3]\n  pop {r4-r11, pc}\n  .ltorg\n");

int main(void)
{
    bbb_map_low((uint32_t)low_mem);
    for (uint32_t *p = (uint32_t *)(scratch + 0x2000 + 0x38000); p < (uint32_t *)(scratch + 0x2000 + 0x50000); p++) *p = (uint32_t)(scratch + 0x2000 + 0x22000);   // the stack: valid pointers to pop
    printf("=== generated code on the Cortex-A8: cycles per iteration of hot straight-line segments ===\n");
    for (int s = 0; s < NSEGS; s++) {
        unsigned n = segs[s].n;
        for (unsigned i = 0; i < n; i++) code[i] = segs[s].w[i];
        code[n] = 0xE2544001;                                              // subs r4, r4, #1
        code[n + 1] = 0x1A000000 | ((uint32_t)(-(int)(n + 1) - 2) & 0xFFFFFF);   // bne top
        code[n + 2] = 0xE12FFF1E;                                          // bx lr
        arm_flush_cache((uintptr_t)code, sizeof code);
        run_seg(code, 200, scratch + 0x2000);
        uint32_t t0 = bbb_pmu_cycles(); run_seg(code, 1000, scratch + 0x2000); uint32_t t1 = bbb_pmu_cycles();
        run_seg(code, 4000, scratch + 0x2000); uint32_t t2 = bbb_pmu_cycles();
        uint32_t a = t1 - t0, b = t2 - t1;
        printf("[SEG] %d %u %u\n", s, (unsigned)n, (unsigned)(((b - a) * 100) / 3000));
    }
    // all the segments one after the other: no loop-carried dependency between a segment's end and its own start, one loop branch per ~600 words
    unsigned n = 0;
    for (int s = 0; s < NSEGS; s++) for (unsigned i = 0; i < segs[s].n; i++) code[n++] = segs[s].w[i];
    code[n] = 0xE2544001; code[n + 1] = 0x1A000000 | ((uint32_t)(-(int)(n + 1) - 2) & 0xFFFFFF); code[n + 2] = 0xE12FFF1E;
    arm_flush_cache((uintptr_t)code, sizeof code);
    run_seg(code, 100, scratch + 0x2000);
    uint32_t t0 = bbb_pmu_cycles(); run_seg(code, 400, scratch + 0x2000); uint32_t t1 = bbb_pmu_cycles();
    run_seg(code, 1400, scratch + 0x2000); uint32_t t2 = bbb_pmu_cycles();
    printf("[MEGA] %u %u\n", n, (unsigned)(((t2 - t1) - (t1 - t0)) * 100 / 1000));
    printf("PASS\n");
    return 0;
}
