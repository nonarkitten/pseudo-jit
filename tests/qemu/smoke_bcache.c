/// @file smoke_bcache.c
/// @brief The block cache (pjit/bcache.s): tags over a segmented bump-allocated ring, with chaining links. Two properties matter:
///        (1) a lookup NEVER returns a block whose bytes were overwritten (by a later block, the garbage of an abandoned reservation, a segment turnover);
///        (2) invariant L: no stub of a live block is linked to a dead block, and every stub is either exactly its unlinked word or a branch to a live block's
///            start -- a turnover must also never write into a stub whose own segment has been recycled since (it would corrupt the new block).
///        Every block carries a stamp over its body (after its stub words); a model of the cache (each block's seq, liveness = hw - seq <= mask) is checked
///        after every operation of a soak mixing commits, abandoned reservations, links, flushes, wraps and set conflicts.
#include <stdint.h>
#include "pjit_libc.h"
#include "bcache.h"

#define RING 4096u
#define SEG  512u
#define NSEG (RING / SEG)
#define SETS 32u
static uint8_t  ring[RING] __attribute__((aligned(4096)));
static uint32_t tags[SETS * 8] __attribute__((aligned(64)));
static uint32_t links[NSEG * 64] __attribute__((aligned(64)));
static uint8_t  epochs[NSEG];
static bcache_t bc;
static int pass, fail;
static uint32_t hook_calls;
static void hook(bcache_t *b) { (void)b; hook_calls++; }
static void dummy_exit(void) { }
static void dummy_call(void) { }

static void check(const char *what, uint32_t got, uint32_t want)
{
    if (got == want) pass++;
    else { printf("[FAIL] %-60s got 0x%08X want 0x%08X\n", what, got, want); fail++; }
}

// ---- the model: one record per committed block, indexed by its start / 16 ----
typedef struct { uint32_t code, guest, serial, seq, used, k; uint8_t type[4]; } meta_t;
static meta_t meta[RING / 16];
static uint32_t serial_no;
static void setup(void)
{
    bc_init(&bc, ring, RING, SEG, tags, SETS, links, epochs);
    bc.h_exit = (uint32_t)dummy_exit; bc.h_call = (uint32_t)dummy_call; bc.hook = (uint32_t)hook;
    for (unsigned i = 0; i < RING / 16; i++) meta[i].serial = 0;
}

static uint32_t rng = 0x12345678;
static uint32_t rnd(void) { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }

static uint32_t stamp(uint32_t guest, uint32_t serial, uint32_t i) { return (guest * 2654435761u) ^ (serial * 40503u) ^ (i * 0x9E3779B1u); }
static uint32_t unlinked(uint32_t stub, int type)
{
    if (type == 0) return 0xE1A0E00Fu;                                                  // mov lr, pc
    uint32_t h = type == 1 ? (uint32_t)dummy_call : (uint32_t)dummy_exit;
    return 0xEB000000u | (((h - stub - 8) >> 2) & 0xFFFFFFu);                           // bl helper
}
static uint32_t branch(uint32_t stub, int type, uint32_t target)
{
    return 0xEA000000u | (((target - stub - 8) >> 2) & 0xFFFFFFu);
}
static meta_t *meta_at(uint32_t code) { return &meta[(code - (uint32_t)ring) >> 4]; }
static int live(const meta_t *m) { return m->serial && (bc.hw - m->seq) <= bc.mask; }
static int intact(const meta_t *m)
{
    for (uint32_t i = m->k; i < m->used / 4; i++) if (((uint32_t *)m->code)[i] != stamp(m->guest, m->serial, i)) return 0;
    return 1;
}

/// Reserve `max`, trash all of it, then (if `commit`) write a block of `used` bytes: k stub words of the given types, then a stamp; commit it.
static uint32_t translate_k(uint32_t guest, uint32_t used, uint32_t max, int commit, uint32_t k, const uint8_t *types)
{
    uint32_t p = bc_reserve(&bc, max);
    if (p < (uint32_t)ring || p + max > (uint32_t)ring + RING || ((p - (uint32_t)ring) & (SEG - 1)) + max > SEG) {
        printf("[FAIL] reservation 0x%08X + %u outside the ring or straddling a segment\n", p, max); fail++; return 0; }
    for (uint32_t i = 0; i < max / 4; i++) ((uint32_t *)p)[i] = 0xDEAD0000u + i;
    if (!commit) return 0;
    uint32_t serial = ++serial_no, seq = bc.cursor;
    for (uint32_t i = 0; i < k; i++) ((uint32_t *)p)[i] = unlinked(p + 4 * i, types[i]);
    for (uint32_t i = k; i < used / 4; i++) ((uint32_t *)p)[i] = stamp(guest, serial, i);
    uint32_t code = bc_commit(&bc, guest, used);
    if (code != p) { printf("[FAIL] commit returned 0x%08X, block starts at 0x%08X\n", code, p); fail++; }
    meta_t *m = meta_at(p);
    m->code = p; m->guest = guest; m->serial = serial; m->seq = seq; m->used = used; m->k = k;
    for (uint32_t i = 0; i < k; i++) m->type[i] = types[i];
    return code;
}
static uint32_t translate(uint32_t guest, uint32_t used, uint32_t max, int commit) { return translate_k(guest, used, max, commit, 0, 0); }

/// A hit must be a live, intact block of this guest.
static int verify(uint32_t guest, uint32_t code, const char *where)
{
    int ok = code >= (uint32_t)ring && code < (uint32_t)ring + RING;
    meta_t *m = ok ? meta_at(code) : 0;
    ok = ok && m->code == code && m->guest == guest && live(m) && intact(m);
    if (!ok) { printf("[FAIL] %s: guest 0x%08X returned a block that is not its own / not live / overwritten (0x%08X)\n", where, guest, code); fail++; }
    else pass++;
    return ok;
}

/// Invariant L, and the integrity of every live block, from the model.
static int inv(const char *where)
{
    int ok = 1;
    for (uint32_t s = 0; s < RING / 16; s++) {
        meta_t *m = &meta[s];
        if (!live(m)) continue;
        if (!intact(m)) { printf("[FAIL] %s: live block 0x%08X (guest 0x%08X) was overwritten\n", where, m->code, m->guest); fail++; ok = 0; m->serial = 0; continue; }
        for (uint32_t j = 0; j < m->k; j++) {
            uint32_t stub = m->code + 4 * j, w = *(uint32_t *)stub;
            if (w == unlinked(stub, m->type[j])) continue;
            uint32_t op = w >> 24, tgt = stub + 8 + (uint32_t)((int32_t)(w << 8) >> 6);
            meta_t *t = (tgt >= (uint32_t)ring && tgt < (uint32_t)ring + RING) ? meta_at(tgt) : 0;
            if (op != 0xEAu || !t || t->code != tgt || !live(t) || !intact(t)) {
                printf("[FAIL] %s: stub %u of block 0x%08X (type %u) is %08X: %s\n", where, j, m->code, m->type[j], w,
                       (op == 0xEA) ? "linked to a dead block" : "corrupt"); fail++; ok = 0; *(uint32_t *)stub = unlinked(stub, m->type[j]); }
        }
    }
    if (ok) pass++;
    return ok;
}

#define NG 300
static uint32_t G(int k) { return 0x00010000u + 2u * (uint32_t)k * 6u; }       // spreads over the sets, all even

int main(void)
{
    printf("=== PJIT smoke test: the block cache (segmented ring, tags, links) ===\n");
    check("BC_HORIZON agrees with bcache.s", BC_HORIZON, 0xE0000000u);
    setup();

    // ---- empty
    check("empty cache: lookup misses", bc_lookup(&bc, 0x1000), 0);
    check("empty cache: guest 0 misses (a zeroed tag is dead)", bc_lookup(&bc, 0), 0);

    // ---- the newest blocks are found: 32 blocks of 64 bytes in 32 different sets
    uint32_t code[SETS];
    for (uint32_t k = 0; k < SETS; k++) code[k] = translate(0x2000 + 2 * k, 64, 64, 1);
    int found = 0;
    for (uint32_t k = 0; k < SETS; k++) { uint32_t r = bc_lookup(&bc, 0x2000 + 2 * k); if (r == code[k] && verify(0x2000 + 2 * k, r, "recent")) found++; }
    check("32 blocks in 32 sets (2 KB of 4 KB): all found", found, SETS);
    check("a block's code address is its place in the ring (no header)", code[0], (uint32_t)ring + ((2 * RING) & (RING - 1)));
    check("block 1 follows block 0 by its rounded length", code[1] - code[0], 64);

    // ---- a set keeps its newest 4 ways: 5 guests in one set, the oldest goes
    setup();
    uint32_t gs[6], cs[6];
    for (int j = 0; j < 5; j++) { gs[j] = 0x4000 + 2 * SETS * j; cs[j] = translate(gs[j], 48, 48, 1); }
    check("5 guests in one set: the oldest is evicted", bc_lookup(&bc, gs[0]), 0);
    for (int j = 1; j < 5; j++) check("... the newer four are found", bc_lookup(&bc, gs[j]), cs[j]);
    uint32_t again = translate(gs[2], 48, 48, 1);
    check("re-translating a guest address: found at its new block", bc_lookup(&bc, gs[2]), again);
    int lv = 0; for (int j = 1; j < 5; j++) lv += bc_lookup(&bc, gs[j]) != 0;
    check("... and it took its own way: the other three are still there", lv, 4);

    // ---- an abandoned reservation moves nothing; a reservation over a segment is refused
    uint32_t p1 = bc_reserve(&bc, 128), p2 = bc_reserve(&bc, 128);
    check("reserve without commit: the next reserve returns the same start", p2, p1);
    check("... and the cursor is at it", bc.cursor & (RING - 1), p1 - (uint32_t)ring);
    check("a reservation larger than a segment is refused", bc_reserve(&bc, SEG + 16), 0);

    // ---- segments: a block that does not fit moves to the next segment; blocks of a whole segment each take one; a lap overwrites the oldest segment
    setup();
    hook_calls = 0;
    uint32_t a = translate(0x8000, 300, 304, 1);
    uint32_t b = translate(0x8002, 300, 304, 1);
    check("a second 300-byte block does not fit in the segment: it starts the next one", b - (uint32_t)ring, ((a - (uint32_t)ring) & ~(SEG - 1)) + SEG);
    check("... the hook ran for that segment entry", hook_calls, 1);
    check("... both are found", (bc_lookup(&bc, 0x8000) == a) + (bc_lookup(&bc, 0x8002) == b), 2);
    uint32_t last = 0;
    for (int i = 0; i < 40; i++) last = translate(0x9000 + 2 * i, 500, 512, 1);
    check("40 blocks of a segment each: the newest is found", bc_lookup(&bc, 0x9000 + 2 * 39), last);
    check("... the previous 7 too", (bc_lookup(&bc, 0x9000 + 2 * 32) != 0) + (bc_lookup(&bc, 0x9000 + 2 * 38) != 0), 2);
    check("... one from a lap ago is gone", bc_lookup(&bc, 0x9000 + 2 * 31), 0);
    check("... the hook ran once per segment entered", hook_calls, 1 + 40);

    // ---- the boundary of liveness: the first block of the oldest live segment is found; one more segment entered and it is gone
    setup();
    uint32_t first = translate(0x7000, 64, 64, 1);
    for (int i = 0; i < 7; i++) translate(0x7100 + 2 * i, 512, 512, 1);          // the other seven segments (the first block's segment was entered first)
    check("8 segments in use: the first block (the very start of the oldest segment) is found", bc_lookup(&bc, 0x7000), first);
    translate(0x7200, 512, 512, 1);
    check("... one more segment entered: it is gone", bc_lookup(&bc, 0x7000), 0);

    // ---- links: the three types; a target recycled while the stub's block is alive puts the stubs back exactly
    static const uint8_t t3[3] = { 0, 1, 2 };
    setup();
    uint32_t B = translate(0xA002, 512, 512, 1);                                     // the target first (older)
    uint32_t A = translate_k(0xA000, 64, 64, 1, 3, t3);                              // the stubs' block after it
    uint32_t B2 = translate(0xA004, 512, 512, 1);                                    // a younger target
    for (int j = 0; j < 3; j++) check("link returns 1", bc_link(&bc, A + 4 * j, j, j == 1 ? B2 : B), 1);
    check("stub 0 is `b B`", *(uint32_t *)A, branch(A, 0, B));
    check("stub 1 is `bl B2` (a call)", *(uint32_t *)(A + 4), branch(A + 4, 1, B2));
    check("stub 2 is `b B` (backward)", *(uint32_t *)(A + 8), branch(A + 8, 2, B));
    inv("linked");
    for (int i = 0; i < 5; i++) translate(0xD000 + 2 * i, 512, 512, 1);            // fill the other segments; the next entry recycles B's
    check("(targets alive) stub 0 still linked", *(uint32_t *)A, branch(A, 0, B));
    inv("before B recycled");
    translate(0xD100, 256, 256, 1);                                                  // (half a segment: the block after it stays in B's segment)
    check("B's segment entered: B is gone", bc_lookup(&bc, 0xA002), 0);
    check("A is alive", bc_lookup(&bc, 0xA000), A);
    check("stub 0 back to `mov lr, pc`", *(uint32_t *)A, 0xE1A0E00Fu);
    check("stub 2 back to `bl h_exit`", *(uint32_t *)(A + 8), unlinked(A + 8, 2));
    check("stub 1 (its target B2 is alive) is still linked", *(uint32_t *)(A + 4), branch(A + 4, 1, B2));
    inv("restored");
    uint32_t B3 = translate(0xA006, 64, 64, 1);
    check("a restored stub can be linked again", bc_link(&bc, A, 0, B3), 1);
    check("... and is a branch to the new target", *(uint32_t *)A, branch(A, 0, B3));
    inv("relinked");

    // ---- stubs with a site word (the translator's direct chaining): the branch that goes to the stub is aimed at the target too; the turnover puts both back
    setup();
    {
        uint32_t Bt = translate(0xA002, 512, 512, 1);                               // the target, in the older segment
        uint32_t *p = (uint32_t *)bc_reserve(&bc, 64);
        for (int i = 0; i < 16; i++) p[i] = 0;
        uint32_t u0 = (uint32_t)p, u1 = (uint32_t)p + 16, s0 = (uint32_t)&p[8], s1 = (uint32_t)&p[9];
        p[0] = unlinked(u0, 0); p[3] = s0;                                         // a jump stub: mov lr,pc ; ... ; site
        p[6] = unlinked(u1 + 8, 1); p[7] = s1;                                     // a call stub: ... ; bl exit ; site
        p[8] = 0x1A000000u | (((u0 - s0 - 8) >> 2) & 0xFFFFFFu);                    // bne stub0
        p[9] = 0xEB000000u | (((u1 - s1 - 8) >> 2) & 0xFFFFFFu);                    // bl stub1
        bc_commit(&bc, 0xB000, 64);
        check("a jump stub with a site links", bc_link(&bc, u0, 0 | 4, Bt), 1);
        check("a call stub with a site links", bc_link(&bc, u1 + 8, 1 | 4, Bt), 1);
        check("the jump stub's first word is `b target`", p[0], branch(u0, 0, Bt));
        check("... and its site (bne) goes to the target", p[8], 0x1A000000u | (((Bt - s0 - 8) >> 2) & 0xFFFFFFu));
        check("the call stub's third word is `b target`", p[6], branch(u1 + 8, 1, Bt));
        check("... and its site (bl) goes to the target", p[9], 0xEB000000u | (((Bt - s1 - 8) >> 2) & 0xFFFFFFu));
        check("a stub of another type with the same words after it is untouched", p[3], s0);
        for (int i = 0; i < 6; i++) translate(0xD000 + 2 * i, 512, 512, 1);
        translate(0xD100, 256, 256, 1);                                             // enters the target's segment
        check("the turnover restores the jump stub", p[0], unlinked(u0, 0));
        check("... and its site goes back to the stub", p[8], 0x1A000000u | (((u0 - s0 - 8) >> 2) & 0xFFFFFFu));
        check("... the call stub", p[6], unlinked(u1 + 8, 1));
        check("... and its site", p[9], 0xEB000000u | (((u1 - s1 - 8) >> 2) & 0xFFFFFFu));
        check("... the site words are still the stubs' words", (p[3] == s0) + (p[7] == s1), 2);
    }

    // ---- a stub whose own segment was recycled before its target's: the record must be skipped, not written into the new block
    setup();
    A = translate_k(0xA000, 64, 64, 1, 3, t3);
    translate(0xA100, 448, 448, 1);                                                  // the rest of A's segment
    B = translate(0xA002, 512, 512, 1);                                              // the target, a segment later
    for (int j = 0; j < 3; j++) bc_link(&bc, A + 4 * j, j, B);
    for (int i = 0; i < 6; i++) translate(0xB000 + 2 * i, 512, 512, 1);
    uint32_t D = translate(0xB100, 512, 512, 1);                                     // A's segment is entered: A is dead
    check("A's segment was recycled: A is gone", bc_lookup(&bc, 0xA000), 0);
    check("... B is alive", bc_lookup(&bc, 0xA002), B);
    translate(0xB102, 512, 512, 1);                                                  // B's segment entered: the record of A's dead stub is skipped
    check("... B is gone now", bc_lookup(&bc, 0xA002), 0);
    check("... and the block that took A's place is untouched", bc_lookup(&bc, 0xB100), D);
    inv("epoch skip");

    // ---- a full list: 63 records per target segment, then the stub stays unlinked
    setup();
    uint32_t T = translate(0xE000, 512, 512, 1);                                     // the target segment is the oldest
    static uint8_t t16[16];
    uint32_t S4[4];
    for (int i = 0; i < 4; i++) S4[i] = translate_k(0xE100 + 2 * i, 512, 512, 1, 16, t16);
    int okl = 0; uint32_t refused = 0;
    for (int i = 0; i < 4; i++) for (int j = 0; j < 16; j++) { uint32_t r = bc_link(&bc, S4[i] + 4 * j, 0, T); if (r) okl++; else { refused++; check("a refused link leaves the stub alone", *(uint32_t *)(S4[i] + 4 * j), unlinked(S4[i] + 4 * j, 0)); } }
    check("63 links accepted into one segment", okl, BC_LMAX);
    check("... the 64th refused", refused, 64 - BC_LMAX);
    inv("full list");
    for (int i = 0; i < 3; i++) translate(0xE200 + 2 * i, 512, 512, 1);
    translate(0xE300, 512, 512, 1);                                                  // the lap: the target's segment is entered
    int restored = 0;
    for (int i = 0; i < 4; i++) for (int j = 0; j < 16; j++) restored += *(uint32_t *)(S4[i] + 4 * j) == unlinked(S4[i] + 4 * j, 0);
    check("... when its segment is entered all 64 stubs read `mov lr, pc` again", restored, 64);
    check("... and its list is empty again", links[((T - (uint32_t)ring) >> 9) * 64], 0);
    inv("after the full list's turnover");

    // ---- flush: everything dies at once, links included, O(segments)
    setup();
    A = translate_k(0xA000, 64, 64, 1, 3, t3);
    translate(0xA100, 448, 448, 1);
    B = translate(0xA002, 128, 128, 1);                                              // the next segment
    bc_link(&bc, A, 0, B);
    translate(0xA200, 512, 512, 1);                                                  // the current segment is a third one: flush restarts there, not in B's
    uint32_t hw0 = bc.hw, hk0 = hook_calls;
    bc_flush(&bc);
    check("flush: hw moved a whole ring ahead", bc.hw, hw0 + RING);
    uint32_t nonempty = 0; for (uint32_t i = 0; i < NSEG; i++) nonempty += links[i * 64] != 0;
    check("... every link list is empty", nonempty, 0);
    check("... the hook ran once (the segment writing restarts in)", hook_calls, hk0 + 1);
    check("... A is gone", bc_lookup(&bc, 0xA000), 0);
    check("... B is gone", bc_lookup(&bc, 0xA002), 0);
    uint32_t E = translate(0xA000, 64, 64, 1);
    check("a block committed after the flush is found", bc_lookup(&bc, 0xA000), E);
    inv("after flush");
    for (int i = 0; i < 20; i++) bc_flush(&bc);
    check("20 more flushes: still empty", bc_lookup(&bc, 0xA000), 0);
    uint32_t E2 = translate(0xA008, 64, 64, 1);
    check("... and the cache works", bc_lookup(&bc, 0xA008), E2);

    // ---- the 32-bit horizon
    setup();
    translate(0x9000, 64, 64, 1);
    bc.cursor = BC_HORIZON - SEG; bc.hw = bc.cursor + SEG - 1;                       // the last segment before the horizon
    uint32_t pa = bc_reserve(&bc, 64);
    check("last segment before the horizon: no restart", bc.cursor, BC_HORIZON - SEG);
    check("... the reservation is at the cursor's ring offset", pa, (uint32_t)ring + ((BC_HORIZON - SEG) & (RING - 1)));
    bc.cursor = BC_HORIZON - 16;
    uint32_t pb = bc_reserve(&bc, 64);
    int zero = 1; for (uint32_t i = 0; i < SETS * 8; i++) if (tags[i]) zero = 0;
    check("a reservation that would enter the horizon: the tags were cleared", zero, 1);
    check("... the cursor restarted at 2 * ring_bytes", bc.cursor, 2 * RING);
    check("... hw is the last byte of that segment", bc.hw, 2 * RING + SEG - 1);
    check("... and the reservation starts at the ring's beginning", pb, (uint32_t)ring);
    uint32_t c2 = translate(0x9100, 64, 64, 1);
    check("a block committed after the restart is found", bc_lookup(&bc, 0x9100), c2);
    check("... and the old one is not", bc_lookup(&bc, 0x9000), 0);

    // ---- the soak
    static const struct { uint32_t ng, umax, slack; } cfg[] = { { NG, 300, 200 }, { 24, 120, 80 }, { 10, 60, 40 } };
    for (unsigned c = 0; c < 3; c++) {
        setup();
        uint32_t hits = 0, misses = 0, commits = 0, aborts = 0, newest_ok = 0, nlinks = 0, nfull = 0, nflush = 0;
        for (int n = 0; n < (c ? 6000 : 3000); n++) {
            uint32_t r0 = rnd() % 100;
            if (r0 < 1 && (rnd() & 3) == 0) { bc_flush(&bc); nflush++; inv("flush"); continue; }
            if (r0 < 45) {                                                          // link a random unlinked stub of a live block to a random live block
                for (int tries = 0; tries < 40; tries++) {
                    meta_t *o = &meta[rnd() % (RING / 16)], *t = &meta[rnd() % (RING / 16)];
                    if (!live(o) || !live(t) || !o->k) continue;
                    uint32_t j = rnd() % o->k, stub = o->code + 4 * j;
                    if (*(uint32_t *)stub != unlinked(stub, o->type[j])) continue;
                    uint32_t r = bc_link(&bc, stub, o->type[j], t->code);
                    if (r) { nlinks++; check("link: the stub is the branch", *(uint32_t *)stub, branch(stub, o->type[j], t->code)); }
                    else { nfull++; check("refused link leaves the stub alone", *(uint32_t *)stub, unlinked(stub, o->type[j])); }
                    inv("link");
                    break;
                }
                continue;
            }
            uint32_t used = 20 + (rnd() % cfg[c].umax);
            used = (used + 3) & ~3u;
            uint32_t k = rnd() % 4;
            uint8_t ty[4]; for (int i = 0; i < 4; i++) ty[i] = rnd() % 3;
            if (used < 4 * k + 8) used = 4 * k + 8;
            uint32_t max = (used + (rnd() % cfg[c].slack) + 15) & ~15u;
            if (max > SEG) max = SEG;
            if (used > max) used = max & ~3u;
            int commit = (rnd() % 10) < 7;
            uint32_t g = G(rnd() % cfg[c].ng);
            uint32_t cd = translate_k(g, used, max, commit, k, ty);
            if (commit) {
                commits++;
                uint32_t r = bc_lookup(&bc, g);
                if (r == cd && verify(g, r, "newest")) newest_ok++;
                else if (r != cd) { printf("[FAIL] the block just committed for 0x%08X is not found (got 0x%08X want 0x%08X)\n", g, r, cd); fail++; }
            } else aborts++;
            for (int q = 0; q < 6; q++) { uint32_t gg = G(rnd() % cfg[c].ng), r = bc_lookup(&bc, gg); if (r) { hits++; verify(gg, r, "soak"); } else misses++; }
            inv("op");
            if (fail > 10) break;
        }
        check("soak: every committed block was found right after its commit", newest_ok, commits);
        printf("[SOAK] %u guests: %u commits, %u abandoned reservations, %u links (%u refused as full), %u flushes, %u hits and %u misses on random lookups\n",
               cfg[c].ng, commits, aborts, nlinks, nfull, nflush, hits, misses);
    }

    printf("[TEST] %d passed, %d failed\n", pass, fail);
    printf(fail ? "FAIL\n" : "PASS\n");
    return fail ? 1 : 0;
}
