/// @file smoke_blkfast_empty.c
/// @brief The block fast table's EMPTY entries must never match a guest transfer: a zeroed entry {0, 0} matches a transfer to guest address 0 and the router's fast path branches to host
///        address 0. An empty entry is {an odd key (a big-endian 1), the router's slow path}.
///        Checks the table after boot, after pjit_cache_flush and after a latched interrupt would clear it (pjit_post_flush).
#include <stdint.h>
#include "pjit.h"
#include "blk.h"

extern void emit_opcode_table(void), pjit_post_flush(void);
static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));
static int check(const char *when)
{
    int bad = 0, zero = 0;
    for (uint32_t i = 0; i < 2u << BLK_FAST_BITS; i += 2) {
        if (blk_fast[i] != 0x01000000u) bad++;                                  // the key: an odd address, big-endian 1
        if (blk_fast[i + 1] == 0) zero++;                                       // the block word: never zero
    }
    printf("%s: %d entries without the odd key, %d with a zero block word: %s\n", when, bad, zero, (bad || zero) ? "FAIL" : "PASS");
    return bad || zero;
}
int main(void)
{
    printf("=== PJIT smoke test: empty entries of the block fast table ===\n");
    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    emit_opcode_table();
    int bad = check("after boot");
    pjit_cache_flush(); bad |= check("after pjit_cache_flush");
    pjit_post_flush(); bad |= check("after pjit_post_flush");
    printf("%s\n", bad ? "FAIL" : "PASS");
    return 0;
}
