/// @file smoke_tables.c
/// @brief Sanity-checks the generated opcode and extension-word tables.
#include <stdint.h>
#include "pjit.h"

extern void emit_opcode_table(void);
extern void qemu_exit(void);

/* Room for pjit_cache_init()'s downward bump allocations: cache_data
 * (1MB @ default BLOCK_BITS/INDEX_BITS), maprom_data (1MB, since the
 * zero-initialized default config's maprom_page happens to be 0 != 0xFF),
 * opcode_table (256KB), opcode_stubs (2MB), cache_tags (4KB). ~4.25MB
 * rounded up generously. */
static uint8_t ram_pool[12 * 1024 * 1024] __attribute__((aligned(4096)));

int main(void)
{
    printf("=== PJIT core smoke test: table generation ===\n");

    pjit_cache_init((uint32_t)(ram_pool + sizeof(ram_pool)));
    printf("[TEST] pjit_cache_init() returned\n");

    emit_opcode_table();
    printf("[TEST] emit_opcode_table() returned\n");

    printf("PASS\n");
    qemu_exit();
    return 0;
}
