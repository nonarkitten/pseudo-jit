/// @file bcache.h
/// @brief The block cache: tags over a bump-allocated code ring cut into segments, with chaining links (pjit/bcache.s). Design: docs/design/BLOCK_RUNTIME.md.
///
///   * A block is just its code (no header). The ring is `ring_bytes` (a power of two) split into segments of `seg_bytes` (a power of two, at least the
///     largest block); a block never straddles a segment. `cursor` is a monotonic 32-bit byte count (ring offset = cursor & mask).
///   * Tags: sets of 4 ways, 8 bytes per way {guest address, seq}; seq is the cursor value the block was written at. A tag is live iff `hw - seq <= mask`,
///     where hw is the last byte of the segment being written. Entering a segment (the cursor moves into it) kills the whole old content of that slot at
///     once; nothing is ever cleared, nothing walks the blocks. A set's victim is its oldest way (dead ways are the oldest): FIFO, no LRU state.
///   * Links: a stub is a patchable word in a block (type 0: forward jump, restored to `mov lr,pc`; type 1: static call, `bl h_call`; type 2: backward stub,
///     `bl h_exit`; every linked stub is a plain `b`). bc_link patches it to a branch to the target block and files a 4-byte record in the target segment's list; when that segment is
///     entered every stub that still points into it is put back (a record whose stub segment has been recycled since, tracked by a per-segment epoch, is
///     skipped). A list holds LMAX records; when full the stub simply stays unlinked. Invariant L: no live stub is linked to a dead block.
///   * bc_flush kills every block at once by moving hw a whole ring ahead (O(segments), independent of the cache contents).
///   * The 32-bit horizon: when the cursor would pass BC_HORIZON the tags are cleared and everything restarts at 2 * ring_bytes.
///   * hook: called (r0 = bc) each time a segment is entered, after the stubs into it were restored (the runtime clears its shadow return stack there).
#ifndef PJIT_BCACHE_H
#define PJIT_BCACHE_H
#include <stdint.h>

#define BC_WAYS     4
#define BC_HORIZON  0xE0000000u
/* A segment's link list is a count and its records; it is max(256, seg_bytes / 16) bytes (one record per 16 bytes of segment, at least 63 records). */
static inline uint32_t bc_list_shift(uint32_t seg_bytes) { uint32_t sh = 8; while ((1u << (sh + 4)) < seg_bytes) sh++; return sh; }
#define BC_LMAX     63u     /* the records of the smallest list (a 512-byte segment, as in tests/qemu/smoke_bcache.c) */

typedef struct {
    uint32_t cursor;        /* +0  next block's seq (the monotonic write position) */
    uint32_t hw;            /* +4  last byte of the segment being written (as a cursor value) */
    uint32_t ring;          /* +8  address of the ring */
    uint32_t mask;          /* +12 ring_bytes - 1 */
    uint32_t tags;          /* +16 address of the tag array: (set_mask + 1) sets x 4 ways x {guest, seq} */
    uint32_t set_mask;      /* +20 number of sets - 1 */
    uint32_t seg_shift;     /* +24 log2(seg_bytes) */
    uint32_t links;         /* +28 the link lists: nseg x (1 << list_shift) bytes */
    uint32_t epochs;        /* +32 nseg bytes: how often each segment slot was entered */
    uint32_t h_exit;        /* +36 where an unlinked backward stub (type 2) calls */
    uint32_t h_call;        /* +40 where an unlinked call stub (type 1) calls */
    uint32_t hook;          /* +44 0 or a function called on every segment entry */
    uint32_t list_shift;    /* +48 log2 of a link list's size in bytes (bc_list_shift) */
    uint32_t lmax;          /* +52 the records a list holds: (1 << (list_shift - 2)) - 1 */
    uint32_t save[10];      /* +56 r4-r11 and lr while a routine runs (no stack in runtime asm) */
} bcache_t;                 /* 96 bytes */

/// Boot-time setup. Sizes are powers of two; seg_bytes <= ring_bytes; ring 16-byte aligned; tags needs sets * 32 bytes, links nseg << bc_list_shift(seg_bytes), epochs nseg.
static inline void bc_init(bcache_t *bc, void *ring, uint32_t ring_bytes, uint32_t seg_bytes, void *tags, uint32_t sets, void *links, void *epochs)
{
    uint32_t nseg = ring_bytes / seg_bytes, sh = 0;
    while ((1u << sh) < seg_bytes) sh++;
    bc->ring = (uint32_t)ring; bc->mask = ring_bytes - 1; bc->tags = (uint32_t)tags; bc->set_mask = sets - 1; bc->seg_shift = sh;
    bc->links = (uint32_t)links; bc->epochs = (uint32_t)epochs; bc->h_exit = bc->h_call = bc->hook = 0;
    for (uint32_t i = 0; i < sets * 8; i++) ((uint32_t *)tags)[i] = 0;
    bc->list_shift = bc_list_shift(seg_bytes); bc->lmax = (1u << (bc->list_shift - 2)) - 1;
    for (uint32_t i = 0; i < (nseg << bc->list_shift) / 4; i++) ((uint32_t *)links)[i] = 0;
    for (uint32_t i = 0; i < nseg; i++) ((uint8_t *)epochs)[i] = 0;
    bc->cursor = 2 * ring_bytes; bc->hw = bc->cursor + seg_bytes - 1;
    ((uint8_t *)epochs)[(bc->cursor & bc->mask) >> sh] = 1;
}

/// The code address of the live block for `guest`, or 0.
uint32_t bc_lookup(bcache_t *bc, uint32_t guest);
/// Start of `max` bytes (a multiple of 16, at most one segment) for a new block at the cursor, or 0 if max exceeds a segment. If the bytes do not fit in
/// the rest of the segment the cursor moves into the next one (turning it over: its old blocks die and the stubs into it are restored). Nothing is
/// published until bc_commit; giving up is not calling bc_commit (the cursor stays).
uint32_t bc_reserve(bcache_t *bc, uint32_t max);
/// Publish the block written into the last reservation, `used` bytes long: returns its code address.
uint32_t bc_commit(bcache_t *bc, uint32_t guest, uint32_t used);
/// Link `stub` (the address of the patchable word, in a live block) of `type` to `target` (the code address of a live block). Returns 1, or 0 if the
/// target segment's list is full (the stub is left as it was).
uint32_t bc_link(bcache_t *bc, uint32_t stub, uint32_t type, uint32_t target);
/// Forget every block at once.
void bc_flush(bcache_t *bc);

#endif
