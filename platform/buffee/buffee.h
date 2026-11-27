/// @file buffee.h
/// @brief Buffee platform glue (see buffee.c) on top of the BeagleBone API in bbb.h.
#ifndef BUFFEE_H
#define BUFFEE_H
/* Where the platform keeps itself in the ARM's virtual address space. The guest's 24 address lines repeat the 16 MB space over the whole 4 GB (buffee.c), and
   the platform's own megabytes are the only places where that repetition breaks: a guest pointer whose top byte is one of $1B-$1F (tag bits no known 68000
   software uses: the Mac's handle flags are bits 31-29, address arithmetic gives $01, $FE and $FF) reaches them instead of the 16 MB space. Top down:
     0x1EF00000-0x1FFFFFFF   the top 17 MB of the DDR (physical 0x9EF00000-0x9FFFFFFF: the slot cache's tags, the slot cache, bootinfo, the PJIT image, MapROM),
                             at a fixed distance: virtual = physical - BUFFEE_DDR_SHIFT; an image whose bss lies lower in the DDR (bench_dram.ld) is mapped down to it
     0x1CA00000-0x1CFFFFFF   the write-detection tables, in the DDR right behind the guest's RAM regions
     0x1C900000              the custom chips ($DFxxxx), writable (the guest's own mapping is read-only while writes are traced)
     0x1C000000-0x1C8FFFFF   the SoC's peripherals, one megabyte each, in the order of BUFFEE_IO_SECTIONS
     0x1B000000-0x1BFFFFFF   the Amiga's 16 MB through the GPMC window, domain 0: how an emulated access reaches the bus */
#define BUFFEE_VA_LOW       0x1B000000u    // the lowest virtual address the platform uses for itself
#define BUFFEE_BUS_ALIAS    0x1B000000
#define BUFFEE_IO_VA        0x1C000000
#define BUFFEE_CHIP_ALIAS   0x1C900000u
#define BUFFEE_TRK_VA       0x1CA00000u
#define BUFFEE_TRK_MB       6u
#define BUFFEE_DDR_SHIFT    0x80000000
#define BUFFEE_DDR_VA       0x1EF00000u    // the lowest megabyte of the DDR mapping (a lower bss extends it: __pjit_ddr_low)
/* The peripherals' megabytes (physical address >> 20) in the order they are mapped from BUFFEE_IO_VA: boot ROM and OCMC RAM 0x400-0x403, L4_WKUP 0x44E (clocks,
   control module, UART0, GPIO0, I2C0, WDT1), L4_PER 0x480-0x482 (DMTimers, GPIO1-3, INTC), the GPMC's registers 0x500. */
#define BUFFEE_IO_SECTIONS  0x400, 0x401, 0x402, 0x403, 0x44E, 0x480, 0x481, 0x482, 0x500
#define BUFFEE_IO_SLOT(s)   ((s) < 0x404 ? (s) - 0x400 : (s) == 0x44E ? 4 : (s) < 0x483 ? (s) - 0x480 + 5 : 8)
#define BUFFEE_IO(pa)       (BUFFEE_IO_VA + (BUFFEE_IO_SLOT((pa) >> 20) << 20) + ((pa) & 0xFFFFF))     // a peripheral register's virtual address
#define BUFFEE_DDR(pa)      ((pa) - BUFFEE_DDR_SHIFT)                                                  // a top-of-DDR physical address as a pointer
#define BUFFEE_BRIDGE_DDR   0x1D0          // boot.S's bridge table maps the DDR from this megabyte up (every layout's lowest: bench_dram.ld's bss is at 0x1D7)
/* (the constants boot.S uses are written without the u suffix, which the assembler does not take) */
#ifndef __ASSEMBLER__
#include <stdint.h>
#define BUFFEE_DRAM_GUEST   0x08000000u    // guest address of the DRAM (physical 0x80000000), 128 MB
#define BUFFEE_DRAM_PHYS    0x80000000u
#define BUFFEE_CACHE_TOP    BUFFEE_DDR(0x9F400000u)   // pjit_cache_init(): the 4 MB slot cache below it (physical 0x9F000000), its tags under that
#define BUFFEE_IMAGE_PHYS   0x9F600000u    // where the bootloader puts the payload and calls it (the reset vector); linked at BUFFEE_DDR() of it (platform/buffee/pjit_dram.ld)
#define BUFFEE_BOOTINFO     ((const volatile uint8_t *)BUFFEE_DDR(0x9F5FFF00u))   // bootinfo_t (bootloader/inc/config.h)
uint32_t buffee_pa(const volatile void *va);   // the physical address of one of the platform's own virtual addresses (the DDR mapping, the write-detection tables)
extern volatile uint32_t buffee_reset_events, buffee_halt_events, buffee_ipl_events, buffee_reset_pending;
unsigned buffee_ipl(void);                 // IPL2..0 as a level 0-7
void buffee_pulse_reset(void);             // the 68000's RESET instruction: the Amiga's RESET line low for 124 of its clocks; the edge is not taken for the Amiga's own reset
void buffee_fc_init(void);                 // FC1 / FC2 GPIOs (LCD_DATA3 / 4): FC = 101 (supervisor data); FC0 is tied high on the board (bbb_init calls this)
unsigned buffee_fc_get(void);              // the FC2 FC1 FC0 value being driven
void buffee_fc_supervisor(int on);         // FC2 = the guest's S bit (the generated SR writes and the exception entry do this themselves under PJIT_FC)
uint8_t buffee_iack(unsigned level, int supervisor);   // one interrupt-acknowledge cycle (FC = 111, A3-A1 = level); returns the byte on D7-D0; FC2 restored to `supervisor`
#define BUFFEE_MAPROM_PHYS  0x9FE00000u    // the 1 MB behind guest $F00000-$FFFFFF (the ROM in its upper half)
#define BUFFEE_MAPROM       BUFFEE_DDR(BUFFEE_MAPROM_PHYS)   // the same megabyte, to read and write
#define BUFFEE_STAGE_PHYS   0x9EE00000u    // 1 MB at the top of the guest's pool, never given to a RAM region: an image sent through the control page (control.c) waits here
#define BUFFEE_STAGE_VA     BUFFEE_DDR(BUFFEE_STAGE_PHYS)
unsigned buffee_cpu_mhz(void);             // the ARM's clock as the MPU PLL is set now (control_board.c)
void buffee_clock_changed(void);           // called after the ARM's clock changed (the pace governor's constant follows it); does nothing unless a payload has one
void buffee_smc_report(void);             // write-detection statistics (platform/buffee/buffee.c)
uint32_t buffee_maprom(uint32_t size);     // copy the Kickstart through the window and map it; returns the sum of its words
void buffee_sampler_start(uint32_t cycles);   // print the guest PC every `cycles` host cycles (an interrupt on the ARM performance monitor)
void buffee_trace_custom(void);              // log every write to $DFF000-$DFF1FF in buffee_wlog (each write faults and is emulated: slow)
extern uint16_t buffee_shadow[];
extern uint32_t buffee_wlog[], buffee_wlog_pos;
extern uint32_t sample_stop, sample_verbose, buffee_last_pc, buffee_last_r4; extern volatile uint32_t buffee_sample_stopped;   // the sampler ends pjit_run() after sample_stop samples (0 = never); sample_verbose prints each one
#define BUFFEE_REROLL_EXIT(k) (0x110u + (k))  // the bootloader re-initialises a Buffee-side piece (k: bit 0 PRU, bit 1 GreenPAK image, bit 2 GPMC), recalibrates and calls the payload again (tests/buffee/buffee_reroll.c)
#define BUFFEE_BREAK_EXIT   0x1B0u         // ESC three times on the UART while the payload runs: buffee_irq_c hands the CPU straight back to the bootloader's menu (not a restart code)
#define BUFFEE_RESET_EXIT   0x100u         // pjit_run()'s result when the Amiga's RESET line fell (pjit_platform_irq posted level 8)
uint16_t buffee_peek16(uint32_t a);       // a guest word for a report, read without faulting (an odd or unmapped address reads $EEEE)
#define BUFFEE_MAILBOX      ((volatile uint32_t *)BUFFEE_DDR(0x9F5FFFF0u))   // bootloader -> payload: 1 = this is a restart after an Amiga reset (the bootloader recalibrated the bus)
#endif
#endif
