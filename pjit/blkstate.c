/// @file blkstate.c
/// @brief The block runtime's one global (see blk.h).
#include "blk.h"
#include "pjit.h"
blk_t pjit_blk;
uint32_t blk_fast[2 << BLK_FAST_BITS] __attribute__((aligned(64)));
/// The tiered build's counters (pjit/runtime_dr.s pjit_goto, pjit/tier.s): per index {last target seen, count}, big-endian words, touched only by big-endian code and tier.s (which byte-swaps).
uint32_t pjit_tier_cnt[2 << 10] __attribute__((aligned(64)));

// The default configuration, set up by emit_opcode_table() in a BLK build so the existing tests run through blocks unchanged:
// a 256 KB ring in 8 segments of 32 KB, 4-way x 1024 sets of tags.
#ifndef BLK_RING_KB
#define BLK_RING_KB 256                          /* the block arena in KB (8 segments); -DBLK_RING_KB=n for the arena-size sweeps */
#endif
/* a block reserves up to its cap at once and must fit well inside a segment: cap <= segment / 2 (8 KB for the default 256 KB / 8 segments) */
#define BLK_CAP_BYTES ((BLK_RING_KB * 1024 / 8 / 2) < 8192 ? (BLK_RING_KB * 1024 / 8 / 2) : 8192)
static uint8_t blk_ring[BLK_RING_KB * 1024] __attribute__((aligned(4096)));
static uint32_t blk_tags[1024 * 8] __attribute__((aligned(64)));
static uint32_t blk_links[8 * 512] __attribute__((aligned(64)));   // 8 lists of 2 KB (seg 32 KB: 511 records each)
static uint8_t blk_epochs[8];
static uint8_t blk_stack[4096] __attribute__((aligned(16)));
void blk_boot(void)
{
    blk_init(blk_ring, sizeof blk_ring, sizeof blk_ring / 8, blk_tags, 1024, blk_links, blk_epochs, blk_stack, sizeof blk_stack, BLK_CAP_BYTES, 256);
    extern void pjit_blk_fast_clear(void);
    pjit_blk_fast_clear();                       // (empty entries are {an odd key, the slow path}, not zeros: see blk.s)
}
/// The epoch (0..31) of the ring segment the stub (a possibly tagged address) lies in, 0xFF if it is not in the ring: see blk_t.stub_ep.
uint32_t blk_stub_epoch(uint32_t stub)
{
    uint32_t off = (stub & ~3u) - pjit_blk.bc.ring;
    if (!pjit_blk.bc.ring || off > pjit_blk.bc.mask) return 0xFF;
    return ((volatile uint8_t *)pjit_blk.bc.epochs)[off >> pjit_blk.bc.seg_shift] & 31u;
}
#ifdef PJIT_BK_DEBUG
void blk_bk_check(uint32_t stub)                // called by pjit_bk_unlink for each registered stub
{
    uint32_t ring = pjit_blk.bc.ring, mask = pjit_blk.bc.mask;
    volatile uint32_t *w = (volatile uint32_t *)stub;
    int in_ring = stub - ring <= mask;
    uint32_t w0 = in_ring ? w[0] : 0, site = in_ring ? w[3] : 0;
    int ok = in_ring && ((w0 >> 24) == 0xEA || w0 == 0xE1A0E00Fu || (w0 >> 24) == 0xEB) && (site == 0 || site - ring <= mask);
    if (!ok) {
        printf("BKCHECK bad registered stub %08X (ring %08X +%X, in ring %d): words %08X %08X %08X %08X; bk_n %u; registry:", stub, ring, mask + 1, in_ring, in_ring ? w[0] : 0, in_ring ? w[1] : 0, in_ring ? w[2] : 0, in_ring ? w[3] : 0, pjit_blk.bk_n);
        for (uint32_t i = 0; i < pjit_blk.bk_n && i < 16; i++) printf(" %08X", pjit_blk.bk[i]);
        printf("\n");
        for (;;) ;
    }
}
#endif
void blk_flush(void) { extern void bc_flush(bcache_t *); if (pjit_blk.bc.ring) bc_flush(&pjit_blk.bc); pjit_blk.bk_n = 0; }   // (the registered backward stubs lie in the ring that was just emptied: pjit_bk_unlink must not walk them)   // (pjit_cache_init flushes before blk_boot has run)
