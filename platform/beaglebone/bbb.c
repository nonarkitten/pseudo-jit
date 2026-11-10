/// @file bbb.c
/// @brief BeagleBone platform: page tables and caches, the RAM log console (and UART0 if it is listening), the watchdog, the cycle
///        counter / performance monitor, and the clean return to U-Boot. Runs little-endian, before and after guest code only.
#include <stdint.h>
#include "bbb.h"
#include "cp15.h"
#include "pjit_libc.h"

// ---- registers (AM335x TRM) -----------------------------------------------
#define REG32(a)        (*(volatile uint32_t *)(a))
#define UART0_BASE      0x44E09000u
#define UART0_THR       REG32(UART0_BASE + 0x00)
#define UART0_SSR       REG32(UART0_BASE + 0x44)        // bit 0: TX FIFO full
#define WDT1_BASE       0x44E35000u
#define WDT1_WCLR       REG32(WDT1_BASE + 0x24)
#define WDT1_WCRR       REG32(WDT1_BASE + 0x28)
#define WDT1_WLDR       REG32(WDT1_BASE + 0x2C)
#define WDT1_WTGR       REG32(WDT1_BASE + 0x30)
#define WDT1_WWPS       REG32(WDT1_BASE + 0x34)         // posted-write pending: bit 0 WCLR, 1 WCRR, 2 WLDR, 3 WTGR, 4 WSPR
#define WDT1_WSPR       REG32(WDT1_BASE + 0x48)
#define CM_WKUP_WDT1_CLKCTRL  REG32(0x44E004D4u)        // MODULEMODE 2 = enabled
#define PRM_RSTCTRL     REG32(0x44E00F00u)              // bit 0 warm, bit 1 cold global reset
#define CM_PER_GPIO1_CLKCTRL REG32(0x44E000ACu)         // MODULEMODE 2 = enabled
#define CONF_GPMC_A5    REG32(0x44E10854u)              // the four user LEDs: GPMC_A5..A8 = GPIO1_21..24
#define GPIO1_OE        REG32(0x4804C134u)
#define GPIO1_CLEARDATAOUT REG32(0x4804C190u)
#define GPIO1_SETDATAOUT   REG32(0x4804C194u)
#define WDT_HZ          32768u                          // the watchdog runs from the 32 kHz clock, no prescaler

#ifndef BBB_UART
#define BBB_UART 1                                      // also send the console to UART0 (U-Boot left it at 115200 8N1)
#endif

// ---- the RAM log --------------------------------------------------------------------------------------------------------------
static volatile bbb_log_t *const logh = (volatile bbb_log_t *)BBB_LOG_BASE;
static volatile char *const logtext = (volatile char *)(BBB_LOG_BASE + sizeof(bbb_log_t));

void pjit_console_putc(char c)
{
    if (c == '\n') pjit_console_putc('\r');
    uint32_t n = logh->length;
    if (n < BBB_LOG_SIZE - sizeof(bbb_log_t)) { logtext[n] = c; logh->length = n + 1; }
#if BBB_UART
    for (uint32_t spin = 0; (UART0_SSR & 1) && spin < 200000; spin++) { }
    UART0_THR = (uint8_t)c;
#endif
}

// ---- watchdog ------------------------------------------------------------------------------------------------------------------
static void wdt_wait(uint32_t mask) { for (uint32_t spin = 0; (WDT1_WWPS & mask) && spin < 1000000; spin++) { } }

void bbb_wdt_stop(void)
{
    WDT1_WSPR = 0xAAAA; wdt_wait(1u << 4);
    WDT1_WSPR = 0x5555; wdt_wait(1u << 4);
}

/// The 68K test programs (Dhrystone among them) read DMTIMER1_1MS's TCRR at 0x44E31028 as their clock.
/// U-Boot leaves the module clock off, and a read from an unclocked module is a data abort: enable it, 24 MHz oscillator, free running.
static void bbb_timer1ms_start(void)
{
    REG32(0x44E004C4u) = 2;                             // CM_WKUP_TIMER1_CLKCTRL: module enabled
    REG32(0x44E00528u) = 0;                             // CLKSEL_TIMER1MS_CLK: CLK_M_OSC (24 MHz)
    for (int i = 0; i < 100000 && (REG32(0x44E004C4u) >> 16 & 3); i++) { }   // IDLEST: wait for functional
    REG32(0x44E31024u) = 1;                             // TCLR: start
    uint32_t c0 = bbb_pmu_cycles(), t0 = REG32(0x44E31028u);
    while (bbb_pmu_cycles() - c0 < 100000000u) { }
    printf("[BBB] DMTIMER1_1MS: %u ticks in 100M cycles (CLKSEL %u, TCLR %02X, TPIR %u TNIR %u)\n", REG32(0x44E31028u) - t0, REG32(0x44E00528u),
           REG32(0x44E31024u), REG32(0x44E31048u), REG32(0x44E3104Cu));
}

void bbb_wdt_arm(uint32_t seconds)
{
    CM_WKUP_WDT1_CLKCTRL = 2;                           // module enabled (U-Boot normally has it on already)
    bbb_wdt_stop();                                     // reprogram while it is not counting
    WDT1_WCLR = 0;                                      // no prescaler
    wdt_wait(1u << 0);
    WDT1_WLDR = 0u - seconds * WDT_HZ;                  // counts up to 2^32, then resets
    wdt_wait(1u << 2);
    WDT1_WTGR = WDT1_WTGR + 1;                          // reload
    wdt_wait(1u << 3);
    WDT1_WSPR = 0xBBBB; wdt_wait(1u << 4);
    WDT1_WSPR = 0x4444; wdt_wait(1u << 4);              // start
}


// ---- the user LEDs: a status channel that needs no console -------------------------------------------------------------------
void bbb_led(uint32_t mask)                             // bit n = USR n
{
    static int ready;
    if (!ready) {
        CM_PER_GPIO1_CLKCTRL = 2;
        for (int n = 0; n < 4; n++) (&CONF_GPMC_A5)[n] = 0x07;      // mode 7 (GPIO), output
        GPIO1_OE &= ~(0xFu << 21);
        ready = 1;
    }
    GPIO1_CLEARDATAOUT = 0xFu << 21;
    GPIO1_SETDATAOUT = (mask & 0xFu) << 21;
}

// ---- performance monitor -----------------------------------------------------------------------------------------------------
void bbb_pmu_start(const uint8_t events[4])
{
    uint32_t v;
    __asm__ volatile ("mrc p15, 0, %0, c9, c12, 0" : "=r"(v));
    v |= 1u | 2u | 4u;                                  // E: enable, P: reset the event counters, C: reset the cycle counter
    v &= ~(1u << 3);                                    // D: the cycle counter counts every cycle
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 0" :: "r"(v));
    for (int n = 0; n < 4; n++) {
        __asm__ volatile ("mcr p15, 0, %0, c9, c12, 5" :: "r"(n));              // PMSELR
        __asm__ volatile ("mcr p15, 0, %0, c9, c13, 1" :: "r"((uint32_t)events[n]));   // PMXEVTYPER
    }
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 1" :: "r"(0x8000000Fu));       // PMCNTENSET: cycle counter + four event counters
}

uint32_t bbb_pmu_cycles(void)
{
    uint32_t v; __asm__ volatile ("mrc p15, 0, %0, c9, c13, 0" : "=r"(v)); return v;
}

uint32_t bbb_pmu_event(int n)
{
    uint32_t v;
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 5" :: "r"(n));
    __asm__ volatile ("mrc p15, 0, %0, c9, c13, 2" : "=r"(v));
    return v;
}

// ---- MMU: 1 MB sections, identity-mapped ---------------------------------------------------------------------------------------
// 0x00000000-0x7FFFFFFF  peripherals, OCRAM, ROM: shareable device memory
// 0x80000000-0x9FFFFFFF  DDR: normal, inner/outer write-back write-allocate (the DR cache is filled with code, which the runtime
//                        cleans / invalidates itself); the 1 MB section holding the RAM log is uncached so it survives a hang
// 0xA0000000-0xFFFFFFFF  nothing: faults
static uint32_t pagetable[4096] __attribute__((aligned(16384)));

static void mmu_init(void)
{
    for (uint32_t i = 0; i < 4096; i++) pagetable[i] = 0;
    for (uint32_t i = 0x000; i < 0x800; i++) pagetable[i] = (i << 20) | 0x0C06;     // AP=11, B=1 (device), section
    for (uint32_t i = 0x800; i < 0xA00; i++) pagetable[i] = (i << 20) | 0x1C0E;     // TEX=001 C=1 B=1, AP=11, section
    pagetable[BBB_LOG_BASE >> 20] = (BBB_LOG_BASE & 0xFFF00000u) | 0x0C02;          // strongly ordered: uncached, unbuffered
    CP15DCacheCleanFlush();                                                         // U-Boot's dirty lines out before we change tables
    CP15DCacheDisable();
    CP15MMUDisable();
    CP15ICacheFlush();
    CP15BranchPredictorInvalidate();
    CP15Ttb0Set((uint32_t)pagetable);
    CP15TtbCtlTtb0Config();
    CP15DomainAccessClientSet();
    CP15TlbInvalidate();
    CP15MMUEnable();
    CP15ICacheEnable();
    CP15DCacheEnable();
    CP15BranchPredictionEnable();
}

/// Guest address 0..0xFFFFF is the 1 MB-aligned RAM at `phys` (a 68K image that lives at address 0, and its zero page). The cache is cleaned first so
/// the table walk sees the new entry.
void bbb_map_low(uint32_t phys)
{
    pagetable[0] = (phys & 0xFFF00000u) | 0x1C0E;                                    // normal, write-back, like the DDR sections
    CP15DCacheCleanFlushBuff((uint32_t)&pagetable[0], 4);
    CP15TlbInvalidate();
    CP15BranchPredictorInvalidate();
}

// ---- boot and exit ----------------------------------------------------------------------------------------------------------------
#ifndef BBB_WDT_SECONDS
#define BBB_WDT_SECONDS 120
#endif

void bbb_init(void)
{
    logh->magic = BBB_LOG_MAGIC; logh->length = 0; logh->state = BBB_RUN_STARTED; logh->code = 0; logh->cycles = 0;
    bbb_led(BBB_LED_RUNNING);
    mmu_init();
    {   // U-Boot leaves SCTLR.A (alignment check) on; 68K code does word-aligned longword accesses, which the A8 handles in hardware once A is clear (U is set)
        uint32_t v; __asm__ volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r"(v));
        v &= ~2u; __asm__ volatile ("mcr p15, 0, %0, c1, c0, 0\n isb" : : "r"(v));
    }
    static const uint8_t ev[4] = { PMU_INSTR, PMU_BR_MISPRED, PMU_L1D_MISS, PMU_L1I_MISS };
    bbb_pmu_start(ev);
    bbb_timer1ms_start();
    bbb_wdt_arm(BBB_WDT_SECONDS);
    printf("[BBB] up: SCTLR=%08X ACTLR=%08X watchdog %us\n", ({ uint32_t v; __asm__ volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r"(v)); v; }),
           ({ uint32_t v; __asm__ volatile ("mrc p15, 0, %0, c1, c0, 1" : "=r"(v)); v; }), (unsigned)BBB_WDT_SECONDS);
}

extern void bbb_return(uint32_t code);                  // boot.S

void bbb_exit(uint32_t code)
{
    logh->cycles = bbb_pmu_cycles();
    logh->code = code;
    logh->state = (logh->state == BBB_RUN_FAULT) ? BBB_RUN_FAULT : BBB_RUN_DONE;
    bbb_led(logh->state == BBB_RUN_FAULT ? BBB_LED_FAULT : BBB_LED_DONE);
    printf("[BBB] exit %u\n", (unsigned)code);
    bbb_wdt_stop();                                     // a clean exit must not be reset behind U-Boot's back
    CP15DCacheCleanFlush();                             // everything out to DDR (the log is uncached anyway)
    bbb_return(code);
    for (;;) { }
}

/// regs = r0..r12 as saved by boot.S's Fault_Common (r0 = type, r1 = lr). The faulting instruction is at lr-8 for a data abort.
void bbb_fault_dump(const uint32_t *regs)
{
    for (int i = 2; i < 13; i += 4) {
        for (int k = i; k < i + 4 && k < 13; k++) printf(" r%d=%08X", k, (unsigned)regs[k]);
        printf("\n");
    }
    const uint32_t *pc = (const uint32_t *)(regs[1] - 8);
    for (int k = -6; k <= 2; k++) printf("[FAULT] %08X: %08X%s\n", (unsigned)(pc + k), (unsigned)pc[k], k == 0 ? "  <--" : "");
}

void bbb_fault_exit(void)
{
    logh->state = BBB_RUN_FAULT;
    bbb_exit(1);
}

/// The tests end with qemu_exit(); here it is the clean return to U-Boot.
void qemu_exit(void) { bbb_exit(0); }

/// Default hook for tests that look at D0 after TRAP #15.
__attribute__((weak)) void pjit_test_exit(unsigned d0) { (void)d0; }

/// The interrupt controller is not wired up yet: no 68K interrupts on this platform (pjit_irq_enable stays off).
__attribute__((weak)) unsigned pjit_platform_irq(void) { return 0; }
