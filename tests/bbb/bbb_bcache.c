/// @file bbb_bcache.c
/// @brief The one thing QEMU's TCG cannot check about the block cache: the instruction-cache maintenance after a stub is patched (bc_link) and put back
///        (segment turnover). Real silicon, caches on: a stub is executed (warming the I-cache and branch predictor), linked, executed (must now take the
///        link), its target's segment is entered, executed again (must be back to the unlinked path). 300 rounds. `make bbb-bbb_bcache`.
#include <stdint.h>
#include "pjit.h"
#include "bcache.h"
#include "bbb.h"

#define RING 4096u
#define SEG  512u
#define SETS 32u
static uint8_t  ring[RING] __attribute__((aligned(4096)));
static uint32_t tags[SETS * 8] __attribute__((aligned(64)));
static uint32_t links[(RING / SEG) * 64] __attribute__((aligned(64)));
static uint8_t  epochs[RING / SEG];
static bcache_t bc;
extern void arm_flush_cache(uintptr_t addr, uint32_t length);

extern uint32_t call_blk(uint32_t addr), h_exit_fn(void);
__asm__(".text\n.arm\n.global call_blk\ncall_blk:\n  push {r4, lr}\n  adr r4, 1f\n  bx r0\n1:\n  pop {r4, pc}\n"
        ".global h_exit_fn\nh_exit_fn:\n  mov r0, #1\n  bx r4\n");

static uint32_t unlinked_bl(uint32_t stub) { return 0xEB000000u | ((((uint32_t)h_exit_fn - stub - 8) >> 2) & 0xFFFFFFu); }

int main(void)
{
    printf("=== block cache on silicon: I-cache maintenance of patched stubs ===\n");
    bc_init(&bc, ring, RING, SEG, tags, SETS, links, epochs);
    bc.h_exit = (uint32_t)h_exit_fn; bc.h_call = (uint32_t)h_exit_fn;
    int bad_link = 0, bad_unlink = 0, bad_base = 0;
    for (int round = 0; round < 300; round++) {
        uint32_t T = bc_reserve(&bc, SEG);                      // the target: a whole segment, the oldest
        ((uint32_t *)T)[0] = 0xE3A00002u; ((uint32_t *)T)[1] = 0xE12FFF14u;       // mov r0,#2 ; bx r4
        bc_commit(&bc, 0x1000 + 2 * round, SEG);
        arm_flush_cache(T, 8);
        uint32_t X = bc_reserve(&bc, 64);                       // the stub's block, in the next segment
        ((uint32_t *)X)[0] = unlinked_bl(X); ((uint32_t *)X)[1] = 0xE3A00001u; ((uint32_t *)X)[2] = 0xE12FFF14u;   // bl h_exit ; mov r0,#1 ; bx r4
        bc_commit(&bc, 0x2000 + 2 * round, 64);
        arm_flush_cache(X, 12);
        for (int i = 0; i < 20; i++) if (call_blk(X) != 1) bad_base++;     // warm
        if (!bc_link(&bc, X, 2, T)) bad_link += 1000;
        for (int i = 0; i < 3; i++) if (call_blk(X) != 2) bad_link++;      // must take the link
        for (int i = 0; i < 6; i++) { bc_reserve(&bc, SEG); bc_commit(&bc, 0x3000 + 2 * i, SEG); }   // fill the other segments ...
        if (call_blk(X) != 2) bad_link++;                                  // (target still alive)
        bc_reserve(&bc, 16);                                    // ... the next entry is the target's segment: its stubs are put back
        for (int i = 0; i < 3; i++) if (call_blk(X) != 1) bad_unlink++;
        bc_commit(&bc, 0x4000, 16);                             // keep the cursor consistent
    }
    printf("[BCACHE] unlinked path wrong: %d, linked path wrong: %d, restored path wrong: %d (of 300 rounds)\n", bad_base, bad_link, bad_unlink);
    printf((bad_base | bad_link | bad_unlink) ? "FAIL\n" : "PASS\n");
    return 0;
}
