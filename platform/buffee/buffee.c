/// @file buffee.c
/// @brief Buffee platform (OSD335x on an Amiga): the PJIT payload that the bootloader (bootloader/) unpacks into DRAM and calls. Same API as the BeagleBone's bbb.c so the
///        bbb_* tests run unchanged, but nothing here assumes U-Boot: the bootloader has already brought up the clocks, DDR, UART0, the GPMC, the PRU and the GreenPAK.
///        What this file adds: our own page table (the Amiga window, guest DRAM at 0x08000000, caches on), the console on UART0, the cycle counter, and the Amiga's
///        interrupt lines (IPL2..0, RESET, HALT) behind pjit_platform_irq(). boot.S (platform/beaglebone/boot.S) saves the bootloader's CPU state and hands it back at the end.
#include <stdint.h>
#include "bbb.h"
#include "cp15.h"
#include "pjit_libc.h"
#include "buffee.h"
#include "pjit.h"
#include "mem_be.h"

#define REG32(a)        (*(volatile uint32_t *)(a))
#define UART0_BASE      0x44E09000u
#define UART0_THR       REG32(UART0_BASE + 0x00)
#define UART0_SSR       REG32(UART0_BASE + 0x44)        // bit 0: TX FIFO full

// ---- console: the bootloader left UART0 at 115200 8N1 -----------------------------------------------------------------------------
void pjit_console_putc(char c)
{
    if (c == '\n') pjit_console_putc('\r');
    for (uint32_t spin = 0; (UART0_SSR & 1) && spin < 2000000; spin++) { }
    UART0_THR = (uint8_t)c;
}

void bbb_led(uint32_t mask) { (void)mask; }

// ---- performance monitor -----------------------------------------------------------------------------------------------------
void bbb_pmu_start(const uint8_t events[4])
{
    uint32_t v;
    __asm__ volatile ("mrc p15, 0, %0, c9, c12, 0" : "=r"(v));
    v |= 1u | 2u | 4u;
    v &= ~(1u << 3);
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 0" :: "r"(v));
    for (int n = 0; n < 4; n++) {
        __asm__ volatile ("mcr p15, 0, %0, c9, c12, 5" :: "r"(n));
        __asm__ volatile ("mcr p15, 0, %0, c9, c13, 1" :: "r"((uint32_t)events[n]));
    }
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 1" :: "r"(0x8000000Fu));
}
uint32_t bbb_pmu_cycles(void) { uint32_t v; __asm__ volatile ("mrc p15, 0, %0, c9, c13, 0" : "=r"(v)); return v; }
uint32_t bbb_pmu_event(int n)
{
    uint32_t v;
    __asm__ volatile ("mcr p15, 0, %0, c9, c12, 5" :: "r"(n));
    __asm__ volatile ("mrc p15, 0, %0, c9, c13, 2" : "=r"(v));
    return v;
}

// ---- MMU: 1 MB sections ---------------------------------------------------------------------------------------------------------
// virtual 0x00000000-0x01FFFFFF   the Amiga's 16 MB address space (and its mirror): the GPMC window at 0x02000000, device memory (a halfword-wide bus: 16-bit accesses only)
// virtual 0x02000000-0x02FFFFFF   the GPMC window itself, identity (device); the rest of 0x02000000-0x07FFFFFF is unmapped
// guest RAM regions (EEPROM: up to four, 1 MB aligned, power-of-two sizes; default 128 MB at $08000000 where Kickstart 3.1 looks for fast RAM): sections backed by consecutive megabytes of the
//                                 DDR from physical 0x80000000, cached; code in them is translated (smc_xlat); see smc_regions_init(). Everything not listed here faults.
// virtual 0x10000000-0x7FFFFFFF   only the sections the platform touches, identity (device): boot ROM / OCRAM 0x400-0x403, L4_WKUP 0x44E (clocks, control, UART0, GPIO0, I2C0, timers, WDT),
//                                 L4_PER 0x480-0x482 (SPI, timers, GPIO1-3, INTC), GPMC registers 0x500; everything else is unmapped, so a wild guest access is a bus error
//                                 (buffee_guest_abort), not a write into a peripheral. A new peripheral section goes in the list in mmu_init().
// virtual 0x80000000-0x9FFFFFFF   the DDR itself, identity, cached (PJIT's image, tables and cache live at the top: docs/buffee/BUFFEE_MAP.md)
// virtual 0xA0000000-0xFFFFFFFF   nothing: faults
static uint32_t pagetable[4096] __attribute__((aligned(16384)));
#define SEC_DEVICE   0x0C06u            // AP=11, B=1
#define SEC_DDR      0x1C0Eu            // TEX=001 C=1 B=1 (write-back, write-allocate)

static void smc_regions_init(void);
static void smc_xlat_rom(int on);
static void smc_xlat_low(int on);
static void mmu_init(void)
{
    for (uint32_t i = 0; i < 4096; i++) pagetable[i] = 0;
    for (uint32_t i = 0x000; i < 0x020; i++) pagetable[i] = (((i & 0xF) | 0x020) << 20) | SEC_DEVICE;   // 0x01000000-0x01FFFFFF wraps onto the first 16 MB: a 68000 has 24 address lines
    for (uint32_t i = 0x020; i < 0x030; i++) pagetable[i] = (i << 20) | SEC_DEVICE;
    static const uint16_t periph[] = { 0x400, 0x401, 0x402, 0x403, 0x44E, 0x480, 0x481, 0x482, 0x500 };
    for (unsigned k = 0; k < sizeof periph / sizeof periph[0]; k++) pagetable[periph[k]] = ((uint32_t)periph[k] << 20) | SEC_DEVICE;
    for (uint32_t i = 0x800; i < 0xA00; i++) pagetable[i] = (i << 20) | SEC_DDR;                    // the DDR itself, identity
    smc_regions_init();                                                   // the guest RAM regions (EEPROM via the bootloader), the translate map and the code-tracking tables
    CP15DCacheCleanFlush();
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

/// Guest address 0..0xFFFFF is the 1 MB-aligned RAM at `phys` (the vectors and a low image). The cache is cleaned first so the table walk sees the new entry.
void bbb_map_low(uint32_t phys)
{
    pagetable[0] = pagetable[0x10] = (phys & 0xFFF00000u) | SEC_DDR;
    smc_xlat_low(1);
    CP15DCacheCleanFlushBuff((uint32_t)&pagetable[0], 4); CP15DCacheCleanFlushBuff((uint32_t)&pagetable[0x10], 4);
    CP15TlbInvalidate();
    CP15BranchPredictorInvalidate();
}

/// MapROM: copies the Kickstart ROM (512 KB at $F80000, read through the GPMC window, halfword by halfword: 1.8 MB/s) into the DDR at BUFFEE_MAPROM_PHYS and maps the guest's
/// 1 MB section $F00000-$FFFFFF onto it (the half below the ROM reads 0xFF, as an empty bus mostly does). Guest reads of the ROM then run at DRAM speed and do not occupy the
/// Amiga's bus. Returns the sum of the ROM's 16-bit words as read (a check for the caller). `size` is 0x40000 / 0x80000 (a 256 KB ROM is mirrored in the 512 KB).
/* ---- the resident tag that adds our RAM regions to exec's memory list --------------------------------------------------------------------------
   Kickstart 3.1's coldstart scan (F80E42) walks $F80000-$FFFFFF and $F00000-$F7FFFF for RomTags (checked once with a ROM scanner against the 40.63 image). The lower half of the MapROM
   megabyte is ours, so a RomTag there is found without touching the Kickstart image. Its init (COLDSTART, priority 106: after expansion.library has configured the real boards, before
   'diag init') is 68K code that calls exec AddMemList for each accepted region from the table that follows it. Regions at the A500's slow RAM ($C00000-$DFFFFF) are left to exec's own probe. */
static struct { uint32_t base, size; } buffee_rg[4];
static unsigned buffee_nrg;
static void buffee_install_tag(void)
{
    uint8_t *t = (uint8_t *)BUFFEE_MAPROM_PHYS + 0x100;                          // guest $F00100
    const uint32_t G = 0xF00100u;
    uint8_t *q = t;
#define B(v) (*q++ = (uint8_t)(v))
#define W(v) (B((v) >> 8), B(v))
#define L(v) (W((v) >> 16), W(v))
    static const char nm[] = "buffee.ram", id[] = "buffee.ram 1.0 (Buffee memory regions)";
    uint32_t nm_o = 26, id_o = nm_o + sizeof nm, code_o = (id_o + sizeof id + 1) & ~1u, tab_o = code_o + 44, end_o = tab_o + 12 * 5;
    W(0x4AFC); L(G); L(G + end_o); B(0x01); B(1); B(0); B(106); L(G + nm_o); L(G + id_o); L(G + code_o);
    for (unsigned i = 0; i < sizeof nm; i++) B(nm[i]);
    for (unsigned i = 0; i < sizeof id; i++) B(id[i]);
    while ((unsigned)(q - t) < code_o) B(0);
    W(0x48E7); W(0x2020);                                                          // movem.l d2/a2,-(a7)
    W(0x45FA); W(tab_o - (code_o + 6));                                            // lea tab(pc),a2
    W(0x202A); W(4);                                                               // l: move.l 4(a2),d0   (size; 0 ends)
    W(0x6716);                                                                     //    beq.s done
    W(0x222A); W(8);                                                               //    move.l 8(a2),d1   (attributes)
    W(0x7400);                                                                     //    moveq #0,d2       (priority)
    W(0x2052);                                                                     //    movea.l (a2),a0   (base)
    W(0x43FA); W(nm_o - (code_o + 22 + 2));                                        //    lea name(pc),a1
    W(0x4EAE); W(0xFD96);                                                          //    jsr -618(a6)      AddMemList
    W(0x45EA); W(12);                                                              //    lea 12(a2),a2
    W(0x60E4);                                                                     //    bra.s l
    W(0x4CDF); W(0x0404);                                                          // done: movem.l (a7)+,d2/a2
    W(0x7000); W(0x4E75);                                                          //    moveq #0,d0 ; rts
    for (unsigned i = 0; i < buffee_nrg; i++) {
        uint32_t b = buffee_rg[i].base, sz = buffee_rg[i].size;
        L(b); L(sz); L(0x5);                                                                    // MEMF_PUBLIC | MEMF_FAST (no MEMF_24BITDMA: no DMA master can reach this RAM)
    }
    L(0); L(0); L(0);
#undef L
#undef W
#undef B
    CP15DCacheCleanFlushBuff((uint32_t)t, 0x400);
    printf("[TAG] RomTag at $%06X, %u region(s)\n", (unsigned)G, buffee_nrg);
}

uint32_t buffee_maprom(uint32_t size)
{
    volatile uint16_t *src = (volatile uint16_t *)0xF80000u;
    uint16_t *dst = (uint16_t *)(BUFFEE_MAPROM_PHYS + 0x80000u);
    uint32_t sum = 0;
    // The bootloader may have staged the image already (EEPROM: MapROM from the flash or a copy of the Amiga's ROM; the rest of the megabyte is 0xFF): then only the mapping is left.
    const volatile uint32_t *bi = (const volatile uint32_t *)0x9F5FFF00u;      // bootinfo_t: magic 'BOOT', ..., byte 10 = rom_staged
    int staged = bi[0] == 0x544F4F42u && ((const volatile uint8_t *)bi)[10] == 1;
    if (staged) {
        for (uint32_t i = 0; i < 0x80000u / 2; i++) sum += __builtin_bswap16(dst[i]);
    } else {
        for (uint32_t i = 0; i < 0x80000u / 2; i++) {
            uint16_t v = src[i % (size / 2)];
            dst[i] = v; sum += __builtin_bswap16(v);
        }
        uint32_t *lo = (uint32_t *)BUFFEE_MAPROM_PHYS;
        for (uint32_t i = 0; i < 0x80000u / 4; i++) lo[i] = 0xFFFFFFFFu;
    }
    // $E00000-$EFFFFF (no extension ROM, and Zorro II autoconfig at $E80000): an empty bus reads 0xFF, which expansion.library takes for "no board". The Amiga's own bus floats
    // there on this accelerator (the previous cycle's data comes back: 0x001F, which looked like a board), so the section is a page of 0xFF in the DRAM.
    buffee_install_tag();
#ifdef FFPAGE
    uint32_t *ff = (uint32_t *)(BUFFEE_MAPROM_PHYS - 0x100000u);
    for (uint32_t i = 0; i < 0x100000u / 4; i++) ff[i] = 0xFFFFFFFFu;
    CP15DCacheCleanFlushBuff(BUFFEE_MAPROM_PHYS - 0x100000u, 0x100000u);
    pagetable[0x00E] = pagetable[0x01E] = ((BUFFEE_MAPROM_PHYS - 0x100000u) & 0xFFF00000u) | SEC_DDR;
#else
    pagetable[0x00E] = pagetable[0x01E] = (0x02Eu << 20) | SEC_DEVICE;                     // the real bus: a Zorro II board in the machine configures itself here ($E80000); with none the pulled-up bus reads high nibbles of F, as the page of 0xFF did
#endif
    CP15DCacheCleanFlushBuff((uint32_t)&pagetable[0x00E], 4); CP15DCacheCleanFlushBuff((uint32_t)&pagetable[0x01E], 4);
    CP15DCacheCleanFlushBuff(BUFFEE_MAPROM_PHYS, 0x100000u);
    pagetable[0x00F] = pagetable[0x01F] = (BUFFEE_MAPROM_PHYS & 0xFFF00000u) | SEC_DDR;
    smc_xlat_rom(1);                                                            // the ROM is translated code (not tracked for writes: it is read-only to the guest)
    CP15DCacheCleanFlushBuff((uint32_t)&pagetable[0x00F], 4); CP15DCacheCleanFlushBuff((uint32_t)&pagetable[0x01F], 4);
    CP15TlbInvalidate();
    CP15BranchPredictorInvalidate();
    return sum;
}

// ---- the Amiga's interrupt lines --------------------------------------------------------------------------------------------------
// The bootloader's InitIRQ() set up the INTC and the GPIO banks (IPL2..0 on GPIO2_6..8, both edges; RESET GPIO3_16 and HALT GPIO0_13, falling edge). Our vector table
// replaces its IRQ vector, so this is the ISR: pjit_irq_entry calls pjit_platform_irq() in IRQ mode and takes the 68K level it returns.
#define INTC            0x48200000u
#define INTC_SIR_IRQ    (INTC + 0x40)
#define INTC_CONTROL    (INTC + 0x48)
#define GPIO0           0x44E07000u
#define GPIO2           0x481AC000u
#define GPIO3           0x481AE000u
#define G_IRQSTATUS0    0x2C
#define G_DATAIN        0x138
#define IPL_MASK        (7u << 6)
#define RESET_BIT       (1u << 16)
#define HALT_BIT        (1u << 13)

uint32_t pjit_flog[1 + 64];                  // runtime.s (PJIT_DLOG): per delivery {sp after the push, new SR, frame words read back}, big-endian; word 0 = byte offset of the next entry
uint32_t pjit_xlog[32], pjit_xlog_pos;                        // runtime.s (PJIT_DLOG): the last 16 exceptions that are not interrupts / Supervisor(): (vector, guest PC after the instruction)
uint32_t pjit_dlog[256], pjit_dlog_pos, pjit_dlog_count;      // runtime.s (PJIT_DLOG): a ring of (vector, guest PC) of the exceptions delivered
volatile uint32_t buffee_reset_events, buffee_halt_events, buffee_ipl_events;
volatile uint32_t buffee_reset_pending;

/// The level on IPL2..0 now (active low, GPIO2_6 = IPL2). The three lines settle within a few ns of each other: read until two reads agree.
unsigned buffee_ipl(void)
{
    uint32_t a, b = REG32(GPIO2 + G_DATAIN);
    do { a = b; b = REG32(GPIO2 + G_DATAIN); } while (((a ^ b) & IPL_MASK) != 0);
    b = ~b;
    return ((b >> 6) & 1) << 2 | ((b >> 7) & 1) << 1 | ((b >> 8) & 1);
}

/// The lines are a level, not an event: whatever woke us, the answer is the level on the pins now (0 when the source was acknowledged). RESET posts the pseudo level 8: the router
/// ends pjit_run() with 0x100 and the C side restarts the machine. HALT is only counted.
uint32_t buffee_last_pc, buffee_last_r4, sample_stop, sample_verbose; volatile uint32_t buffee_sample_stopped;
static volatile uint32_t sample_period;               // cycles between PC samples (0 = off)
static volatile uint32_t sample_n;
uint32_t buffee_irq_pc, buffee_irq_r4;                             // the interrupted host PC (set by the stub in boot.S)

/// The guest PC behind a host PC: inside the slot cache the tag of its line and the slot offset give it back (HOST2GUEST in runtime.s); elsewhere the interpreter's.
static uint32_t guest_pc_of(uint32_t host)
{
    uint32_t base = (uint32_t)cpu_state.cache_data;
    if (host - base < DR_CACHE_BYTES) {
        uint32_t off = host - base;
        return ((uint32_t)cpu_state.tags[off >> DR_HL_SHIFT] << DR_TAG_SHIFT) | (off >> 3);
    }
    return __builtin_bswap32(cpu_state.ipc);
}

/// Starts the PC sampler: the ARM cycle counter's second event counter overflows every `cycles` and raises INTC line 3; the ISR prints where the guest is.
void buffee_sampler_start(uint32_t cycles)
{
    sample_period = cycles;
    REG32(INTC + 0x88) = 1u << 3;                                                   // MIR_CLEAR0: line 3 (the performance monitor)
    __asm__ volatile("mcr p15, 0, %0, c9, c12, 5" :: "r"(1));
    __asm__ volatile("mcr p15, 0, %0, c9, c13, 1" :: "r"(0x11u));                   // counter 1 counts cycles
    __asm__ volatile("mcr p15, 0, %0, c9, c13, 2" :: "r"(-cycles));
    __asm__ volatile("mcr p15, 0, %0, c9, c12, 3" :: "r"(0xFFFFFFFFu));
    __asm__ volatile("mcr p15, 0, %0, c9, c14, 1" :: "r"(2));                       // PMINTENSET: counter 1
    __asm__ volatile("mcr p15, 0, %0, c9, c12, 1" :: "r"(2));                       // PMCNTENSET: counter 1 on
}

void bbb_exit(uint32_t code);
unsigned buffee_irq_c(void)
{
    uint32_t n = REG32(INTC_SIR_IRQ) & 0x7F;
    unsigned lvl = 0;
    if (n == 3) {
        __asm__ volatile("mcr p15, 0, %0, c9, c12, 3" :: "r"(2));                   // PMOVSR: counter 1's overflow
        __asm__ volatile("mcr p15, 0, %0, c9, c12, 5" :: "r"(1));
        __asm__ volatile("mcr p15, 0, %0, c9, c13, 2" :: "r"(-sample_period));
        uint32_t h = buffee_irq_pc; ++sample_n;
        buffee_last_pc = h; buffee_last_r4 = buffee_irq_r4;
        if (sample_stop && sample_n >= sample_stop) { buffee_sample_stopped = 1; lvl = 8; }        // ends pjit_run() like a RESET: the caller inspects the machine
        else lvl = buffee_ipl();
        if (sample_verbose) { printf("[PC %u] r4 %08X guest %08X host %08X\n", (unsigned)sample_n, buffee_irq_r4, guest_pc_of(h), h); }
        if (sample_verbose && sample_n % 10 == 0) buffee_smc_report();
    } else if (n == 32) {
        uint32_t st = REG32(GPIO2 + G_IRQSTATUS0) & IPL_MASK;
        REG32(GPIO2 + G_IRQSTATUS0) = st;
        buffee_ipl_events++;
        lvl = buffee_ipl();
    } else if (n == 62) {
        uint32_t st = REG32(GPIO3 + G_IRQSTATUS0) & RESET_BIT;
        REG32(GPIO3 + G_IRQSTATUS0) = st;
        if (st) { buffee_reset_events++; buffee_reset_pending = 1; lvl = 8; }     // level 8 = RESET: pjit_run() returns 0x100
        else lvl = buffee_ipl();
    } else if (n == 72) {                               // UART0 receive (the bootloader left the interrupt on): ESC three times in a row = back to the bootloader's menu
        static uint32_t esc_run;
        while (REG32(UART0_BASE + 0x14) & 1) {          // LSR bit 0: a byte is waiting
            uint8_t c = (uint8_t)REG32(UART0_BASE + 0x00);
            esc_run = (c == 0x1B) ? esc_run + 1 : 0;
            if (esc_run >= 3) {
                REG32(INTC_CONTROL) = 1; __asm__ volatile("dsb");
                printf("\n[BUFFEE] ESC x3: back to the bootloader\n");
                bbb_exit(BUFFEE_BREAK_EXIT);            // bbb_return works from any mode: guest code and bare-metal loops alike stop here
            }
        }
        lvl = buffee_ipl();
    } else if (n == 96) {
        uint32_t st = REG32(GPIO0 + G_IRQSTATUS0) & HALT_BIT;
        REG32(GPIO0 + G_IRQSTATUS0) = st;
        if (st) buffee_halt_events++;
        lvl = buffee_ipl();
    }
    REG32(INTC_CONTROL) = 1;                            // new IRQ agreement
    __asm__ volatile("dsb");
    return lvl;
}

// ---- unaligned longs on the Amiga window -------------------------------------------------------------------------------------------
/// A guest `ldr` / `str` (word) at an address that is not a multiple of 4 faults on Device memory (ARMv7: always). The 68000 allows it at any even address, so the abort
/// handler (boot.S) calls this with the saved registers: f[0] = the guest's r13, f[1] = r14, f[2+n] = r_n, f[15] = lr (the faulting instruction + 8). The access is redone as
/// two halfword accesses, big-endian as the guest sees them; the base register is written back as the instruction asked. Returns 0 for anything it does not understand.
static inline uint32_t gr(uint32_t *f, unsigned n) { return n < 13 ? f[2 + n] : n == 13 ? f[0] : f[1]; }
static inline void sr(uint32_t *f, unsigned n, uint32_t v) { if (n < 13) f[2 + n] = v; else if (n == 13) f[0] = v; else f[1] = v; }
uint16_t buffee_shadow[256];                                                // the last value the CPU wrote to each custom register (while tracing)
uint32_t buffee_wlog[2 * 4096], buffee_wlog_pos, buffee_wlog_on;      // debug: (address, value | size << 28) of the writes to the custom chips ($DFF000-$DFFFFF), when buffee_trace_custom() is on
uint32_t buffee_wlog_all;                                                    // 0: log only the blitter, DMACON and interrupt registers (the ring then holds a whole boot); 1: every custom register
extern uint32_t pjit_trace_frz __attribute__((weak));
#ifdef WTRACE
// WTRACE: a big ring (1 M entries) of the CPU's writes to the blitter registers ($040-$07F), DMACON and the interrupt registers, each with the size and the guest PC (cpu->ipc: exact in the interpreter), kept by
// buffee_trace_custom()'s permission faults and printed by the ESC report: what did the guest write to the blitter, and from where
#define WL_N (1u << 20)                                   /* 1 M entries x 12 bytes = 12 MB of free DDR at 0x92000000 (above the xmodem buffer and the unpack scratch; identity-mapped, cached) */
#define buffee_wl ((uint32_t *)0x92000000u)
uint32_t buffee_wl_pos;
static inline void wl_add(uint32_t a, uint32_t v, int size)
{
    uint32_t r = a & 0x1FE;
    if (*(volatile uint8_t *)&pjit_trace_frz) return;                        // frozen at the first exception outside the ROM (runtime.s, PJIT_ITRACE)
    if (!((r >= 0x040 && r < 0x09A) || (r >= 0x0E0 && r < 0x120))) return;                 // blitter, copper, DIW / DDF, DMACON; bitplane pointers, BPLCON / modulos (not INTENA / INTREQ: too noisy)
    uint32_t i = buffee_wl_pos++ & (WL_N - 1);
    buffee_wl[3 * i] = (a & 0xFFF) | (uint32_t)size << 28; buffee_wl[3 * i + 1] = v; buffee_wl[3 * i + 2] = __builtin_bswap32(cpu_state.ipc);
}
#else
#define wl_add(a, v, size) ((void)0)
#endif
#ifdef BLTIMM
// BLTIMM: 'immediate blitter' for a CPU that is much faster than a 68000 (WinUAE's 'wait for blitter'): after the CPU starts a blit (a write to BLTSIZE, or to ECS BLTSIZV / H) it waits until
// BBUSY clears. Programs written for a 68000 do other work while the blitter runs and test BBUSY afterwards (Planet Rocklobster's vector plots skip their blit setup when it is still busy),
// which only works if the CPU is as slow as the blitter expects. Needs the custom-chip section to fault on writes (buffee_trace_custom).
uint32_t buffee_blt_waits, buffee_blt_wait_long;
static void blt_imm(uint32_t a)
{
    uint32_t r = a & 0x1FE;
    if (r != 0x058 && r != 0x05C && r != 0x05E) return;
    volatile uint16_t *dmaconr = (volatile uint16_t *)(0x30000000u | 0xFF002u);       // through the writable alias; the first read after the write may miss BBUSY, so look twice
    uint32_t n = 0;
    (void)*dmaconr;
    while ((__builtin_bswap16(*dmaconr) & 0x4000) && n < 4000000) n++;
    buffee_blt_waits++; if (n > 1000) buffee_blt_wait_long++;
}
#else
#define blt_imm(a) ((void)0)
#endif
static inline int wlog_keep(uint32_t a) { uint32_t r = a & 0x1FE; return buffee_wlog_all || (r >= 0x040 && r < 0x060) || r == 0x096; }
void buffee_trace_custom(void)
{
    pagetable[0x300] = (0x02Du << 20) | SEC_DEVICE;                   // an alias of the custom-chip section that can be written
    pagetable[0x00D] = pagetable[0x01D] = (0x02Du << 20) | 0x8C06u;   // privileged read-only: every write is a permission fault that buffee_unaligned() performs through the alias and logs
    CP15DCacheCleanFlushBuff((uint32_t)&pagetable[0], 0x400 * 4);
    CP15TlbInvalidate();
    buffee_wlog_on = 1;
}
static void rawc(char c) { for (uint32_t spin = 0; (UART0_SSR & 1) && spin < 2000000; spin++) { } UART0_THR = (uint8_t)c; }
static void rawh(const char *t, uint32_t v) { while (*t) rawc(*t++); for (int i = 28; i >= 0; i -= 4) rawc("0123456789ABCDEF"[v >> i & 15]); rawc(' '); }

// ---- self-modifying code: write detection by page protection ----------------------------------------------------------------------
// The translating tiers (slot cache, blocks) copy guest code into host sequences, so a guest write to code that was already translated must throw the translation away. A 68000 has no
// instruction cache, so Kickstart never says so (library jump tables, SetFunction, MakeLibrary ...). The runtime announces every range of guest code it translates (pjit_platform_code):
//   * the 128-byte lines it covers are recorded in smc_lines (cpu->codebits: the slot router skips the announcement for a line that is already in), and
//   * the 4 KB pages it touches become READ-ONLY in the guest page table (RAM only: chip RAM, the slow RAM window, the guest DRAM; a 1 MB section is split into 4 KB pages when the first
//     page in it is protected; ROM stays sections).
// A guest write to a protected page is a permission fault; the abort handler emulates the store (the page is made writable around it) and, if it overlaps a recorded line, calls
// pjit_post_flush(): both tiers are flushed at the router's next entry (the faulting store sits inside a translated slot, so the flush cannot run under it). A data write to a page that
// merely shares a page with code costs one fault, not a flush (library bases sit next to their jump tables). A store the handler cannot decode unprotects the page, posts the flush and
// is retried. The flush (pjit_platform_flush) unprotects every page and empties the records. DMA masters (blitter, disk) write behind the CPU's back and are not seen.
// Tracked space: the guest megabytes that are translated (a region's, plus the MapROM megabyte and the low RAM a test maps) are numbered 0..smc_nmb-1 in the order their backing is
// allocated; smc_xlat[guest megabyte] = that number + 1 (0: not translated: the interpreter runs it, no bitmaps). The runtime reads the same table (cpu->xlat) for its translate test and its
// bitmap offsets: a guest address's tracked offset is (number << 20) | (address & 0xFFFFF). The bitmaps and the 1 KB coarse tables (one per tracked megabyte, for the 4 KB protection) sit in the DDR
// right behind the last region (smc_regions_init), so their size follows the regions.
#define POOL_BASE    0x80000000u
#define POOL_MAX_MB  495u                                     // [0x80000000, 0x9EF00000): below the slot cache's tags and the PJIT image
uint8_t pjit_pace_mb[4096] __attribute__((aligned(64)));      // 1 = the megabyte runs unpaced (a zone with flag F), read by the pace governor
static uint16_t smc_xlat[4096] __attribute__((aligned(64)));
static uint16_t smc_inv[512];                                 // tracked megabyte -> guest megabyte
static uint32_t smc_nmb, smc_nregion_mb, smc_rom_idx = ~0u, smc_low_idx = ~0u;
static uint32_t *smc_lines, *smc_exact, *smc_prot, (*smc_l2)[256];
static unsigned smc_l2used, smc_l2n;
static inline uint32_t smc_fold(uint32_t a) { uint32_t x = smc_xlat[a >> 20]; return x ? (((x - 1) << 20) | (a & 0xFFFFFu)) : ~0u; }
static inline uint32_t smc_unfold(uint32_t f) { return ((uint32_t)smc_inv[f >> 20] << 20) | (f & 0xFFFFFu); }
static struct { uint32_t a, n, last, lastsize; } smc_hot_hit[24], smc_hot_page[24];     // where the hits (128-byte line) and the faults (page) land: the most frequent
static uint32_t smc_note_exact, smc_note_size;
static void smc_note(typeof(smc_hot_hit) h, uint32_t a)
{
    int free_i = -1, min_i = 0;
    for (int i = 0; i < 24; i++) { if (h[i].n && h[i].a == a) { h[i].n++; h[i].last = smc_note_exact; h[i].lastsize = smc_note_size; return; } if (!h[i].n && free_i < 0) free_i = i; if (h[i].n < h[min_i].n) min_i = i; }
    int k = free_i >= 0 ? free_i : min_i; h[k].a = a; h[k].n = 1; h[k].last = smc_note_exact; h[k].lastsize = smc_note_size;
}
static uint32_t smc_nd_ins[16], smc_nd_n[16], smc_nd_pc[16], smc_nd_addr;                      // the instruction words (and a host PC) of the stores that were not decoded, with counts
static void smc_nd_note(uint32_t ins, uint32_t pc) { int fr = -1; for (int i = 0; i < 16; i++) { if (smc_nd_n[i] && smc_nd_ins[i] == ins) { smc_nd_n[i]++; return; } if (!smc_nd_n[i] && fr < 0) fr = i; } if (fr >= 0) { smc_nd_ins[fr] = ins; smc_nd_n[fr] = 1; smc_nd_pc[fr] = pc; } }
uint32_t buffee_smc_faults, buffee_smc_hits, buffee_smc_retry, buffee_smc_flushes, buffee_smc_prot_pages;   // statistics
extern void pjit_post_flush(void);
uint32_t *pjit_platform_codebits(void) { return smc_lines; }
uint32_t *pjit_platform_codeexact(void) { return smc_exact; }
uint16_t *pjit_platform_xlat(void) { return smc_xlat; }
static void smc_xlat_rom(int on) { if (smc_rom_idx != ~0u) smc_xlat[0xF] = smc_xlat[0x1F] = on ? smc_rom_idx + 1 : 0; }
static void smc_xlat_low(int on) { if (smc_low_idx != ~0u) smc_xlat[0] = smc_xlat[0x10] = on ? smc_low_idx + 1 : 0; }
static int smc_reserved(uint32_t s)                           // guest megabytes the platform keeps for itself (the Amiga window, peripherals, the DDR identity map, the custom-chip alias)
{
    return (s >= 0x20 && s < 0x30) || s == 0x300 || (s >= 0x400 && s < 0x404) || s == 0x44E || (s >= 0x480 && s < 0x483) || s == 0x500 || (s >= 0x800 && s < 0xA00);
}
/// The guest RAM regions of the EEPROM (bootinfo, bytes 16..31: base in MB, log2 size, flags, four times; no bootinfo: 128 MB at $08000000): each is mapped as 1 MB sections onto consecutive
/// megabytes of the DDR from physical 0x80000000 and numbered in the tracked space. A region is rejected (and so is every later one) when it is not a power of two in 1 MB ... 512 MB, runs
/// past 4 GB, overlaps a megabyte the platform needs or another region, or does not fit with its share of the tracking tables into the 495 MB pool. The tables (1 KB coarse table, 1 KB of line bits,
/// 8 KB of group bits and 32 bytes of page bits per tracked megabyte) follow the last region. Two more tracked numbers are reserved for the MapROM megabyte and for the low RAM that
/// bbb_map_low() gives a test. Regions below 16 MB are zeroed (exec probes memory there: stale contents of an earlier boot look like RAM that was in use).
static void smc_regions_init(void)
{
    struct { uint16_t base_mb; uint8_t l2, flags; } r[4] = { { 128, 27, 0 }, { 0, 0, 0 }, { 0, 0, 0 }, { 0, 0, 0 } };
    const volatile uint8_t *bi = (const volatile uint8_t *)0x9F5FFF00u;
    if (*(const volatile uint32_t *)bi == 0x544F4F42u)
        for (int k = 0; k < 4; k++) { r[k].base_mb = bi[16 + 4 * k] | bi[17 + 4 * k] << 8; r[k].l2 = bi[18 + 4 * k]; r[k].flags = bi[19 + 4 * k]; }
#ifdef FORCEB
    r[0].base_mb = 2; r[0].l2 = 23; r[0].flags = 7; r[1].l2 = 0;                                    // test: 8 MB of bus RAM at $200000 whatever the EEPROM says
#endif
#ifdef NOREGIONS
    for (int k = 0; k < 4; k++) r[k].l2 = 0;                           // test: no host RAM for the guest at all (chip RAM over the bus only: a stock 68000's speed)
#endif
#ifdef BUFFEE_SLOWRAM
    for (int k = 0; k < 4; k++) if (!r[k].l2) { r[k].base_mb = 12; r[k].l2 = 20; break; }       // test: 1 MB where an A500's trapdoor ('slow') RAM sits, $C00000
#endif
    int mode24 = *(const volatile uint32_t *)bi == 0x544F4F42u && (bi[9] & 4);          // BOOT_24BIT
    for (uint32_t i = 0; i < 4096; i++) { smc_xlat[i] = 0; pjit_pace_mb[i] = 0; }
    smc_nregion_mb = 0; smc_l2used = 0; buffee_nrg = 0;
    uint32_t ntrk = 0;                                                    // tracked megabytes so far (host DRAM regions and bus regions, in the order they are given); smc_nregion_mb counts only the pool
    for (int k = 0; k < 4 && r[k].l2; k++) {
        if (r[k].l2 < 20 || r[k].l2 > 29) break;
        uint32_t mb = 1u << (r[k].l2 - 20), base = r[k].base_mb;
        int bus = (r[k].flags & 2) != 0;                              // flags bit 1: RAM on the Amiga bus (a Zorro II board: expansion.library adds it): no host DRAM, the megabytes stay the window, their code is translated and cached
        if (base + mb > 4096) break;
        if (mode24 && base + mb > 16) continue;                      // (a region above the 16 MB space is out of reach with 24 address lines)
        int bad = 0;
        for (uint32_t i = 0; i < mb; i++) if (smc_reserved(base + i) || smc_xlat[base + i]) bad = 1;
        uint32_t nmb = ntrk + mb + 2, meta_mb = (nmb * 10272u + 4096u + 0xFFFFFu) >> 20;
        if (bad || smc_nregion_mb + (bus ? 0 : mb) + meta_mb > POOL_MAX_MB) break;
        for (uint32_t i = 0; i < mb; i++) {
            if (!bus) pagetable[base + i] = ((POOL_BASE >> 20) + smc_nregion_mb + i) << 20 | SEC_DDR;
            if (!(r[k].flags & 1)) smc_xlat[base + i] = ntrk + i + 1;          // flags bit 0 = interpret only: the memory is there, but its code is never translated (and so runs at the pace governor's speed)
            smc_inv[ntrk + i] = base + i;
            if (r[k].flags & 4) pjit_pace_mb[base + i] = 1;                    // flags bit 2 = unpaced (Fast RAM): interpreted at full speed, not at 68000 pace
        }
        if (!bus && base < 16) for (volatile uint32_t *q = (volatile uint32_t *)(POOL_BASE + (smc_nregion_mb << 20)); q < (volatile uint32_t *)(POOL_BASE + ((smc_nregion_mb + mb) << 20)); q++) *q = 0;
        if (!bus && buffee_nrg < 4 && !(base * 0x100000ull < 0xE00000 && (base + mb) * 0x100000ull > 0xC00000)) { buffee_rg[buffee_nrg].base = base << 20; buffee_rg[buffee_nrg].size = mb << 20; buffee_nrg++; }
        if (!bus) smc_nregion_mb += mb;
        ntrk += mb;
    }
    if (mode24)                                                   // 24 address lines: A24-A31 are ignored, so every 16 MB block of the 4 GB is the first one (except what the platform needs at its own addresses)
        for (uint32_t i = 16; i < 4096; i++) if (!smc_reserved(i)) { pagetable[i] = pagetable[i & 15]; smc_xlat[i] = smc_xlat[i & 15]; pjit_pace_mb[i] = pjit_pace_mb[i & 15]; }
    smc_rom_idx = ntrk; smc_low_idx = ntrk + 1; smc_inv[smc_rom_idx] = 0xF; smc_inv[smc_low_idx] = 0;
    smc_nmb = ntrk + 2;
    uint32_t p = POOL_BASE + (smc_nregion_mb << 20), start = p;
    smc_l2 = (uint32_t (*)[256])p;     p += smc_nmb * 1024u; smc_l2n = smc_nmb;
    smc_lines = (uint32_t *)p;         p += smc_nmb * 1024u;
    smc_exact = (uint32_t *)p;         p += smc_nmb * 8192u + 64u;
    smc_prot = (uint32_t *)p;          p += smc_nmb * 32u;
    for (volatile uint32_t *q = (volatile uint32_t *)start; q < (volatile uint32_t *)p; q++) *q = 0;
    printf("[MEM] %u MB of guest RAM regions, tracking tables %u KB (%u tracked megabytes)\n", (unsigned)smc_nregion_mb, (unsigned)((p - start) >> 10), (unsigned)smc_nmb);
}
static inline int smc_ram_section(uint32_t s) { uint32_t x = smc_xlat[s]; return x && x - 1 != smc_rom_idx; }       // translated, writable memory (the MapROM megabyte is not)
static uint32_t *smc_pte(uint32_t a)                          // the small-page descriptor of guest address a (splitting its section first), or 0 if it is not protectable
{
    uint32_t s = a >> 20;
    if (!smc_ram_section(s)) return 0;
    if ((pagetable[s] & 3) == 2) {
        if (smc_l2used >= smc_l2n) return 0;
        uint32_t d = pagetable[s], base = d & 0xFFF00000u, attr = (d & 0x1000u) ? 0x7Eu : 0x36u;      // normal cached (TEX 1, C, B, AP 3) or device (B, AP 3), small page
        uint32_t *t = smc_l2[smc_l2used++];
        for (uint32_t i = 0; i < 256; i++) t[i] = (base + (i << 12)) | attr;
        CP15DCacheCleanFlushBuff((uint32_t)t, 1024);
        pagetable[s] = ((uint32_t)t & ~0x3FFu) | 1u;
        CP15DCacheCleanFlushBuff((uint32_t)&pagetable[s], 4);
        CP15TlbInvalidate();
    }
    if ((pagetable[s] & 3) != 1) return 0;
    return (uint32_t *)(pagetable[s] & ~0x3FFu) + ((a >> 12) & 255);
}
static void smc_set(uint32_t a, int ro)
{
    uint32_t *e = smc_pte(a);
    if (!e) return;
    *e = ro ? (*e | 0x200u) : (*e & ~0x200u);                // APX: read-only for every mode
    __asm__ volatile ("mcr p15, 0, %0, c7, c10, 1" :: "r"(e) : "memory");     // clean the descriptor
    __asm__ volatile ("dsb sy" ::: "memory");
    __asm__ volatile ("mcr p15, 0, %0, c8, c7, 1" :: "r"(a & ~0xFFFu) : "memory");  // TLBIMVA
    __asm__ volatile ("dsb sy\n isb" ::: "memory");
}
static void smc_announce(uint32_t start, uint32_t len);
#ifdef CRUMBS
/* hang diagnosis (-DCRUMBS): a ring of the last guest code ranges announced for translation and the last write faults, kept in DDR at 0x9F5FFC00 (it survives a watchdog reset; each entry is cleaned out of the data cache); read back by buffee_crumbs */
#define CRUMB_BASE ((volatile uint32_t *)0x9F5FFC00u)
static void crumb(uint32_t tag, uint32_t a, uint32_t b) { uint32_t n = CRUMB_BASE[0]++; volatile uint32_t *e = CRUMB_BASE + 4 + 4 * (n & 15); e[0] = tag; e[1] = a; e[2] = b; e[3] = n; __asm__ volatile ("mcr p15, 0, %0, c7, c10, 1\n mcr p15, 0, %1, c7, c10, 1\n dsb sy" :: "r"(e), "r"(CRUMB_BASE) : "memory"); }
#else
#define crumb(t, a, b) ((void)0)
#endif
void pjit_platform_line(uint32_t start, uint32_t len) { crumb(1, start, len); smc_announce(start, len); }       // the slot router: a 128-byte line is about to be translated (its groups are marked by the fill itself)
void pjit_platform_code(uint32_t start, uint32_t len)           // the block translator: [start, start + len) was read as code
{
    crumb(2, start, len);
    if (smc_fold(start) == ~0u || !len) return;
    uint32_t e = start + len; if ((e - 1) >> 20 != start >> 20) e = (start | 0xFFFFFu) + 1;       // (stay in the same megabyte)
    for (uint32_t g = smc_fold(start) >> 4; g <= smc_fold(e - 1) >> 4; g++) smc_exact[g >> 5] |= 1u << (g & 31);
    smc_announce(start, len);
}
static void smc_announce(uint32_t start, uint32_t len)
{
    if (smc_fold(start) == ~0u || !len) return;
    uint32_t end = start + len; if ((end - 1) >> 20 != start >> 20) end = (start | 0xFFFFFu) + 1;
    for (uint32_t a = start & ~127u; a < end; a += 128) { uint32_t f = smc_fold(a); smc_lines[f >> 12] |= 1u << ((f >> 7) & 31); }
    for (uint32_t p = start & ~0xFFFu; p < end; p += 4096) {
        uint32_t pg = smc_fold(p) >> 12;
        if (smc_prot[pg >> 5] & (1u << (pg & 31))) continue;
        if (!smc_ram_section(p >> 20) || !smc_pte(p)) continue;
        smc_set(p, 1); smc_prot[pg >> 5] |= 1u << (pg & 31); buffee_smc_prot_pages++;
    }
}
void pjit_platform_flush(void)                                // every translation is gone: nothing needs protecting
{
    buffee_smc_flushes++;
    for (uint32_t w = 0; w < smc_nmb * 8; w++) {
        uint32_t m = smc_prot[w];
        if (!m) continue;
        for (int b = 0; b < 32; b++) if (m & (1u << b)) smc_set(smc_unfold((w * 32 + b) << 12), 0);
        smc_prot[w] = 0;
        for (int b = 0; b < 32; b++) if (m & (1u << b)) { uint32_t pg = w * 32 + b; smc_lines[pg] = 0; for (int k = 0; k < 8; k++) smc_exact[pg * 8 + k] = 0; }
    }
}
static inline int smc_protected(uint32_t a) { uint32_t f = smc_fold(a); return f != ~0u && (smc_prot[f >> 17] & (1u << ((f >> 12) & 31))); }
static int smc_line_hit(uint32_t a, uint32_t n)               // does [a, a + n) overlap a translated 16-byte group?
{
    for (uint32_t x = a & ~15u; x < a + n; x += 16) { uint32_t g = smc_fold(x); if (g == ~0u) continue; g >>= 4; if (smc_exact[g >> 5] & (1u << (g & 31))) return 1; }
    return 0;
}
// Performs a guest store of `size` bytes of the guest register value d (big-endian memory order) at a, with the pages it covers writable around it. Returns 1 if it overlapped
// translated code (the flush has been posted and the pages stay writable until it runs).
static int smc_store(uint32_t a, uint32_t d, int size)
{
    uint32_t p0 = a & ~0xFFFu, p1 = (a + size - 1) & ~0xFFFu;
    int hit = smc_line_hit(a, size);                              // (the fill marks the group after its own: an instruction's extension words are covered)
    int ro0 = smc_protected(p0), ro1 = p1 != p0 && smc_protected(p1);
    if (ro0) smc_set(p0, 0);
    if (ro1) smc_set(p1, 0);
    if (size == 4) { volatile uint16_t *p = (volatile uint16_t *)a; p[0] = __builtin_bswap16(d >> 16); p[1] = __builtin_bswap16(d); }
    else if (size == 2) *(volatile uint16_t *)a = __builtin_bswap16(d);
    else *(volatile uint8_t *)a = (uint8_t)d;
    __asm__ volatile ("dsb sy" ::: "memory");
    smc_note_exact = a; smc_note_size = size; smc_note(smc_hot_page, a & ~0xFFFu);
    if (hit) { buffee_smc_hits++; smc_note(smc_hot_hit, a & ~127u); pjit_post_flush(); }
    else { if (ro0) smc_set(p0, 1); if (ro1) smc_set(p1, 1); }
    return hit;
}
static int buffee_smc_fault(uint32_t *f, uint32_t dfsr, uint32_t dfar);
void buffee_smc_report(void) { printf("SMC: %u write faults in protected pages (%u hit translated code, %u not decoded), %u flushes, %u pages protected so far, %u coarse tables\n", (unsigned)buffee_smc_faults, (unsigned)buffee_smc_hits, (unsigned)buffee_smc_retry, (unsigned)buffee_smc_flushes, (unsigned)buffee_smc_prot_pages, smc_l2used); 
    for (int pass = 0; pass < 2; pass++) { typeof(smc_hot_hit) *h = pass ? &smc_hot_page : &smc_hot_hit; printf("SMC top %s:", pass ? "pages" : "hit lines");
        for (int r = 0; r < 6; r++) { int b = 0; for (int i = 1; i < 24; i++) if ((*h)[i].n > (*h)[b].n) b = i; if (!(*h)[b].n) break; printf(" %06X x%u (last write %06X size %u)", (unsigned)(*h)[b].a, (unsigned)(*h)[b].n, (unsigned)(*h)[b].last, (unsigned)(*h)[b].lastsize); if (!pass && r < 3) { printf("\n   mem:"); for (uint32_t k = 0; k < 64; k += 2) printf(" %04X", __builtin_bswap16(*(volatile uint16_t *)((*h)[b].a + k))); printf("\n"); } (*h)[b].n = 0; } printf("\n"); }
    printf("SMC retried stores: %u address not decoded; undecoded forms (instruction word x count @ host pc):", (unsigned)smc_nd_addr);
    for (int i = 0; i < 16; i++) if (smc_nd_n[i]) printf(" %08X x%u @%08X", (unsigned)smc_nd_ins[i], (unsigned)smc_nd_n[i], (unsigned)smc_nd_pc[i]);
    printf("\n");
}
static int buffee_unaligned_inner(uint32_t *f, uint32_t spsr, uint32_t dfsr, uint32_t dfar);
extern uint32_t pjit_trace[] __attribute__((weak)), pjit_trace_pos __attribute__((weak));
extern uint32_t pjit_trace_frz __attribute__((weak));
extern uint16_t pjit_xmem[320] __attribute__((weak));
#define pjit_trace_frz_p (&pjit_trace_frz)    // the interpreter's ring of (guest PC, opcode), big-endian words (tests/buffee/buffee_boot.c PCTRACE)
// ---- abort to exception ------------------------------------------------------------------------------------------------------------
// A data abort in guest code that nothing above emulates is what a 68000 calls a bus error (an unmapped address) or an address error (a word / long at an odd address): the guest gets
// the exception, with the group-0 frame (status word, access address, instruction word, SR, PC), instead of the host dying. The abort handler (boot.S) returns 'on to the next
// instruction' with the saved registers patched: r0 = the fault address, r1 = the function-code hint, lr = pjit_abort_bus / pjit_abort_addr (+4), so the exception return lands on an `svc`
// in System mode with the guest's registers and flags back (pjit/runtime.s). The guest PC: a fault in the interpreter has it in cpu->ipc; a fault in a slot is worked out from the slot's tag
// (the same recovery as the svc entry's: the end of the instruction whose last slot it is), also from the guest's lr when a shared body that a slot `bl`'d into faulted; a fault
// inside a translated BLOCK has no better PC than the last one stored in cpu->ipc (blocks only store it before sequences that can trap by themselves), so a bus error there reports
// that. A fault while a frame is being built (pjit_exc_begin..pjit_exc_end) is the 68000's double bus fault: HALT, here the fatal dump.
extern const char pjit_exc_begin[], pjit_exc_end[], pjit_abort_bus[], pjit_abort_addr[];
uint32_t buffee_abort_bus, buffee_abort_addr, buffee_abort_last_pc, buffee_abort_last_addr;
static int buffee_guest_abort(uint32_t *f, uint32_t spsr, uint32_t dfsr, uint32_t dfar)
{
    if ((spsr & 0x1F) != 0x1F) return 0;                                      // not the guest's world (System mode)
    uint32_t hpc = f[15] - 8;
    if (hpc - (uint32_t)pjit_exc_begin < (uint32_t)(pjit_exc_end - pjit_exc_begin)) return 0;     // a fault while delivering one
    int addr = (dfsr & 0x40F) == 1;                                           // alignment fault = an odd word / long access
    uint32_t base = (uint32_t)cpu->cache_data, off = hpc - base, lr = f[1], pc = 0;
    if (off >= DR_CACHE_BYTES) { off = ((lr + 15) & ~15u) - base; off = off < DR_CACHE_BYTES ? off : ~0u; pc = off == ~0u ? 0 : 1; }
    else pc = 2;
    if (pc) {                                                                 // a slot, or a shared body called from one
        uint32_t tag = cpu->tags[off >> DR_HL_SHIFT];
        if (tag == 0xFFFF) pc = 0;
        else { uint32_t g = (off >> 3) | (tag << DR_TAG_SHIFT); pc = (pc == 2) ? (g & ~1u) + 2 : g; cpu->ipc = __builtin_bswap32(pc); }
    }
    cpu->spill[7] = 1;                                                        // 'cpu->ipc holds the PC' for the exception entry
    buffee_abort_last_pc = __builtin_bswap32(cpu->ipc); buffee_abort_last_addr = dfar;
    if (addr) buffee_abort_addr++; else buffee_abort_bus++;
    f[2] = dfar;                                                              // r0
    f[3] = (dfsr & 0x800) ? 0 : 0x10;                                         // r1: data space; bit 4 = read
    f[15] = (uint32_t)(addr ? pjit_abort_addr : pjit_abort_bus) + 4;
    return 1;
}
int buffee_unaligned(uint32_t *f, uint32_t spsr, uint32_t dfsr, uint32_t dfar)
{
#ifdef ABORT_TRACE
    rawh("\r\n[abt in] dfsr ", dfsr); rawh("dfar ", dfar); rawh("pc ", f[15] - 8); rawh("spsr ", spsr); rawh("r5 ", f[7]); rawh("lr ", f[1]); rawc('\r'); rawc('\n');
#endif
    int r = buffee_unaligned_inner(f, spsr, dfsr, dfar);
#ifndef NO_GUEST_ABORT
    if (!r) r = buffee_guest_abort(f, spsr, dfsr, dfar);
#endif
    if (!r) { rawh("\r\n[ABT] dfsr ", dfsr); rawh("dfar ", dfar); rawh("pc ", f[15] - 8); rawh("ins ", *(volatile uint32_t *)(f[15] - 8)); rawh("spsr ", spsr); rawh("sp ", f[0]); rawc('\r'); rawc('\n');
        if (f[0] < 0x200000) { rawh("guest stack:", 0); for (uint32_t k = 0; k < 24; k++) { volatile uint16_t *q = (volatile uint16_t *)(f[0] + 4 * k); rawh("", (uint32_t)__builtin_bswap16(q[0]) << 16 | __builtin_bswap16(q[1])); } rawc('\r'); rawc('\n'); }
        if (&pjit_trace_pos && pjit_trace_pos) { uint32_t pos = __builtin_bswap32(pjit_trace_pos); rawh("last guest PC / opcode (oldest first):", 0); rawc('\r'); rawc('\n');
            for (int k = 40; k >= 1; k--) { uint32_t o = (pos - 8u * k) & 0x7FFFF; rawh("  ", __builtin_bswap32(pjit_trace[o / 4])); rawh("", __builtin_bswap32(pjit_trace[o / 4 + 1]) & 0xFFFF); if (k % 4 == 1) { rawc('\r'); rawc('\n'); } } }
        rawh("exceptions delivered (vector, guest PC), oldest first:", 0); rawc('\r'); rawc('\n');
        for (int j = 16; j >= 1; j--) { uint32_t e = ((pjit_dlog_pos / 8 - j) & 31) * 2; rawh(" ", pjit_dlog[e]); rawh("@", pjit_dlog[e + 1]); if (j % 4 == 1) { rawc('\r'); rawc('\n'); } }
        { rawh("frames (sp after push, new SR, frame at push | frame now), oldest first:", 0); rawc('\r'); rawc('\n');
          uint32_t pos = __builtin_bswap32(pjit_flog[0]);
          for (int j = 6; j >= 1; j--) { uint32_t e = 1 + 4 * (((pos / 16) - j) & 15); uint32_t sp = __builtin_bswap32(pjit_flog[e]);
              rawh(" sp", sp); rawh("sr", __builtin_bswap32(pjit_flog[e + 1])); rawh("push", __builtin_bswap32(pjit_flog[e + 2])); rawh("", __builtin_bswap32(pjit_flog[e + 3]));
              if (sp < 0x200000) { volatile uint16_t *q = (volatile uint16_t *)sp; rawh("| now", (uint32_t)__builtin_bswap16(q[0]) << 16 | __builtin_bswap16(q[1])); rawh("", (uint32_t)__builtin_bswap16(q[2]) << 16 | __builtin_bswap16(q[3])); }
              rawc('\r'); rawc('\n'); } }
        { rawh("guest 0x00-0x40:", 0); for (uint32_t a = 0; a < 0x40; a += 4) { volatile uint16_t *q = (volatile uint16_t *)a; rawh("", (uint32_t)__builtin_bswap16(q[0]) << 16 | __builtin_bswap16(q[1])); } rawc('\r'); rawc('\n'); }
        { uint32_t usp = __builtin_bswap32(cpu->usp), ssp = __builtin_bswap32(cpu->ssp); rawh("SR", __builtin_bswap16(cpu->sr)); rawh("USP", usp); rawh("SSP", ssp); rawc('\r'); rawc('\n');
          for (int w = 0; w < 2; w++) { uint32_t a = w ? ssp : usp; if (a < 0x200000) { rawh(w ? "SSP stack:" : "USP stack:", 0); for (uint32_t k = 0; k < 12; k++) { volatile uint16_t *q = (volatile uint16_t *)(a + 4 * k); rawh("", (uint32_t)__builtin_bswap16(q[0]) << 16 | __builtin_bswap16(q[1])); } rawc('\r'); rawc('\n'); } } }
        rawh("regs r0-r12:", 0); for (int k = 2; k < 15; k++) rawh("", f[k]); rawc('\r'); rawc('\n'); }
    return r;
}
static int buffee_smc_fault(uint32_t *f, uint32_t dfsr, uint32_t dfar)   // 0: not ours, 1: store emulated (skip it), 2: unprotected + flush posted (retry it)
{
    if (((dfsr & 0x40F) != 0xF && (dfsr & 0x40F) != 0xD) || (dfsr & 0x800) == 0 || !smc_protected(dfar)) return 0;     // a write (WnR) permission fault in a protected page
    buffee_smc_faults++;
    crumb(3, dfar, f[15]);
    uint32_t ins = *(volatile uint32_t *)(f[15] - 8);
    unsigned rn = ins >> 16 & 15, rt = ins >> 12 & 15;
    int half = ((ins >> 25) & 7) == 0 && ((ins >> 4) & 15) == 0xB && !(ins & (1u << 20));
    int word = ((ins >> 26) & 3) == 1 && !(ins & (1u << 20));
    int ok = (half || word) && rn != 15 && rt != 15;
    uint32_t off = 0;
    if (ok) {
        if (half) { if (ins & (1u << 22)) off = (ins >> 4 & 0xF0) | (ins & 0xF); else off = gr(f, ins & 15); }
        else if (ins & (1u << 25)) {
            uint32_t m = gr(f, ins & 15), sh = ins >> 7 & 31;
            switch (ins >> 5 & 3) { case 0: off = m << sh; break; case 1: off = sh ? m >> sh : 0; break; case 2: off = sh ? (uint32_t)((int32_t)m >> sh) : (uint32_t)((int32_t)m >> 31); break; default: ok = 0; }
        } else off = ins & 0xFFF;
    }
    if (!ok && ((ins >> 25) & 7) == 4 && !(ins & (1u << 20)) && !(ins & (1u << 22))) {      // STM (any of IA / IB / DA / DB; MOVEM.L regs,-(A7) is stmdb sp!): word by word, ascending registers
        unsigned rn2 = ins >> 16 & 15, list = ins & 0xFFFF, n = 0;
        for (unsigned m = list; m; m &= m - 1) n++;
        int up = (ins >> 23) & 1, pre = (ins >> 24) & 1, wb2 = (ins >> 21) & 1;
        if (rn2 != 15 && !(list & 0x8000) && n && !(wb2 && ((list >> rn2) & 1))) {
            uint32_t b = gr(f, rn2), start = up ? (pre ? b + 4 : b) : (pre ? b - 4 * n : b - 4 * n + 4);
            if (dfar - start < 4 * n) {                             // (the faulting word is one of the list's)
                uint32_t k = 0;
                for (unsigned r = 0; r < 15; r++) if (list >> r & 1) smc_store(start + 4 * k++, gr(f, r), 4);
                if (wb2) sr(f, rn2, up ? b + 4 * n : b - 4 * n);
                return 1;
            }
        }
    }
    if (!ok) {                                                  // a form not decoded here (STRD, other STMs ...): unprotect the page, flush, retry the instruction
        smc_nd_note(ins, f[15] - 8);
        smc_set(dfar & ~0xFFFu, 0); pjit_post_flush(); buffee_smc_retry++;
        return 2;
    }
    uint32_t base = gr(f, rn), nb = (ins & (1u << 23)) ? base + off : base - off;
    int pre = (ins & (1u << 24)) != 0, wb = !pre || (ins & (1u << 21));
    uint32_t a = pre ? nb : base;
    int size = half ? 2 : (ins & (1u << 22)) ? 1 : 4;
    if (dfar - a > 3) { smc_nd_addr++; smc_set(dfar & ~0xFFFu, 0); pjit_post_flush(); buffee_smc_retry++; return 2; }    // (not the address decoded: do not guess)
    smc_store(a, gr(f, rt), size);
    if (wb) sr(f, rn, nb);
    return 1;
}
static int buffee_unaligned_inner(uint32_t *f, uint32_t spsr, uint32_t dfsr, uint32_t dfar)
{
    { int r = buffee_smc_fault(f, dfsr, dfar); if (r) return r; }
    if ((dfsr & 0x40F) == 0xD || (dfsr & 0x40F) == 0xF) {         // a write to a read-only custom-chip section (buffee_trace_custom)
        uint32_t ins = *(volatile uint32_t *)(f[15] - 8);
        unsigned rn = ins >> 16 & 15, rt = ins >> 12 & 15;
        int half = ((ins >> 25) & 7) == 0 && ((ins >> 4) & 15) == 0xB && !(ins & (1u << 20));
        int word = ((ins >> 26) & 3) == 1 && !(ins & (1u << 20));
        if (rn == 15 || rt == 15 || !(half || word)) return 0;
        uint32_t off;
        if (half) off = (ins & (1u << 22)) ? ((ins >> 4 & 0xF0) | (ins & 0xF)) : gr(f, ins & 15);
        else if (ins & (1u << 25)) {
            uint32_t m = gr(f, ins & 15), sh = ins >> 7 & 31;
            switch (ins >> 5 & 3) { case 0: off = m << sh; break; case 1: off = sh ? m >> sh : 0; break; case 2: off = sh ? (uint32_t)((int32_t)m >> sh) : (uint32_t)((int32_t)m >> 31); break; default: return 0; }
        } else off = ins & 0xFFF;
        uint32_t base = gr(f, rn), nb = (ins & (1u << 23)) ? base + off : base - off;
        int pre = (ins & (1u << 24)) != 0, wb = !pre || (ins & (1u << 21));
        uint32_t a = pre ? nb : base, d = gr(f, rt);
        if (dfar - a > 3) return 0;
        uint32_t alias = 0x30000000u | (a & 0xFFFFFu);
        int size = half ? 2 : (ins & (1u << 22)) ? 1 : 4;
        if (size == 4) { volatile uint16_t *p = (volatile uint16_t *)alias; p[0] = __builtin_bswap16(d >> 16); p[1] = __builtin_bswap16(d); }
        else if (size == 2) *(volatile uint16_t *)alias = __builtin_bswap16(d);
        else *(volatile uint8_t *)alias = (uint8_t)d;
        if (buffee_wlog_on && (a & 0xFFFFF) >= 0xFF000 && (a & 0xFFFFF) < 0xFF200) {
            uint32_t r = (a & 0x1FE) >> 1; if (size == 4) { buffee_shadow[r] = d >> 16; buffee_shadow[r + 1] = d; } else if (size == 2) buffee_shadow[r] = d;
            wl_add(a, size == 4 ? d : size == 2 ? (d & 0xFFFF) : (d & 0xFF), size);
            if (wlog_keep(a)) { uint32_t i = buffee_wlog_pos++ & 4095; buffee_wlog[2 * i] = a; buffee_wlog[2 * i + 1] = (size == 4 ? d : size == 2 ? (d & 0xFFFF) : (d & 0xFF)) ; buffee_wlog[2 * i] |= (uint32_t)size << 28; }
        }
        blt_imm(a);
        if (wb) sr(f, rn, nb);
        return 1;
    }
    if ((dfsr & 0x40F) != 1) return 0;                            // not an alignment fault
    uint32_t ins = *(volatile uint32_t *)(f[15] - 8);
    static const uint8_t cc[16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    (void)cc;
    uint32_t n = spsr >> 31, z = spsr >> 30 & 1, c = spsr >> 29 & 1, v = spsr >> 28 & 1;
    int ok;
    switch (ins >> 28) {
    case 0: ok = z; break;          case 1: ok = !z; break;
    case 2: ok = c; break;          case 3: ok = !c; break;
    case 4: ok = n; break;          case 5: ok = !n; break;
    case 6: ok = v; break;          case 7: ok = !v; break;
    case 8: ok = c && !z; break;    case 9: ok = !c || z; break;
    case 10: ok = n == v; break;    case 11: ok = n != v; break;
    case 12: ok = !z && n == v; break;  case 13: ok = z || n != v; break;
    default: ok = 1; break;
    }
    if (((ins >> 26) & 3) != 1 || (ins & (1u << 22))) return 0;   // not a word LDR / STR
    unsigned rn = ins >> 16 & 15, rt = ins >> 12 & 15;
    if (rn == 15 || rt == 15) return 0;
    if (!ok) return 1;                                            // condition false: the instruction did nothing (it still faulted at decode on some cores)
    uint32_t off;
    if (ins & (1u << 25)) {
        uint32_t m = gr(f, ins & 15), sh = ins >> 7 & 31;
        switch (ins >> 5 & 3) { case 0: off = m << sh; break; case 1: off = sh ? m >> sh : 0; break; case 2: off = sh ? (uint32_t)((int32_t)m >> sh) : (uint32_t)((int32_t)m >> 31); break; default: return 0; }
    } else off = ins & 0xFFF;
    uint32_t base = gr(f, rn), nb = (ins & (1u << 23)) ? base + off : base - off;
    int pre = (ins & (1u << 24)) != 0, wb = !pre || (ins & (1u << 21));
    uint32_t a = pre ? nb : base;
    if (a & 1) return 0;                                          // an odd address: the 68000's address error (buffee_guest_abort)
    if (dfar - a > 3) return 0;                                   // (an access that straddles two sections reports the second one)
    volatile uint16_t *p = (volatile uint16_t *)((buffee_wlog_on && ((a & 0xFFF00000u) == 0xD00000u || (a & 0xFFF00000u) == 0x1D00000u)) ? (0x30000000u | (a & 0xFFFFFu)) : a);   // (the custom chips are read-only while tracing: write through the alias)
    if (ins & (1u << 20)) sr(f, rt, (uint32_t)__builtin_bswap16(p[0]) << 16 | __builtin_bswap16(p[1]));
    else {
        uint32_t d = gr(f, rt);
        if (smc_protected(a) || smc_protected(a + 3)) smc_store(a, d, 4); else { p[0] = __builtin_bswap16(d >> 16); p[1] = __builtin_bswap16(d); }     // (a protected page would fault again inside the handler)
        if (buffee_wlog_on && (a & 0xFFFFF) >= 0xFF000 && (a & 0xFFFFF) < 0xFF200 && ((a & 0xFFF00000u) == 0xD00000u || (a & 0xFFF00000u) == 0x1D00000u)) { uint32_t r = (a & 0x1FE) >> 1; buffee_shadow[r] = d >> 16; buffee_shadow[r + 1] = d; wl_add(a, d, 4); if (wlog_keep(a)) { uint32_t i = buffee_wlog_pos++ & 4095; buffee_wlog[2 * i] = a | 4u << 28; buffee_wlog[2 * i + 1] = d; } }
    }
    if (wb) sr(f, rn, nb);
    return 1;
}

// ---- 68K function codes ---------------------------------------------------------------------------------------------------------
// FC0 is tied high on the board (the GreenPAK drives it to 1), FC1 and FC2 are GPIOs behind the level shifters: LCD_DATA3 = GPIO2_9 = FC1, LCD_DATA4 = GPIO2_10 = FC2 (bodged from the strap-resistor pads).
// FC = 001 user data, 101 supervisor data, 111 interrupt acknowledge, so FC1 = IACK and FC2 = IACK || supervisor. The Amiga ignores the lines; an Atari ST's GLUE needs the supervisor bit
// for its I/O decode and the IACK to clear its interrupt latches and to reach the MFP, a Mac's glue the IACK for its autovectors. The supervisor bit is kept by the generated SR-write
// sequences and the exception entry (tools/gen/gen.py _fc_sync, pjit/runtime.s under PJIT_FC); the SET / CLEAR registers are single-bit writes, so the IPL interrupt on the same bank cannot race them.
#define GPIO2_BASE   0x481AC000u
#define FC1_BIT      (1u << 9)
#define FC2_BIT      (1u << 10)
#define CTRL_LCD_DATA3 0x44E108ACu
#define CTRL_LCD_DATA4 0x44E108B0u
void buffee_fc_init(void)
{
    *(volatile uint32_t *)CTRL_LCD_DATA3 = 0x2Fu;                      // mode 7 (GPIO), pull disabled, receiver on (DATAIN reads the pad: a contention shows), output
    *(volatile uint32_t *)CTRL_LCD_DATA4 = 0x2Fu;
    *(volatile uint32_t *)(GPIO2_BASE + 0x190) = FC1_BIT;                // FC1 low (not an acknowledge)
    *(volatile uint32_t *)(GPIO2_BASE + 0x194) = FC2_BIT;                // FC2 high: the 68000 starts in supervisor state
    *(volatile uint32_t *)(GPIO2_BASE + 0x134) &= ~(FC1_BIT | FC2_BIT); // outputs
    __asm__ volatile ("dsb sy" : : : "memory");
}
unsigned buffee_fc_get(void) { return 1u | ((*(volatile uint32_t *)(GPIO2_BASE + 0x13C) & FC1_BIT) ? 2u : 0u) | ((*(volatile uint32_t *)(GPIO2_BASE + 0x13C) & FC2_BIT) ? 4u : 0u); }
void buffee_fc_supervisor(int on)
{
    *(volatile uint32_t *)(GPIO2_BASE + (on ? 0x194 : 0x190)) = FC2_BIT;
    __asm__ volatile ("dsb sy" : : : "memory");
}
// One interrupt-acknowledge bus cycle at `level` (1-7): FC = 111, address $FFFFF1 | level << 1 (A23-A4 high, A3-A1 the level; the byte read at an odd address comes from D7-D0, where a vectored device
// puts its vector; a glue that autovectors asserts VPA and the byte is not used). Read through the 24-bit alias of the window at $01FFFFF1 (the guest's $F00000 section may be the ROM). FC2 goes
// back to `supervisor` (the exception being processed runs in supervisor state, so 1).
uint8_t buffee_iack(unsigned level, int supervisor)
{
    *(volatile uint32_t *)(GPIO2_BASE + 0x194) = FC1_BIT | FC2_BIT;
    __asm__ volatile ("dsb sy" : : : "memory");
    uint8_t v = *(volatile uint8_t *)(0x01FFFFF1u | ((level & 7u) << 1));
    __asm__ volatile ("dsb sy" : : : "memory");
    *(volatile uint32_t *)(GPIO2_BASE + 0x190) = FC1_BIT;
    *(volatile uint32_t *)(GPIO2_BASE + (supervisor ? 0x194 : 0x190)) = FC2_BIT;
    __asm__ volatile ("dsb sy" : : : "memory");
    return v;
}

// ---- boot and exit ----------------------------------------------------------------------------------------------------------------
extern void bbb_return(uint32_t code);                  // boot.S

void bbb_init(void)
{
    mmu_init();
    buffee_fc_init();
    {   // the bootloader may have SCTLR.A (alignment check) set; 68K code does word-aligned longword accesses that the A8 handles in hardware once A is clear
        uint32_t v; __asm__ volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r"(v));
        v &= ~2u; __asm__ volatile ("mcr p15, 0, %0, c1, c0, 0\n isb" : : "r"(v));
    }
    static const uint8_t ev[4] = { PMU_INSTR, PMU_BR_MISPRED, PMU_L1D_MISS, PMU_L1I_MISS };
    bbb_pmu_start(ev);
    printf("[BUFFEE] up: SCTLR=%08X, IPL level %u\n", ({ uint32_t v; __asm__ volatile ("mrc p15, 0, %0, c1, c0, 0" : "=r"(v)); v; }), buffee_ipl());
}

void bbb_exit(uint32_t code)
{
    printf("[BUFFEE] exit %u\n", (unsigned)code);
    buffee_smc_report();
    printf("LIVE: %u exceptions delivered in total, guest PC at the ESC interrupt %08X (host PC %08X)\n", (unsigned)pjit_dlog_count, (unsigned)guest_pc_of(buffee_irq_pc), (unsigned)buffee_irq_pc);
    if (code) { printf("REGS:"); for (int i = 0; i < 8; i++) printf(" D%d=%08X", i, (unsigned)pjit_get_dn(i)); for (int i = 0; i < 8; i++) printf(" A%d=%08X", i, (unsigned)pjit_get_an(i)); printf("\n"); }   // (a bare payload that returned 0 never ran a guest: r5 is not the CPU base)
    printf("TRAPS (oldest first; vector@PC after the instruction, the words from PC-8 to PC+4):\n");
    for (int k = 16; k >= 1; k--) {
        uint32_t e = ((pjit_xlog_pos / 8 - k) & 15) * 2, pc = pjit_xlog[e + 1];
        if (!pjit_xlog[e] && !pc) continue;
        printf("  %X@%X:", (unsigned)pjit_xlog[e], (unsigned)pc);
        if (pc >= 8 && pc < 0x1000000) for (int d = -8; d <= 4; d += 2) printf(" %04X", (unsigned)read_BE_word(pc + d));
        printf("\n");
        if (pc >= 0x1000 && pc < 0x200000) {                                  // the memory before the PC, to see whether the bad words are one or a pattern (a blit's rows, a stray store)
            printf("  mem %06X..%06X:", (unsigned)(pc - 256), (unsigned)(pc + 32));
            for (uint32_t a = pc - 256; a < pc + 32; a += 2) { if (((a - (pc - 256)) & 31) == 0) printf("\n   %06X:", (unsigned)a); printf(" %04X", (unsigned)read_BE_word(a)); }
            printf("\n");
        }
    }
    if (pjit_trace && pjit_trace_frz_p) {
        printf("ITRACE (%s; oldest first, PC:opcode):", *pjit_trace_frz_p ? "frozen at the illegal instruction" : "NOT frozen: no trap outside the ROM yet");
        uint32_t end = __builtin_bswap32(pjit_trace_pos) / 8, run = 0;           // entries before `end`; skip a trailing run of equal opcodes at consecutive 4-byte steps (a guest sliding through a table / a blit's pattern)
        #define TR_PC(i)  (__builtin_bswap32(pjit_trace[(((i) & 0xFFFF)) * 2]) & 0xFFFFFF)
        #define TR_OP(i)  (__builtin_bswap32(pjit_trace[(((i) & 0xFFFF)) * 2 + 1]) & 0xFFFF)
        while (run < 60000 && TR_PC(end - 1 - run - 1) + 4 == TR_PC(end - 1 - run) && TR_OP(end - 1 - run - 1) == TR_OP(end - 1 - run)) run++;
        if (run > 4) printf(" [... %u sequential %04X at 4-byte steps, %06X..%06X ...]", (unsigned)run, (unsigned)TR_OP(end - 1), (unsigned)TR_PC(end - 1 - run), (unsigned)TR_PC(end - 1));
        else run = 0;
        for (int j = 64; j >= 1; j--) { uint32_t i = end - run - j; printf(" %06X:%04X", (unsigned)TR_PC(i), (unsigned)TR_OP(i)); }
        printf("\n");
    }
#ifdef WTRACE
    {   uint32_t n = buffee_wl_pos, from = n;
        for (uint32_t got = 0; from > 0 && got < 600; ) { from--; got++; }
        printf("WTRACE (%s): %u blitter / copper / display register writes in all; the last %u, oldest first, reg=value/size@PC:", *(volatile uint8_t *)&pjit_trace_frz ? "frozen at the exception" : "NOT frozen", (unsigned)n, (unsigned)(n - from));
        for (uint32_t j = from; j < n; j++) { uint32_t i = j & (WL_N - 1), w = buffee_wl[3 * i]; if ((j - from) % 5 == 0) printf("\n "); printf(" %03X=%X/%u@%X", (unsigned)(w & 0xFFF), (unsigned)buffee_wl[3 * i + 1], (unsigned)(w >> 28), (unsigned)buffee_wl[3 * i + 2]); }
        printf("\n");
    }
#endif
#ifdef PACE
    { extern uint32_t pjit_pace_wraps __attribute__((weak)), pjit_pace_dl[8] __attribute__((weak)), pjit_pace_k __attribute__((weak)), pjit_pace_gap[34] __attribute__((weak));
      printf("PACE: %u counter-wrap resyncs (the guest was away from the interpreter for more than 2.1 s); K %u ARM cycles per 68000 cycle; the virtual clock is %d ns ahead (+) / behind (-) of the counter now\n",
             (unsigned)__builtin_bswap32(pjit_pace_wraps), (unsigned)__builtin_bswap32(pjit_pace_k), (int)(__builtin_bswap32(pjit_pace_dl[0]) - bbb_pmu_cycles()));
      printf("PACE lag: %u resyncs (more than 20 us behind the virtual 68000), %u us behind in total, the worst %u us at guest PC %X; %u of them over 1 ms\n", (unsigned)__builtin_bswap32(pjit_pace_dl[4]), (unsigned)__builtin_bswap32(pjit_pace_dl[3]), (unsigned)(__builtin_bswap32(pjit_pace_dl[5]) / 1000), (unsigned)__builtin_bswap32(pjit_pace_dl[6]), (unsigned)__builtin_bswap32(pjit_pace_dl[7]));
      uint32_t n = __builtin_bswap32(pjit_pace_gap[0]);
      printf("PACE gaps > 17 ms between two interpreted instructions: %u in all; the last %u, oldest first: gap ms @ guest PC of the instruction that took it:", (unsigned)n, (unsigned)(n > 16 ? 16 : n));
      for (uint32_t j = n > 16 ? n - 15 : 1; j <= n; j++) { uint32_t e = (j & 15) * 2 + 2; printf(" %u.%03u@%X", (unsigned)(__builtin_bswap32(pjit_pace_gap[e]) / 1000000), (unsigned)(__builtin_bswap32(pjit_pace_gap[e]) / 1000 % 1000), (unsigned)__builtin_bswap32(pjit_pace_gap[e + 1])); }
      printf("\n"); }
#endif
#ifdef VBLLOG
    { extern uint8_t pjit_vbl_ring[] __attribute__((weak)); extern uint32_t pjit_vbl_pos __attribute__((weak));
      uint32_t total = pjit_vbl_pos / 16, n = total > 16384 ? 16384 : total, first = total - n;
      #define VR(i, w) (((volatile uint32_t *)pjit_vbl_ring)[((((i) & 16383u) * 4) + (w))])
      /* 20 ms windows of the cycle counter (1 ns per count): the governor's lag in each, the gaps between deliveries */
      uint32_t gl[8] = {0}, gt[8] = {0}, gpc[8] = {0}, hist[6] = {0}, gaps[4] = {0}, nwin = 0, wl[16] = {0}, wt[16] = {0}, wpc[16] = {0}, wdt[16] = {0};
      uint32_t t0 = 0, lag0 = 0, tw = 0; int started = 0;
      for (uint32_t i = first; i < total; i++) {
          uint32_t t = VR(i, 0), lag = __builtin_bswap32(VR(i, 3));
          if (!started) { t0 = tw = t; lag0 = lag; started = 1; continue; }
          uint32_t gap = t - VR(i - 1, 0);
          { int w = 0; for (int k = 1; k < 8; k++) if (gl[k] < gl[w]) w = k; if (gap > gl[w]) { gl[w] = gap; gt[w] = (t - t0) / 1000000; gpc[w] = VR(i - 1, 2); } }
          if (gap > 25000000u) gaps[0]++;
          if (gap > 40000000u) gaps[1]++;
          if (gap > 100000000u) gaps[2]++;
          if (t - tw >= 20000000u) {
              uint32_t l = lag - lag0; nwin++;
              hist[l < 100 ? 0 : l < 500 ? 1 : l < 2000 ? 2 : l < 5000 ? 3 : l < 10000 ? 4 : 5]++;
              int w = 0; for (int k = 1; k < 16; k++) if (wl[k] < wl[w]) w = k;
              if (l > wl[w]) { wl[w] = l; wt[w] = (t - t0) / 1000000; wpc[w] = VR(i, 2); wdt[w] = t - tw; }
              tw = t; lag0 = lag;
          }
      }
      printf("VBLLOG: %u exceptions delivered (%u kept); in %u windows of 20 ms the governor lost (us): <100 %u, <500 %u, <2000 %u, <5000 %u, <10000 %u, more %u; gaps between deliveries: >25 ms %u, >40 ms %u, >100 ms %u\n",
             (unsigned)total, (unsigned)n, (unsigned)nwin, (unsigned)hist[0], (unsigned)hist[1], (unsigned)hist[2], (unsigned)hist[3], (unsigned)hist[4], (unsigned)hist[5], (unsigned)gaps[0], (unsigned)gaps[1], (unsigned)gaps[2]);
      printf("VBLLOG worst windows (us lost @ ms into the kept span, window ms, PC of the delivery that closed it):");
      for (int k = 0; k < 16; k++) if (wl[k]) printf(" %u@%u/%ums/%X", (unsigned)wl[k], (unsigned)wt[k], (unsigned)(wdt[k] / 1000000), (unsigned)wpc[k]);
      printf("\n");
      /* the 160 deliveries before the end: time since the previous one in us, vector, PC */
      printf("VBLLOG longest gaps between deliveries (ms @ ms into the span, PC of the delivery before):");
      for (int k = 0; k < 8; k++) if (gl[k]) printf(" %u@%u/%X", (unsigned)(gl[k] / 1000000), (unsigned)gt[k], (unsigned)gpc[k]);
      printf("\n");
      printf("VBLLOG last 40 (dt us:vector@PC):");
      for (uint32_t i = total - (total > 40 ? 40 : total) + 1; i < total; i++) printf(" %u:%X@%X", (unsigned)((VR(i, 0) - VR(i - 1, 0)) / 1000), (unsigned)VR(i, 1), (unsigned)VR(i, 2));
      printf("\n"); }
#endif
#ifdef BLTIMM
    printf("BLTIMM: the CPU waited for the blitter %u times after BLTSIZE writes (%u of them for more than ~1000 polls)\n", (unsigned)buffee_blt_waits, (unsigned)buffee_blt_wait_long);
#endif
    if (pjit_xmem && pjit_trace_frz_p && *(volatile uint8_t *)&pjit_trace_frz) {
        uint32_t pc = 0; for (int k = 1; k <= 16; k++) { uint32_t e = ((pjit_xlog_pos / 8 - k) & 15) * 2; if (pjit_xlog[e] && !(pjit_xlog[e + 1] - 0xF80000 < 0x80000)) { pc = pjit_xlog[e + 1]; break; } }
        printf("SNAPSHOT of guest memory at the exception, %06X..%06X (PC %06X at +512):", (unsigned)(pc - 512), (unsigned)(pc + 128), (unsigned)pc);
        for (int i = 0; i < 320; i++) { if (i % 16 == 0) printf("\n   %06X:", (unsigned)(pc - 512 + 2 * i)); printf(" %04X", (unsigned)__builtin_bswap16(pjit_xmem[i])); }
        printf("\n");
    }
    if (code) { printf("VECTORS (guest $00..$7E, chip RAM):"); for (uint32_t a = 0; a < 0x80; a += 2) { if (a % 32 == 0) printf("\n   %02X:", (unsigned)a); printf(" %04X", (unsigned)read_BE_word(a)); } printf("\n"); }
    { extern uint32_t pjit_vdbg[] __attribute__((weak)); if (pjit_vdbg) { printf("VECREAD (vector: first/second read, recorded when they differ or are 0xAAAAxxxx):"); for (int v = 0; v < 256; v++) { uint32_t a = __builtin_bswap32(pjit_vdbg[2 * v]), b = __builtin_bswap32(pjit_vdbg[2 * v + 1]); if (a || b) printf(" %X:%08X/%08X", v, (unsigned)a, (unsigned)b); } printf("\n"); } }
    if (code) {                                                                  // exec's memory list: what the guest found (chip RAM, expansion boards, our region tag)
#define RL(x) ((uint32_t)read_BE_word(x) << 16 | read_BE_word((x) + 2))
        uint32_t eb = RL(4); printf("MEMLIST (ExecBase %06X):", (unsigned)eb);
        if (eb >= 0x400 && eb < 0x1000000 && !(eb & 1)) {
            uint32_t n = RL(eb + 0x142);
            for (int k = 0; k < 10 && n && n < 0x10000000 && RL(n); k++, n = RL(n)) printf(" [%06X-%06X attr %04X free %u KB pri %d]", (unsigned)RL(n + 20), (unsigned)RL(n + 24), (unsigned)read_BE_word(n + 14), (unsigned)(RL(n + 28) >> 10), (int)(int8_t)(read_BE_word(n + 8) >> 8));
        }
        printf("\n");
        {   uint32_t q = RL(eb + 0x17A), xb = 0;                                // expansion.library's BoardList: the boards it found (ConfigDev: flags +14, ExpansionRom +16, BoardAddr +32, BoardSize +36)
            for (int k = 0; k < 40 && q && RL(q); k++, q = RL(q)) { uint32_t nm = RL(q + 10); if (nm >= 0x400 && nm < 0x1000000 && RL(nm) == 0x65787061u) { xb = q; break; } }       // "expa"
            printf("EXPANSION.LIBRARY %06X:", (unsigned)xb);
            if (xb) { uint32_t c = RL(xb + 0x3C); for (int k = 0; k < 8 && c && RL(c); k++, c = RL(c)) printf(" [ConfigDev %06X flags %02X type %02X prod %02X mfr %04X addr %06X size %06X]", (unsigned)c, (unsigned)(read_BE_word(c + 14) >> 8), (unsigned)(read_BE_word(c + 16) >> 8), (unsigned)(read_BE_word(c + 16) & 0xFF), (unsigned)read_BE_word(c + 20), (unsigned)RL(c + 32), (unsigned)RL(c + 36)); }
            printf("\n"); }
        printf("AUTOCONFIG after the run, $E80000:"); for (uint32_t a = 0xE80000; a < 0xE80010; a += 2) printf(" %04X", read_BE_word(a));
        printf("; first word of $200000..$9FFFFF by megabyte (value, R = write / read-back works):"); for (uint32_t mb = 2; mb < 10; mb++) { uint32_t ad = mb * 0x100000 + 0x1000; uint16_t o = read_BE_word(ad), t; write_BE_word(ad, 0x5A5A); t = read_BE_word(ad); if (t == 0x5A5A) { write_BE_word(ad, 0xA5A5); t = read_BE_word(ad) == 0xA5A5 ? 1 : 0; } else t = 0; write_BE_word(ad, o); printf(" %04X%s", o, t ? "R" : ""); } printf("\n");
        { uint32_t first = 0, last = 0, hits = 0;                                    // every 64 KB of the Zorro II space (+ the slow RAM window): write / read-back
          for (uint32_t ad = 0x200000; ad < 0xE00000; ad += 0x10000) { uint32_t q = ad + 0x2000; uint16_t o = read_BE_word(q); write_BE_word(q, 0x5A5A); int ok = read_BE_word(q) == 0x5A5A; if (ok) { write_BE_word(q, 0xA5A5); ok = read_BE_word(q) == 0xA5A5; } write_BE_word(q, o); if (ok) { if (!hits++) first = ad; last = ad; } }
          printf("RAM64K: %u of the 64 KB blocks in $200000-$DFFFFF hold RAM (first %06X last %06X)\n", (unsigned)hits, (unsigned)first, (unsigned)last); }
    }
    printf("LASTEXC (oldest first):"); for (int k = 16; k >= 1; k--) { uint32_t e = ((pjit_dlog_pos / 8 - k) & 31) * 2; printf(" %X@%X", (unsigned)pjit_dlog[e], (unsigned)pjit_dlog[e + 1]); } printf("\n");
    printf("ABORTS: %u bus errors, %u address errors delivered to the guest; the last at guest PC %08X, address %08X\n", (unsigned)buffee_abort_bus, (unsigned)buffee_abort_addr, (unsigned)buffee_abort_last_pc, (unsigned)buffee_abort_last_addr);
    printf("STATE: guest SR %04X, irq_level %u, irq_flag %u, flush_req %u, IPL pins %u; Amiga INTENAR %04X INTREQR %04X DMACONR %04X VPOSR %04X\n", (unsigned)__builtin_bswap16(cpu_state.sr), (unsigned)(cpu_state.irq_level & 0xFF), (unsigned)(cpu_state.irq_flag & 0xFF), (unsigned)(cpu_state.flush_req & 0xFF), buffee_ipl(),
           (unsigned)__builtin_bswap16(*(volatile uint16_t *)0xDFF01C), (unsigned)__builtin_bswap16(*(volatile uint16_t *)0xDFF01E), (unsigned)__builtin_bswap16(*(volatile uint16_t *)0xDFF002), (unsigned)__builtin_bswap16(*(volatile uint16_t *)0xDFF004));
    /* the PC sampler (INTC line 3, the PMU counter overflow) must not outlive the payload: the bootloader's IRQ handler does not know line 3 and would take it forever */
    __asm__ volatile("mcr p15, 0, %0, c9, c14, 2" :: "r"(0xFFFFFFFFu));              // PMINTENCLR: no counter raises an interrupt
    __asm__ volatile("mcr p15, 0, %0, c9, c12, 3" :: "r"(0xFFFFFFFFu));              // PMOVSR: clear the overflow flags
    REG32(INTC + 0x8C) = 1u << 3;                                                    // MIR_SET0: line 3 masked again
    CP15DCacheCleanFlush();
    bbb_return(code);
    for (;;) { }
}

void bbb_fault_dump(const uint32_t *regs)
{
    for (int i = 2; i < 13; i += 4) {
        for (int k = i; k < i + 4 && k < 13; k++) printf(" r%d=%08X", k, (unsigned)regs[k]);
        printf("\n");
    }
    const uint32_t *pc = (const uint32_t *)(regs[1] - 8);
    for (int k = -6; k <= 2; k++) printf("[FAULT] %08X: %08X%s\n", (unsigned)(pc + k), (unsigned)pc[k], k == 0 ? "  <--" : "");
}
void bbb_fault_exit(void) { bbb_exit(1); }
void qemu_exit(void) { bbb_exit(0); }
__attribute__((weak)) void pjit_test_exit(unsigned d0) { (void)d0; }
