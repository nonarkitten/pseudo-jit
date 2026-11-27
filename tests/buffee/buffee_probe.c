/// @file buffee_probe.c
/// @brief The CPU probes Amiga programs make (SysInfo, MMULib, SetPatch), on the board: user-mode code executes an instruction a 68000 does not have, a handler steps
///        over it (the stacked PC plus its length) and returns with RTE. A two-word line F (a 68030 PMOVE) caught directly and through exec's way (the vector pushes the
///        trap number, a long, and jumps to the task's trap code, which pops it), privilege violations (ORI #,SR, MOVE #,SR), an illegal MOVEC, a one-word line F; and a
///        line F whose vector is odd, which a 68000 turns into an address error at the handler's address; and the double fault, an address error whose own vector is
///        odd, where pjit_run() returns PJIT_VEC_HALT. Every case runs with the vector table, the supervisor and user
///        stacks and the code each in chip RAM (the Amiga's, through the bus; on a board without the byte swapper every access emulated) or in the board's DRAM, the
///        stacks at 4n and 4n+2, and the code interpreted and translated; each probe 300 times in a loop, then 100,000 times with a CIA-A timer interrupt at ~7 kHz
///        (level 2) landing in the probes and their handlers; then all of it again with the loop and the handlers writing into the code's page (protection faults), and
///        with the loop writing inside the probe's own line (the translation thrown away every iteration). Like tests/qemu/smoke_probe.c, where QEMU cannot run the
///        unaligned frames.
///        make bundle-probe
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "buffee.h"
#include "mem_be.h"

extern void emit_opcode_table(void);
#define DRAM_CODE   0x08001000u
#define DRAM_VEC    0x08010000u
#define DRAM_STACK  0x08100000u
#define CHIP        0x00070000u                  /* chip RAM: code at +0, vectors at +0x4000, stacks below +0x6000 / +0x8000 */
#define ITER_IRQ    100000                       /* with the CIA interrupt: long enough for ~1000 interrupts inside probes and handlers */
#define IRQ_COUNT   0x08020000u                  /* the interrupt handler's count (a big-endian long) */
static uint32_t ITER = 300;
static int irq_on, smc_mode;                     /* smc_mode 1: the loop and the handlers write into the code's page (not its lines); 2: the loop writes inside the probe's own line */
#define POISON      0xDEADBEEFu

enum { LINE_F_V = 11, PRIV_V = 8, ILL_V = 4, ADDR_V = 3 };

static uint32_t code, hp, np, base_pc, probe_pc, vbr;
static void w(uint16_t v)    { write_BE_word(code + np * 2, v); np++; }
static void wl(uint32_t v)   { w(v >> 16); w(v & 0xFFFF); }
static uint32_t here(void)   { return code + np * 2; }
static void vec(int v, uint32_t a) { write_BE_long(vbr + 4 * v, a); }

/* The handlers go at code + 0x800 */
static uint32_t h_skip(int len)
{
    uint32_t save = np, h; np = hp; h = here();
    if (smc_mode) { w(0x52B9); wl(code + 0x304); }   /* ADDQ.L #1,code+$304: a write into the code's page */
    w(0x242F); w(0x0002);                        /* MOVE.L 2(A7),D2 */
    w(0x5280);                                   /* ADDQ.L #1,D0 */
    w(0x50AF | ((len & 7) << 9)); w(0x0002);     /* ADDQ.L #len,2(A7) */
    w(0x4E73);                                   /* RTE */
    hp = np; np = save; return h;
}
static uint32_t h_dispatch(int num, int len)
{
    uint32_t save = np, codeaddr, entry; np = hp; codeaddr = here();
    if (smc_mode) { w(0x52B9); wl(code + 0x304); }
    w(0x0C9F); wl((uint32_t)num);                /* CMPI.L #num,(A7)+ */
    w(0x6702); w(0x7EFF);                        /* BEQ.S +2 ; MOVEQ #-1,D7 */
    w(0x242F); w(0x0002);                        /* MOVE.L 2(A7),D2 */
    w(0x5280);                                   /* ADDQ.L #1,D0 */
    w(0x50AF | ((len & 7) << 9)); w(0x0002);     /* ADDQ.L #len,2(A7) */
    w(0x4E73);
    entry = here();
    w(0x2F3C); wl((uint32_t)num);                /* MOVE.L #num,-(A7) */
    w(0x4EF9); wl(codeaddr);                     /* JMP code */
    hp = np; np = save; return entry;
}
/* level 2 (CIA-A timer A): acknowledge the CIA and Paula, count, return */
static uint32_t h_irq(void)
{
    uint32_t save = np, h; np = hp; h = here();
    w(0x4A39); wl(0xBFED01);                     /* TST.B $BFED01 (the ICR: reading acknowledges) */
    w(0x33FC); w(0x0008); wl(0xDFF09C);          /* MOVE.W #$0008,INTREQ */
    w(0x52B9); wl(IRQ_COUNT);                    /* ADDQ.L #1,count */
    w(0x4E73);
    hp = np; np = save; return h;
}
static void irq_start(void)
{
    write_BE_long(IRQ_COUNT, 0);
    *(volatile uint16_t *)0xDFF09A = __builtin_bswap16(0x7FFF);
    *(volatile uint16_t *)0xDFF09C = __builtin_bswap16(0x7FFF);
    *(volatile uint8_t *)0xBFED01 = 0x7F;        /* CIA-A: no interrupts, then timer A, 100 E clocks (~140 us), continuous */
    (void)*(volatile uint8_t *)0xBFED01;
    *(volatile uint8_t *)0xBFE401 = 100; *(volatile uint8_t *)0xBFE501 = 0;
    *(volatile uint8_t *)0xBFEE01 = 0x11;
    *(volatile uint8_t *)0xBFED01 = 0x81;
    *(volatile uint16_t *)0xDFF09A = __builtin_bswap16(0xC008);   /* INTENA: master + PORTS */
}
static void irq_stop(void)
{
    *(volatile uint16_t *)0xDFF09A = __builtin_bswap16(0x7FFF);
    *(volatile uint8_t *)0xBFED01 = 0x7F; *(volatile uint8_t *)0xBFEE01 = 0x00; (void)*(volatile uint8_t *)0xBFED01;
    *(volatile uint16_t *)0xDFF09C = __builtin_bswap16(0x7FFF);
}

static uint32_t h_catch(void)
{
    uint32_t save = np, h; np = hp; h = here();
    w(0x7EEE); w(0x4E4F);                        /* MOVEQ #$EE,D7 ; TRAP #15 */
    hp = np; np = save; return h;
}

static void p_pmove(void)   { w(0xF017); w(0x4E00); }
static void p_ori_sr(void)  { w(0x007C); w(0x0000); }
static void p_move_sr(void) { w(0x46FC); w(0x0000); }
static void p_movec(void)   { w(0x4E7B); w(0x0801); }
static void p_linef1(void)  { w(0xF000); w(0x4E71); }

/* the loop around a probe (tests/qemu/smoke_probe.c) */
static void build_loop(void (*probe)(void))
{
    np = 0;
    w(0x7000); w(0x7A00); w(0x7E00);             /* MOVEQ #0,D0 / D5 / D7 */
    w(0x2C3C); wl(ITER - 1);                     /* MOVE.L #ITER-1,D6 */
    uint32_t top = here();
    w(0x76FF);                                   /* MOVEQ #-1,D3 */
    if (smc_mode == 1) { w(0x23C6); wl(code + 0x300); }                       /* MOVE.L D6,code+$300 */
    if (smc_mode == 2) { uint32_t data = here() + 2; w(0x6004); wl(0); w(0x23C6); wl(data); }   /* BRA.S over a long ; MOVE.L D6,that long: inside the probe's line */
    w(0x598F);                                   /* SUBQ.L #4,A7 */
    probe_pc = here();
    probe();
    w(0x4203); w(0x588F); w(0x8A03);             /* CLR.B D3 ; ADDQ.L #4,A7 ; OR.B D3,D5 */
    w(0x5386); w(0x6A00); w((uint16_t)(top - here()));   /* SUBQ.L #1,D6 ; BPL.W top */
    w(0x4E4F);                                   /* TRAP #15 */
}

static int pass, fail;

static uint32_t start(uint32_t sr, uint32_t usp, uint32_t ssp)
{
    for (int r = 0; r < 8; r++) { pjit_set_dn(r, POISON); pjit_set_an(r, 0); }
    cpu->ssp = __builtin_bswap32(ssp);
    cpu->usp = __builtin_bswap32(usp);
    pjit_set_an(7, (sr & 0x2000) ? ssp : usp);
    pjit_set_sr(sr);
    cpu->irq_level = 0; cpu->irq_flag = 0;
    cpu->vbr = vbr;
    pjit_cache_flush();
    if (irq_on) irq_start();
    uint32_t r = pjit_run((uint16_t *)code);
    if (irq_on) irq_stop();
    return r;
}

static void one(const char *what, const char *where, int vector, uint32_t (*mk)(void), void (*probe)(void), uint32_t usp, uint32_t ssp)
{
    hp = 0x400;
    uint32_t c = h_catch();
    for (int v = 0; v < 64; v++) vec(v, c);
    vec(26, h_irq());
    vec(vector, mk());
    build_loop(probe);
    uint32_t r = start(0x0000, usp, ssp);
    uint32_t d0 = pjit_get_dn(0), d2 = pjit_get_dn(2), d5 = pjit_get_dn(5), d7 = pjit_get_dn(7), a7 = pjit_get_an(7), sr = pjit_get_sr();
    int ok = r == 0x2F && d0 == ITER && d2 == probe_pc && d5 == 0 && d7 == 0 && a7 == usp && !(sr & 0x2000);
    if (ok) pass++; else fail++;
    if (irq_on) printf("  %-36s %-40s %u interrupts\n", what, where, (unsigned)read_BE_long(IRQ_COUNT));
    if (!ok) printf("[FAIL] %-32s %s: exit %X D0=%u D2=%08X (want %08X) D5=%X D7=%X A7=%08X (want %08X) SR=%04X\n", what, where, (unsigned)r, (unsigned)d0, (unsigned)d2,
                    (unsigned)probe_pc, (unsigned)d5, (unsigned)d7, (unsigned)a7, (unsigned)usp, (unsigned)sr);
}

static int s_len;
static uint32_t mk_skip(void) { return h_skip(s_len); }
static uint32_t mk_disp(void) { return h_dispatch(11, 4); }

/* an odd line-F vector: the address error's frame (function code, access address, instruction, SR, PC) must name the handler's odd address */
static void odd_vector(const char *where, uint32_t ssp)
{
    hp = 0x400;
    uint32_t c = h_catch();
    for (int v = 0; v < 64; v++) vec(v, c);
    uint32_t save = np; np = hp;
    uint32_t odd = here() + 1; w(0x4E71); w(0x4E71);
    uint32_t aerr = here();
    w(0x242F); w(0x0002);                        /* MOVE.L 2(A7),D2: the access address */
    w(0x7033);                                   /* MOVEQ #$33,D0 */
    w(0x4E4F);
    hp = np; np = save;
    vec(LINE_F_V, odd); vec(ADDR_V, aerr);
    np = 0; w(0x7000); w(0x7E00); w(0xF000); w(0x4E4F);
    uint32_t r = start(0x2000, ssp, ssp);
    uint32_t d0 = pjit_get_dn(0), d2 = pjit_get_dn(2), d7 = pjit_get_dn(7);
    int ok = r == 0x2F && d0 == 0x33 && d2 == odd && d7 == 0;
    if (ok) pass++; else fail++;
    if (!ok) printf("[FAIL] %-32s %s: exit %X D0=%X D2=%08X (want %08X) D7=%X\n", "odd line-F vector", where, (unsigned)r, (unsigned)d0, (unsigned)d2, (unsigned)odd, (unsigned)d7);
}

/* a double fault: an address error (a JMP to an odd address, an odd word read) whose own vector is odd: pjit_run() returns PJIT_VEC_HALT with the handler's address
   in cpu->r0 and the vector in cpu->r1, nothing pushed */
static void double_fault(const char *where, uint32_t ssp)
{
    static const char *const nm[2] = { "double fault: JMP odd", "double fault: odd read" };
    for (int k = 0; k < 2; k++) {
        hp = 0x400;
        uint32_t c = h_catch();
        for (int v = 0; v < 64; v++) vec(v, c);
        uint32_t odd = code + 0x900 + 1, target = code + 0x980 + 1;
        vec(ADDR_V, odd);
        np = 0;
        if (k == 0) { w(0x4EF9); wl(target); }
        else { w(0x207C); wl(target); w(0x3010); w(0x4E4F); }      /* MOVEA.L #odd,A0 ; MOVE.W (A0),D0 ; TRAP #15 */
        cpu->r0 = cpu->r1 = 0;
        uint32_t r = start(0x2700, ssp, ssp);
        int ok = r == PJIT_VEC_HALT && cpu->r0 == odd && cpu->r1 == ADDR_V && pjit_get_an(7) == ssp;
        if (ok) pass++; else fail++;
        if (!ok) printf("[FAIL] %-32s %s: pjit_run %X (want %X) r0 %08X (want %08X) r1 %u A7 %08X\n", nm[k], where, (unsigned)r, PJIT_VEC_HALT, (unsigned)cpu->r0, (unsigned)odd,
                        (unsigned)cpu->r1, (unsigned)pjit_get_an(7));
    }
}

int main(void)
{
    printf("=== CPU probes from user mode (line F, privilege, illegal) ===\n");
    pjit_cache_init(BUFFEE_CACHE_TOP);
    cpu->trap_exit_mask = 0x8000;
    emit_opcode_table();
    pjit_irq_enable(0);
    *(volatile uint8_t *)0xBFE201 |= 3; *(volatile uint8_t *)0xBFE001 &= 0xFC;          /* ROM overlay off: low memory is chip RAM */

    static const struct { const char *name; uint32_t code, vbr, usp, ssp; } place[] = {
        { "all in DRAM",                              DRAM_CODE, DRAM_VEC,      DRAM_STACK,         DRAM_STACK + 0x10000 },
        { "vectors in chip",                          DRAM_CODE, CHIP + 0x4000, DRAM_STACK,         DRAM_STACK + 0x10000 },
        { "vectors, stacks in chip (4n)",             DRAM_CODE, CHIP + 0x4000, CHIP + 0x6000,      CHIP + 0x8000 },
        { "vectors, stacks in chip (4n+2)",           DRAM_CODE, CHIP + 0x4000, CHIP + 0x6002,      CHIP + 0x8002 },
        { "stacks in chip (4n+2), vectors in DRAM",   DRAM_CODE, DRAM_VEC,      CHIP + 0x6002,      CHIP + 0x8002 },
        { "code, vectors, stacks in chip",            CHIP,      CHIP + 0x4000, CHIP + 0x6002,      CHIP + 0x8000 },
    };
    static const struct { const char *name; int vector, len, disp; void (*probe)(void); } probes[] = {
        { "line F, PMOVE, +4",                   LINE_F_V, 4, 0, p_pmove },
        { "line F through a trap-code dispatch", LINE_F_V, 4, 1, p_pmove },
        { "privilege, ORI #,SR",                 PRIV_V,   4, 0, p_ori_sr },
        { "privilege, MOVE #,SR",                PRIV_V,   4, 0, p_move_sr },
        { "illegal, MOVEC",                      ILL_V,    4, 0, p_movec },
        { "line F, one word, +2",                LINE_F_V, 2, 0, p_linef1 },
    };
    for (int t = 0; t < 12; t++) {
        cpu->dr_threshold = (t & 1) ? 0x1000u : 0xFFFFFFFFu;  /* interpreted, then translated (code in chip RAM stays interpreted); then both with the CIA interrupt; */
        irq_on = (t >> 1) & 1; smc_mode = t >> 2;              /* then both again with writes into the code (smc_mode 1, 2) */
        ITER = irq_on ? (smc_mode ? 3000 : ITER_IRQ) : 300;
        pjit_irq_enable(irq_on);
        for (unsigned p = 0; p < sizeof place / sizeof place[0]; p++) {
            code = place[p].code; vbr = place[p].vbr;
            int before = fail;
            for (unsigned k = 0; k < sizeof probes / sizeof probes[0]; k++) {
                s_len = probes[k].len;
                one(probes[k].name, place[p].name, probes[k].vector, probes[k].disp ? mk_disp : mk_skip, probes[k].probe, place[p].usp, place[p].ssp);
            }
            if (!irq_on && !smc_mode) { odd_vector(place[p].name, place[p].ssp); double_fault(place[p].name, place[p].ssp); }
            printf("%-12s %-9s %-6s %-40s %s\n", (t & 1) ? "translated" : "interpreted", irq_on ? "+ CIA irq" : "", smc_mode == 1 ? "+ SMC" : smc_mode == 2 ? "+ SMC*" : "",
                   place[p].name, fail == before ? "PASS" : "FAIL");
        }
    }
    printf("%d passed, %d failed\n%s\n", pass, fail, fail ? "FAIL" : "PASS");
    *(volatile uint8_t *)0xBFE001 |= 1;
    return 0;
}
