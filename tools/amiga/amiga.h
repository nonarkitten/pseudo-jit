/* amiga.h -- a small, deterministic Amiga 500+ (ECS, PAL) device model for differential CPU tests: the same code feeds the golden 68000 core (host)
 * and PJIT (QEMU), so the two see identical hardware as long as they execute identical instruction streams. Time is the number of 68000 instructions
 * completed (AM_CPI 7.09 MHz clocks each); nothing here depends on the host. Freestanding C: no libc. */
#ifndef AMIGA_H
#define AMIGA_H
#include <stdint.h>

#define AM_CPI        8u                    /* 7.09 MHz clocks per instruction (any fixed number: the model is its own clock) */
#define AM_FRAME_CYC  141648u               /* 312 lines x 227 colour clocks x 2 */
#define AM_LINE_CYC   454u

typedef struct {
    uint8_t  pra, prb, ddra, ddrb, sdr, icr_data, icr_mask, cra, crb;
    uint16_t ta, tb, ta_latch, tb_latch;
    uint32_t tod, tod_latch, tod_alarm; int tod_latched, tod_running;
    uint64_t t_sync;                        /* E-clock ticks (CPU clocks / 10) up to which the timers have been advanced */
    uint64_t tod_t0, tod_period;
} am_cia_t;

typedef struct {
    uint8_t *rom;                           /* 512 KB, big-endian, as the ROM file */
    uint16_t intena, intreq, dmacon, adkcon;
    am_cia_t a, b;
    uint64_t next_vblank;                   /* cycle of the next vertical blank */
    uint64_t hash;                          /* running hash of every hardware access (address, size, value read / written, time) */
    uint64_t n_access;
    uint64_t icount;                        /* instructions completed, as last told by am_events() / am_set_time() */
    uint32_t last_ipl;
} am_t;

void     am_init(am_t *m, uint8_t *rom);
int      am_overlay(const am_t *m);         /* ROM mapped over chip RAM at 0 (reads) */
uint32_t am_read(am_t *m, uint32_t addr, int size, uint64_t icount);       /* hardware or unmapped address: the value read (numeric, big-endian semantics) */
void     am_write(am_t *m, uint32_t addr, int size, uint32_t val, uint64_t icount);
int      am_is_hw(uint32_t addr);           /* 1 for $A00000-$BFFFFF, $C00000-$DFFFFF, $E00000-$EFFFFF, and $200000-$9FFFFF (unmapped) */
uint64_t am_deadline(am_t *m);              /* the instruction count of the next event (vertical blank, CIA underflow): events up to it are due */
void     am_events(am_t *m, uint64_t icount);   /* apply every event due at or before `icount` completed instructions */
int      am_ipl(am_t *m);                   /* the interrupt level 0-7 the CPU sees */
#endif
