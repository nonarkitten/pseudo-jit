/// @file bbb_micro.c
/// @brief Cortex-A8 microbenchmarks for the building blocks of the emitted sequences, in cycles per body (PMU cycle counter), each in a
///        tight loop of 8 copies, in LE and in BE (`setend be`, the mode JIT'd code runs in). `make bbb-bbb_micro`.
///        Question it answers: where do the ~9.5 cycles per 68K instruction go on real silicon? (QEMU cannot say.)
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"

uint32_t micro_scratch[64] __attribute__((aligned(64)));

// name, setup-mode (LE / BE), body (one copy)
#define LOOP(name, endian_on, endian_off, body)                                                                             \
    __asm__(".text\n.arm\n.global " #name "\n" #name ":\n"                                                                   \
            "  push {r4-r8, lr}\n  ldr r5, =micro_scratch+128\n  mov r1, #1\n  mov r2, #0\n  mov r6, #0\n  mov r7, #0\n"      \
            "  adr r8, " #name "_f\n" endian_on                                                                              \
            "1:\n" body body body body body body body body                                                                   \
            "  subs r0, r0, #1\n  bne 1b\n" endian_off                                                                       \
            "  pop {r4-r8, pc}\n"                                                                                            \
            "  .ltorg\n" #name "_f:\n  bx lr\n")
// the SUBQ.L #1,D5 sequence as the generator emits it (without the final bx lr)
#define SUBQ "  mov r1, #1\n  ldr r0, [r5, #-0x2c]\n  subs r0, r0, r1\n  mrs r3, apsr\n  eor r3, r3, #0x20000000\n" \
             "  msr apsr_nzcvq, r3\n  str r0, [r5, #-0x2c]\n  movhs r0, #0x10\n  movlo r0, #0\n  strb r0, [r5, #-0x43]\n"
#define NONE ""
#define BE "  setend be\n"
#define LE "  setend le\n"
// the loop counter is r0 and SUBQ clobbers r0: keep the counter in r4 for those cases
#define LOOP4(name, endian_on, endian_off, body)                                                                            \
    __asm__(".text\n.arm\n.global " #name "\n" #name ":\n"                                                                   \
            "  push {r4-r8, lr}\n  mov r4, r0\n  ldr r5, =micro_scratch+128\n  mov r1, #1\n  mov r2, #0\n"                    \
            "  adr r8, " #name "_f\n" endian_on                                                                              \
            "1:\n" body body body body body body body body                                                                   \
            "  subs r4, r4, #1\n  bne 1b\n" endian_off                                                                       \
            "  pop {r4-r8, pc}\n"                                                                                            \
            "  .ltorg\n" #name "_f:\n  bx lr\n" #name "_g:\n" SUBQ "  bx lr\n")

LOOP(m_nop_le,   NONE, NONE, "  add r2, r2, #1\n");                                  // dependent chain of 1-cycle ALU ops
LOOP(m_nop_be,   BE,   LE,   "  add r2, r2, #1\n");
LOOP(m_mrsmsr_le, NONE, NONE, "  mrs r3, apsr\n  msr apsr_nzcvq, r3\n");
LOOP(m_mrsmsr_be, BE,   LE,   "  mrs r3, apsr\n  msr apsr_nzcvq, r3\n");
LOOP(m_mrs_le,   NONE, NONE, "  mrs r3, apsr\n  add r2, r2, r3\n");                  // mrs alone
LOOP(m_msr_le,   NONE, NONE, "  msr apsr_nzcvq, r3\n  add r2, r2, #1\n");            // msr alone
LOOP(m_blbx_le,  NONE, NONE, "  blx r8\n");                                          // call / return pair (indirect call, return-stack predicted)
LOOP(m_blbx_be,  BE,   LE,   "  blx r8\n");
LOOP(m_setend,   NONE, NONE, "  setend be\n  setend le\n");
LOOP4(m_subq_le, NONE, NONE, SUBQ);                                                  // the SUBQ.L #1,D5 sequence, inline
LOOP4(m_subq_be, BE,   LE,   SUBQ);
LOOP4(m_subqcall_be, BE, LE, "  bl 2f\n  b 3f\n2:\n" SUBQ "  bx lr\n3:\n");           // the same, called with bl / bx lr
LOOP4(m_ldrstr_be, BE, LE,   "  ldr r0, [r5, #-0x2c]\n  add r0, r0, r1\n  str r0, [r5, #-0x2c]\n");  // memory-resident register RMW
LOOP4(m_flags_be, BE,  LE,   "  ldr r0, [r5, #-0x2c]\n  subs r0, r0, r1\n  str r0, [r5, #-0x2c]\n");  // the same with flags set, no CPSR moves
LOOP4(m_strb_be, BE,   LE,   "  movhs r0, #0x10\n  movlo r0, #0\n  strb r0, [r5, #-0x43]\n");       // the X-flag byte store


// ---- v2: the call structure, and the carry flip without mrs ----
// 8 calls of one direct `bl` to a shared `bx lr` function
LOOP(m_bl1_be, BE, LE, "  bl m_bl1_be_f\n");
// 8 *different* targets, each a 1-instruction body + bx lr (the cached shape: slot = bl, body shared elsewhere)
#define F8(name, endian_on, endian_off) \
    __asm__(".text\n.arm\n.global " #name "\n" #name ":\n  push {r4-r8, lr}\n  ldr r5, =micro_scratch+128\n  mov r1, #1\n  mov r2, #0\n" endian_on \
            "1:\n  bl " #name "_1\n  bl " #name "_2\n  bl " #name "_3\n  bl " #name "_4\n  bl " #name "_5\n  bl " #name "_6\n  bl " #name "_7\n  bl " #name "_8\n" \
            "  subs r0, r0, #1\n  bne 1b\n" endian_off "  pop {r4-r8, pc}\n  .ltorg\n" \
            #name "_1: add r2, r2, #1\n bx lr\n" #name "_2: add r2, r2, #1\n bx lr\n" #name "_3: add r2, r2, #1\n bx lr\n" #name "_4: add r2, r2, #1\n bx lr\n" \
            #name "_5: add r2, r2, #1\n bx lr\n" #name "_6: add r2, r2, #1\n bx lr\n" #name "_7: add r2, r2, #1\n bx lr\n" #name "_8: add r2, r2, #1\n bx lr\n")
F8(m_bl8_be, BE, LE);
// the same 8 bodies inline, no calls
LOOP(m_inl8_be, BE, LE, "  add r2, r2, #1\n");
// the carry flip: current, and built from condition codes (no mrs)
#define SUBS_BASE "  subs r2, r2, r1\n"
LOOP4(m_flip_mrs_be, BE, LE, SUBS_BASE "  mrs r3, apsr\n  eor r3, r3, #0x20000000\n  msr apsr_nzcvq, r3\n");
LOOP4(m_flip_cc6_be, BE, LE, SUBS_BASE "  mov r3, #0\n  orrmi r3, r3, #0x80000000\n  orreq r3, r3, #0x40000000\n  orrcc r3, r3, #0x20000000\n  orrvs r3, r3, #0x10000000\n  msr apsr_nzcvq, r3\n");
LOOP4(m_flip_cc5_be, BE, LE, SUBS_BASE "  and r3, r2, #0x80000000\n  orreq r3, r3, #0x40000000\n  orrcc r3, r3, #0x20000000\n  orrvs r3, r3, #0x10000000\n  msr apsr_nzcvq, r3\n");
LOOP4(m_flip_none_be, BE, LE, SUBS_BASE);                                           // flags only, for the floor
// a taken conditional branch, predicted (always taken to the next instruction) and a mispredicting pattern
LOOP(m_bne_taken, BE, LE, "  cmp r2, r2\n  beq 8f\n8:\n");
// an indirect `bx r8`-style jump to the next instruction (the router's shape), and a return through a computed register
LOOP(m_bxreg_be, BE, LE, "  adr r3, 7f\n  bx r3\n7:\n");
extern uint32_t m_bl1_be(uint32_t), m_bl8_be(uint32_t), m_inl8_be(uint32_t), m_flip_mrs_be(uint32_t), m_flip_cc6_be(uint32_t), m_flip_cc5_be(uint32_t),
    m_flip_none_be(uint32_t), m_bne_taken(uint32_t), m_bxreg_be(uint32_t);

// ---- v3: what a direct `b` costs, for the "branch to a landing zone and branch back" layout ----
LOOP(m_b_next, BE, LE, "  b 6f\n6:\n");                                                       // taken b to the next instruction
// b out to a per-site continuation (2 instrs), which ends with a direct b back to the instruction after the site: 2 taken branches per body
#define BOUT(name, endian_on, endian_off) \
    __asm__(".text\n.arm\n.global " #name "\n" #name ":\n  push {r4-r8, lr}\n  ldr r5, =micro_scratch+128\n  mov r1, #1\n  mov r2, #0\n" endian_on "1:\n" \
            "  b " #name "_c1\n" #name "_r1:\n  b " #name "_c2\n" #name "_r2:\n  b " #name "_c3\n" #name "_r3:\n  b " #name "_c4\n" #name "_r4:\n" \
            "  b " #name "_c5\n" #name "_r5:\n  b " #name "_c6\n" #name "_r6:\n  b " #name "_c7\n" #name "_r7:\n  b " #name "_c8\n" #name "_r8:\n" \
            "  subs r0, r0, #1\n  bne 1b\n" endian_off "  pop {r4-r8, pc}\n  .ltorg\n" \
            #name "_c1: add r2, r2, #1\n b " #name "_r1\n" #name "_c2: add r2, r2, #1\n b " #name "_r2\n" #name "_c3: add r2, r2, #1\n b " #name "_r3\n" \
            #name "_c4: add r2, r2, #1\n b " #name "_r4\n" #name "_c5: add r2, r2, #1\n b " #name "_r5\n" #name "_c6: add r2, r2, #1\n b " #name "_r6\n" \
            #name "_c7: add r2, r2, #1\n b " #name "_r7\n" #name "_c8: add r2, r2, #1\n b " #name "_r8\n")
BOUT(m_bout_be, BE, LE);
// a body of 4 instructions inline (the 4-word slot), for comparison with b-out of the same four
LOOP(m_inl4_be, BE, LE, "  add r2, r2, #1\n  add r2, r2, #1\n  add r2, r2, #1\n  add r2, r2, #1\n");
extern uint32_t m_b_next(uint32_t), m_bout_be(uint32_t), m_inl4_be(uint32_t);

// ---- v5: X in the GE flags ----
LOOP(m_ge_set_be, BE, LE, "  sbc r3, r3, r3\n  uadd8 r3, r3, r3\n");                    // producer: GE = 1111 iff C was clear
LOOP(m_ge_get_be, BE, LE, "  sel r3, r1, r2\n  adds r3, r3, r3\n");                      // consumer: GE -> a register -> C
LOOP(m_ge_both_be, BE, LE, "  sbc r3, r3, r3\n  uadd8 r3, r3, r3\n  sel r3, r1, r2\n  adds r3, r3, r3\n");
LOOP4(m_x_mem_be, BE, LE, "  movcs r0, #0x10\n  movcc r0, #0\n  strb r0, [r5, #-0x43]\n");              // today's producer
LOOP4(m_x_mem2_be, BE, LE, "  movcs r0, #0x10\n  movcc r0, #0\n  strb r0, [r5, #-0x43]\n  ldrb r0, [r5, #-0x43]\n  cmp r0, #0x10\n"); // producer then consumer
LOOP(m_flagsonly_be, BE, LE, "  adds r3, r3, r3\n  mvns r3, r3\n");                      // an ordinary flag-setting pair, for the floor
extern uint32_t m_ge_set_be(uint32_t), m_ge_get_be(uint32_t), m_ge_both_be(uint32_t), m_x_mem_be(uint32_t), m_x_mem2_be(uint32_t), m_flagsonly_be(uint32_t);

// ---- v6: SUBQ.L #1,D5 with the X save placed in different ways (memory-resident D5: the chain through memory is the loop's) ----
#define D5 "[r5, #-0x2c]"
#define LD "  ldr r0, " D5 "\n"
#define RS "  rsbs r0, r0, #0\n"
#define MV "  mvns r0, r0\n"
#define ST "  str r0, " D5 "\n"
#define SB "  sbc r3, r3, r3\n"
#define UA "  uadd8 r3, r3, r3\n"
LOOP4(m_sq_noX,  BE, LE, LD RS MV ST);                                   // the ceiling: no X save
LOOP4(m_sq_mem,  BE, LE, LD RS MV ST "  movcs r3, #0x10\n  movcc r3, #0\n  strb r3, [r5, #-0x43]\n");   // today
LOOP4(m_sq_ge0,  BE, LE, LD RS MV ST SB UA);                             // GE, after the store
LOOP4(m_sq_ge1,  BE, LE, LD RS SB MV UA ST);                             // sbc beside mvns, uadd8 beside str
LOOP4(m_sq_ge2,  BE, LE, LD RS MV SB ST UA);                             // str between the pair
LOOP4(m_sq_ge3,  BE, LE, LD RS SB UA MV ST);                             // pair early
extern uint32_t m_sq_noX(uint32_t), m_sq_mem(uint32_t), m_sq_ge0(uint32_t), m_sq_ge1(uint32_t), m_sq_ge2(uint32_t), m_sq_ge3(uint32_t);

extern uint32_t m_nop_le(uint32_t), m_nop_be(uint32_t), m_mrsmsr_le(uint32_t), m_mrsmsr_be(uint32_t), m_mrs_le(uint32_t), m_msr_le(uint32_t),
    m_blbx_le(uint32_t), m_blbx_be(uint32_t), m_setend(uint32_t), m_subq_le(uint32_t), m_subq_be(uint32_t), m_subqcall_be(uint32_t),
    m_ldrstr_be(uint32_t), m_flags_be(uint32_t), m_strb_be(uint32_t);

#define N 20000
static void run(const char *name, uint32_t (*f)(uint32_t))
{
    f(1000);                                            // warm
    uint32_t t0 = bbb_pmu_cycles(); f(N); uint32_t t1 = bbb_pmu_cycles();
    uint32_t c100 = (uint32_t)(((uint64_t)(t1 - t0) * 100) / ((uint64_t)N * 8));
    printf("[MICRO] %-14s %3u.%02u cycles per body\n", name, c100 / 100, c100 % 100);
}

// ---- v4: branch-target-buffer reach: K call sites at a fixed stride (4, 8, 16, 32 bytes), each `bl` to a `bx lr` ----
__asm__(".text\n.arm\n.balign 64\n.global m_s4_k128\nm_s4_k128:\n  push {r4-r8, lr}\n  setend be\n1:\n  .rept 128\n  bl m_s4_k128_f\n  .endr\n  subs r0, r0, #1\n  bne 1b\n  setend le\n  pop {r4-r8, pc}\n  .ltorg\nm_s4_k128_f:\n  bx lr\n");
__asm__(".text\n.arm\n.balign 64\n.global m_s4_k256\nm_s4_k256:\n  push {r4-r8, lr}\n  setend be\n1:\n  .rept 256\n  bl m_s4_k256_f\n  .endr\n  subs r0, r0, #1\n  bne 1b\n  setend le\n  pop {r4-r8, pc}\n  .ltorg\nm_s4_k256_f:\n  bx lr\n");
__asm__(".text\n.arm\n.balign 64\n.global m_s4_k512\nm_s4_k512:\n  push {r4-r8, lr}\n  setend be\n1:\n  .rept 512\n  bl m_s4_k512_f\n  .endr\n  subs r0, r0, #1\n  bne 1b\n  setend le\n  pop {r4-r8, pc}\n  .ltorg\nm_s4_k512_f:\n  bx lr\n");
__asm__(".text\n.arm\n.balign 64\n.global m_s8_k128\nm_s8_k128:\n  push {r4-r8, lr}\n  setend be\n1:\n  .rept 128\n  bl m_s8_k128_f\nnop\n  .endr\n  subs r0, r0, #1\n  bne 1b\n  setend le\n  pop {r4-r8, pc}\n  .ltorg\nm_s8_k128_f:\n  bx lr\n");
__asm__(".text\n.arm\n.balign 64\n.global m_s8_k256\nm_s8_k256:\n  push {r4-r8, lr}\n  setend be\n1:\n  .rept 256\n  bl m_s8_k256_f\nnop\n  .endr\n  subs r0, r0, #1\n  bne 1b\n  setend le\n  pop {r4-r8, pc}\n  .ltorg\nm_s8_k256_f:\n  bx lr\n");
__asm__(".text\n.arm\n.balign 64\n.global m_s8_k512\nm_s8_k512:\n  push {r4-r8, lr}\n  setend be\n1:\n  .rept 512\n  bl m_s8_k512_f\nnop\n  .endr\n  subs r0, r0, #1\n  bne 1b\n  setend le\n  pop {r4-r8, pc}\n  .ltorg\nm_s8_k512_f:\n  bx lr\n");
__asm__(".text\n.arm\n.balign 64\n.global m_s16_k128\nm_s16_k128:\n  push {r4-r8, lr}\n  setend be\n1:\n  .rept 128\n  bl m_s16_k128_f\nnop\nnop\nnop\n  .endr\n  subs r0, r0, #1\n  bne 1b\n  setend le\n  pop {r4-r8, pc}\n  .ltorg\nm_s16_k128_f:\n  bx lr\n");
__asm__(".text\n.arm\n.balign 64\n.global m_s16_k256\nm_s16_k256:\n  push {r4-r8, lr}\n  setend be\n1:\n  .rept 256\n  bl m_s16_k256_f\nnop\nnop\nnop\n  .endr\n  subs r0, r0, #1\n  bne 1b\n  setend le\n  pop {r4-r8, pc}\n  .ltorg\nm_s16_k256_f:\n  bx lr\n");
__asm__(".text\n.arm\n.balign 64\n.global m_s16_k512\nm_s16_k512:\n  push {r4-r8, lr}\n  setend be\n1:\n  .rept 512\n  bl m_s16_k512_f\nnop\nnop\nnop\n  .endr\n  subs r0, r0, #1\n  bne 1b\n  setend le\n  pop {r4-r8, pc}\n  .ltorg\nm_s16_k512_f:\n  bx lr\n");
__asm__(".text\n.arm\n.balign 64\n.global m_s32_k128\nm_s32_k128:\n  push {r4-r8, lr}\n  setend be\n1:\n  .rept 128\n  bl m_s32_k128_f\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n  .endr\n  subs r0, r0, #1\n  bne 1b\n  setend le\n  pop {r4-r8, pc}\n  .ltorg\nm_s32_k128_f:\n  bx lr\n");
__asm__(".text\n.arm\n.balign 64\n.global m_s32_k256\nm_s32_k256:\n  push {r4-r8, lr}\n  setend be\n1:\n  .rept 256\n  bl m_s32_k256_f\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n  .endr\n  subs r0, r0, #1\n  bne 1b\n  setend le\n  pop {r4-r8, pc}\n  .ltorg\nm_s32_k256_f:\n  bx lr\n");
__asm__(".text\n.arm\n.balign 64\n.global m_s32_k512\nm_s32_k512:\n  push {r4-r8, lr}\n  setend be\n1:\n  .rept 512\n  bl m_s32_k512_f\nnop\nnop\nnop\nnop\nnop\nnop\nnop\n  .endr\n  subs r0, r0, #1\n  bne 1b\n  setend le\n  pop {r4-r8, pc}\n  .ltorg\nm_s32_k512_f:\n  bx lr\n");
extern uint32_t m_s4_k128(uint32_t), m_s4_k256(uint32_t), m_s4_k512(uint32_t), m_s8_k128(uint32_t), m_s8_k256(uint32_t), m_s8_k512(uint32_t), m_s16_k128(uint32_t), m_s16_k256(uint32_t), m_s16_k512(uint32_t), m_s32_k128(uint32_t), m_s32_k256(uint32_t), m_s32_k512(uint32_t);
static void run_stride(void)
{
    static const struct { const char *n; uint32_t (*f)(uint32_t); int stride, K; } t[] = {
        { "m_s4_k128", m_s4_k128, 4, 128 },
        { "m_s4_k256", m_s4_k256, 4, 256 },
        { "m_s4_k512", m_s4_k512, 4, 512 },
        { "m_s8_k128", m_s8_k128, 8, 128 },
        { "m_s8_k256", m_s8_k256, 8, 256 },
        { "m_s8_k512", m_s8_k512, 8, 512 },
        { "m_s16_k128", m_s16_k128, 16, 128 },
        { "m_s16_k256", m_s16_k256, 16, 256 },
        { "m_s16_k512", m_s16_k512, 16, 512 },
        { "m_s32_k128", m_s32_k128, 32, 128 },
        { "m_s32_k256", m_s32_k256, 32, 256 },
        { "m_s32_k512", m_s32_k512, 32, 512 },
    };
    for (unsigned i = 0; i < sizeof(t) / sizeof(t[0]); i++) {
        t[i].f(20);
        uint32_t m0 = bbb_pmu_event(1), c0 = bbb_pmu_cycles(); t[i].f(200); uint32_t c1 = bbb_pmu_cycles(), m1 = bbb_pmu_event(1);
        uint32_t calls = 200u * t[i].K, c100 = (uint32_t)(((uint64_t)(c1 - c0) * 100) / calls), m100 = (uint32_t)(((uint64_t)(m1 - m0) * 1000) / calls);
        printf("[STRIDE] %2d bytes, %3d call sites: %u.%02u cycles per call, %u.%03u mispredicts per call\n", t[i].stride, t[i].K, c100 / 100, c100 % 100, m100 / 1000, m100 % 1000);
    }
}

int main(void)
{
    printf("=== Cortex-A8 microbenchmarks (cycles per body, 8 per loop iteration; the loop's own subs/bne adds ~1/8) ===\n");
#define R(f) run(#f, f)
    R(m_sq_noX); R(m_sq_mem); R(m_sq_ge0); R(m_sq_ge1); R(m_sq_ge2); R(m_sq_ge3); R(m_ge_both_be); R(m_x_mem_be); R(m_x_mem2_be);
    printf("PASS\n");
    return 0;
}
