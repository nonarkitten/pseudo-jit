/// @file buffee.h
/// @brief Buffee platform glue (see buffee.c) on top of the BeagleBone API in bbb.h.
#ifndef BUFFEE_H
#define BUFFEE_H
#include <stdint.h>
#define BUFFEE_DRAM_GUEST   0x08000000u    // guest address of the DRAM (physical 0x80000000), 128 MB
#define BUFFEE_DRAM_PHYS    0x80000000u
#define BUFFEE_CACHE_TOP    0x9F400000u    // pjit_cache_init(): the 4 MB slot cache below it (0x9F000000), its tags under that
#define BUFFEE_IMAGE_BASE   0x9F600000u    // the payload (platform/buffee/pjit_dram.ld); also its entry point (the reset vector)
extern volatile uint32_t buffee_reset_events, buffee_halt_events, buffee_ipl_events, buffee_reset_pending;
unsigned buffee_ipl(void);                 // IPL2..0 as a level 0-7
void buffee_fc_init(void);                 // FC1 / FC2 GPIOs (LCD_DATA3 / 4): FC = 101 (supervisor data); FC0 is tied high on the board (bbb_init calls this)
unsigned buffee_fc_get(void);              // the FC2 FC1 FC0 value being driven
void buffee_fc_supervisor(int on);         // FC2 = the guest's S bit (the generated SR writes and the exception entry do this themselves under PJIT_FC)
uint8_t buffee_iack(unsigned level, int supervisor);   // one interrupt-acknowledge cycle (FC = 111, A3-A1 = level); returns the byte on D7-D0; FC2 restored to `supervisor`
#define BUFFEE_MAPROM_PHYS  0x9FE00000u    // the 1 MB behind guest $F00000-$FFFFFF (the ROM in its upper half)
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
#define BUFFEE_MAILBOX      ((volatile uint32_t *)0x9F5FFFF0u)   // bootloader -> payload: 1 = this is a restart after an Amiga reset (the bootloader recalibrated the bus)
#endif
