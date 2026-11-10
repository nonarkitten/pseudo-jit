/// @file blk.h
/// @brief The block runtime's state (pjit/blk.s): the block cache, the translator's scratch, and the parked guest registers while the router runs.
///        Design: docs/design/BLOCK_RUNTIME.md. Built only with BLK=1 (pjit/blk.s replaces pjit/runtime_dr.s).
#ifndef PJIT_BLK_H
#define PJIT_BLK_H
#include <stdint.h>
#include "bcache.h"

#define BLK_MAXSTUB 24
#define BLK_MAXSITE 48
#define BLK_MAXITEM 192
#define BLK_MAXBK 128

typedef struct {
    bcache_t bc;                    /* +0   (must be first: the asm passes the blk_t as the bcache_t) */
    uint32_t cap;                   /* +88  bytes reserved per translation (the host cap), at most one segment, at least 2048 */
    uint32_t src_cap;               /* +92  guest bytes a block may cover */
    uint32_t stack_top;             /* +96  the private stack the router and translator run on */
    uint32_t park[8];               /* +100 A0-A7 while the router runs */
    uint32_t stub_in;               /* +132 the stub that led to the router (0 none, 1 interpret-one; else its address, | 2 for a call stub) */
    uint32_t target;                /*      the guest address the router was entered with */
    uint32_t stub_ep;               /*      the epoch (0..31) of the stub's segment when the router was entered (0xFF: not a stub in the ring): translating the target can reclaim the segment the stub lives in,
                                           and bc_link would then patch (and register for unlinking) whatever code lies there now; the router links only if the epoch is still the same */
    uint32_t nstub, nsite;          /* +136, +140 */
    uint32_t stub_tgt[BLK_MAXSTUB]; /* guest target of each exit stub */
    uint32_t stub_kind[BLK_MAXSTUB];/* 0 jump stub, 1 interpret-one stub, 2 call stub (then, once emitted, the stub's address) */
    uint32_t stub_site[BLK_MAXSTUB];/* the first branch site that goes to each stub (its fourth word: linking aims that branch at the target too) */
    uint32_t site_at[BLK_MAXSITE];  /* address of each branch word that goes to a stub ... */
    uint32_t site_stub[BLK_MAXSITE];/* ... and its stub's index */
    uint32_t mirror;                /* forwarding: for each of r0-r3 (a byte each) which D register slot it mirrors: n << 4 | kind, 0 = none */
    uint32_t foldmask;              /* the instruction being emitted: its leader's offset-folding mask (bits 29:28 as in pjit_bv) */
    uint32_t fwdst;                 /* forwarding, per instruction: bit 0 = a branch / barrier was seen (the rest of the instruction is copied), bit 1 = it has a branch */
    uint32_t item_end;              /* the translator's pass 1 result: the end of items[] (an address) */
    uint32_t endkind;               /* how pass 1 ended: 0 a cap (fall-through stub), 1 an instruction the interpreter takes, 2 a terminator */
    uint32_t schedstart;            /* the scheduler's region: where the plain instructions of the current run start in the ring (0 none) */
    uint32_t n_sched_try, n_sched_ok; /* statistics: regions given to the scheduler, and reordered by it */
    uint32_t items[BLK_MAXITEM];    /* op | size << 16 | live-out flag groups << 24, one per instruction of the block being translated */
    uint32_t n_translated, n_instr, n_exits; /* statistics */
    uint32_t n_fwd_mov, n_fwd_ext, n_fwd_del, n_slot_ld, n_slot_st; /* forwarding, counted at translation: moves, extends, deleted loads; slot loads and stores seen */
    uint32_t n_route, n_one;        /* router entries; interpret-one entries */
    uint32_t n_fused;               /* statistics: compare + Bcc pairs translated with the compare in natural polarity */
    uint32_t tier_builds, tier_rebuilds, tier_pause; /* the tiered build's thrash guard: translations / of targets that had a block before in this epoch of 32, promotions still to be refused */
    uint32_t prof_last;             /* BLK_PROF=1: the cycle counter at the last mark; prof[] = cycles per translator phase (0 reserve, 1 decode, 2 liveness, 3 emit, 4 schedule, 5 stubs, 6 commit, 7 cache maintenance, 8 router) */
    uint32_t prof[16];
    uint32_t bk_n;                  /* the registered backward links (stub addresses): undone when an interrupt is latched */
    uint32_t bk[BLK_MAXBK];
    uint32_t sched[2048];           /* the scheduler's workspace (pjit/blksched.s: blk_sched_size bytes); last: its offset does not fit an immediate */
} blk_t;

#define BLK_FAST_BITS 10
extern blk_t pjit_blk;
extern uint32_t blk_fast[2 << BLK_FAST_BITS];   /* the fast table: {guest, block} per index, both byte-swapped (read by big-endian code) */
extern void blk_hook(bcache_t *);

/// Boot-time setup (after emit_opcode_table / the table setup). ring_bytes, seg_bytes, sets are powers of two; stack: at least 1 KB.
static inline void blk_init(void *ring, uint32_t ring_bytes, uint32_t seg_bytes, void *tags, uint32_t sets, void *links, void *epochs,
                            void *stack, uint32_t stack_bytes, uint32_t cap, uint32_t src_cap)
{
    extern void pjit_blk_exit(void), pjit_blk_call_exit(void);
    blk_t *b = &pjit_blk;
    bc_init(&b->bc, ring, ring_bytes, seg_bytes, tags, sets, links, epochs);
    b->bc.h_exit = (uint32_t)pjit_blk_exit; b->bc.h_call = (uint32_t)pjit_blk_call_exit; b->bc.hook = (uint32_t)blk_hook;
    b->cap = cap; b->src_cap = src_cap; b->stack_top = (uint32_t)stack + stack_bytes;
    b->n_translated = b->n_instr = b->n_exits = b->n_route = b->n_one = 0; b->tier_builds = b->tier_rebuilds = b->tier_pause = 0; b->n_fused = 0;
}
#endif
