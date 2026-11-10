/// @file bbb_window.c
/// @brief What an interrupt window costs on the Cortex-A8, and whether a pending IRQ is really taken inside it. `make bbb-bbb_window`.
///        Part 1: cycles per window (`cpsie i ; cpsid i`, with a nop / isb / dsb+isb between) against a flag-free poll of a memory byte, IRQs masked at the INTC.
///        Part 2: the AM335x INTC's software-interrupt register (ISR_SET0) makes line 0 pending; after a delay the window runs; the handler records where the
///        interrupted code would have resumed. Prints, per variant, how many of the trials were taken and at which offset from the `cpsie`.
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"

#define INTC 0x48200000u
#define R32(a) (*(volatile uint32_t *)(a))

// ---- part 1: cost ----
#define CLOOP(name, body)                                                                                              \
    __asm__(".text\n.arm\n.balign 64\n.global " #name "\n" #name ":\n  push {r4}\n1:\n  .rept 8\n" body "\n  .endr\n"   \
            "  subs r0, r0, #1\n  bne 1b\n2:\n  pop {r4}\n  bx lr\n  .ltorg\n")
CLOOP(w_base,   "  add r2, r2, #1");
CLOOP(w_pair,   "  cpsie i\n  cpsid i");
CLOOP(w_nop,    "  cpsie i\n  nop\n  cpsid i");
CLOOP(w_isb,    "  cpsie i\n  isb\n  cpsid i");
CLOOP(w_dsbisb, "  cpsie i\n  dsb\n  isb\n  cpsid i");
CLOOP(w_poll,   "  ldrb r3, [r1]\n  cmp r3, #0\n  bne 2f");                     // the flag-clobbering poll of a pending byte (a flag-free one costs more)
CLOOP(w_pollx,  "  ldrb r3, [r1]\n  cmp r3, #0\n  bne 2f\n  add r2, r2, #1\n  add r2, r2, #1");   // the poll with two other instructions beside it
CLOOP(w_pcpoll, "  ldrb r3, [r1]\n  add pc, pc, r3\n  nop\n  b 4f\n  b 2f\n4:");           // flag-free poll: r3 = 0 or 4 picks `b continue` or `b slow`
CLOOP(w_pcpoll1, "  ldrb r3, [r1]\n  add pc, pc, r3\n  nop\n  nop\n  nop");                   // the same without the taken `b` (poll inline, r3 = 0: skips the nop)
CLOOP(w_two,    "  add r2, r2, #1\n  add r2, r2, #1");
extern uint32_t w_pcpoll(uint32_t, const uint8_t *), w_pcpoll1(uint32_t, const uint8_t *), w_base(uint32_t, const uint8_t *), w_pair(uint32_t, const uint8_t *), w_nop(uint32_t, const uint8_t *), w_isb(uint32_t, const uint8_t *),
    w_dsbisb(uint32_t, const uint8_t *), w_poll(uint32_t, const uint8_t *), w_pollx(uint32_t, const uint8_t *), w_two(uint32_t, const uint8_t *);
static uint8_t zero_byte[64] __attribute__((aligned(64)));

#define N 20000
static void run(const char *name, uint32_t (*f)(uint32_t, const uint8_t *))
{
    f(1000, zero_byte);
    uint32_t t0 = bbb_pmu_cycles(); f(N, zero_byte); uint32_t t1 = bbb_pmu_cycles();
    uint32_t c100 = (uint32_t)(((uint64_t)(t1 - t0) * 100) / ((uint64_t)N * 8));
    printf("[WIN] %-10s %3u.%02u cycles per body\n", name, c100 / 100, c100 % 100);
}

// ---- part 2: is a pending IRQ taken in the window? ----
volatile uint32_t irq_pc, irq_count;
#define TRIAL(name, win)                                                                                               \
    __asm__(".text\n.arm\n.balign 64\n.global " #name "\n" #name ":\n  push {r4, lr}\n  ldr r4, =0x48200000\n  mov r3, #1\n"   \
            "  str r3, [r4, #0x90]\n  dsb\n  .rept 40\n  nop\n  .endr\n"                                                      \
            ".global " #name "_w\n" #name "_w:\n" win "\n.global " #name "_e\n" #name "_e:\n  nop\n  nop\n  nop\n"            \
            "  str r3, [r4, #0x94]\n  dsb\n  pop {r4, pc}\n  .ltorg\n")
TRIAL(t_pair,   "  cpsie i\n  cpsid i");
TRIAL(t_nop,    "  cpsie i\n  nop\n  cpsid i");
TRIAL(t_isb,    "  cpsie i\n  isb\n  cpsid i");
TRIAL(t_dsbisb, "  cpsie i\n  dsb\n  isb\n  cpsid i");
extern void t_pair(void), t_nop(void), t_isb(void), t_dsbisb(void);
extern char t_pair_w[], t_nop_w[], t_isb_w[], t_dsbisb_w[], t_pair_e[], t_nop_e[], t_isb_e[], t_dsbisb_e[];

// our own vector table: the IRQ slot goes to my_irq, the others to the boot table's slots (their `ldr pc,=x` is position independent)
extern void Entry(void);
__asm__(".text\n.arm\n.balign 32\n.global my_vectors\nmy_vectors:\n  b Entry\n  b Entry+4\n  b Entry+8\n  b Entry+12\n  b Entry+16\n  b Entry+20\n"
        "  b my_irq\n  b Entry+28\n"
        "my_irq:\n  sub lr, lr, #4\n  push {r0, r1}\n  ldr r0, =irq_pc\n  str lr, [r0]\n  ldr r0, =irq_count\n  ldr r1, [r0]\n  add r1, r1, #1\n  str r1, [r0]\n"
        "  ldr r0, =0x48200000\n  mov r1, #1\n  str r1, [r0, #0x94]\n  str r1, [r0, #0x48]\n  dsb\n  pop {r0, r1}\n  movs pc, lr\n  .ltorg\n");
extern char my_vectors[];

static void trials(const char *name, void (*f)(void), char *w, char *e)
{
    unsigned taken = 0, missed = 0, at[16] = {0}, late = 0;
    for (int i = 0; i < 200; i++) {
        irq_pc = 0; uint32_t c = irq_count;
        f();
        if (irq_count == c) { missed++; continue; }
        taken++;
        uint32_t off = irq_pc - (uint32_t)w;
        if (irq_pc >= (uint32_t)e) late++;                                // resumed past the window: the IRQ was taken after cpsid took effect
        else if (off / 4 < 16) at[off / 4]++;
    }
    printf("[WIN] %-10s taken %3u missed %3u | resume at cpsie+4*k:", name, taken, missed);
    for (int k = 0; k < 6; k++) printf(" k%d=%u", k, at[k]);
    printf(" | past the window %u\n", late);
}

int main(void)
{
    printf("=== interrupt window cost and behaviour (Cortex-A8, 1 GHz, IRQs masked at the INTC) ===\n");
    for (int i = 0; i < 4; i++) R32(INTC + 0x8C + 0x20 * i) = 0xFFFFFFFFu;            // MIR_SET: everything masked
    run("base", w_base); run("two adds", w_two); run("cpsie;cpsid", w_pair); run("with nop", w_nop); run("with isb", w_isb); run("dsb;isb", w_dsbisb);
    run("poll", w_poll); run("pc-poll", w_pcpoll); run("pc-poll1", w_pcpoll1); run("poll+2", w_pollx);

    R32(INTC + 0x68) = 0xFF;                                                          // THRESHOLD: off
    R32(INTC + 0x100) = 0;                                                            // ILR0: priority 0, IRQ
    R32(INTC + 0x94) = 1;                                                             // ISR_CLEAR0
    R32(INTC + 0x88) = 1;                                                             // MIR_CLEAR0: unmask line 0
    uint32_t vbar_old;
    __asm__ volatile("mrc p15, 0, %0, c12, c0, 0" : "=r"(vbar_old));
    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0\n isb" :: "r"(my_vectors));
    trials("cpsie;cpsid", t_pair, t_pair_w, t_pair_e);
    trials("with nop", t_nop, t_nop_w, t_nop_e);
    trials("with isb", t_isb, t_isb_w, t_isb_e);
    trials("dsb;isb", t_dsbisb, t_dsbisb_w, t_dsbisb_e);
    R32(INTC + 0x8C) = 1;                                                             // line 0 masked again
    __asm__ volatile("mcr p15, 0, %0, c12, c0, 0\n isb" :: "r"(vbar_old));
    printf("PASS\n");
    return 0;
}
