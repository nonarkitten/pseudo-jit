/// @file bbb_lzt.c
/// @brief The table bundle decoder on the Cortex-A8 (`make bbb-bbb_lzt`): cycles to load a whole LZTB bundle (pjit/lzt.h) of the real tables (DT, OT, shared routines, the block
///        translator's tables: 4.5 MB), cold and warm, and a checksum per part.  The bundle is packed by the makefile from the tables of bbb_dhrystone.elf into
///        build/<variant>/bbb/lzt_bundle.h (tools/pack/lzt_elf.py <elf> <out.lzt> --rel --header <out.h>).
#include <stdint.h>
#include "pjit.h"
#include "bbb.h"
#include "lzt.h"
#include "lzt_bundle.h"

static uint8_t region[REAL_REGION + 512] __attribute__((aligned(64))), scr[2500000] __attribute__((aligned(64)));

int main(void)
{
    lzt_neon_enable();
    printf("=== LZTB bundle: %u bytes packed, %u parts, region %u bytes ===\n", (unsigned)sizeof real_bundle, REAL_NPARTS, REAL_REGION);
    for (int run = 0; run < 3; run++) {
        for (uint32_t i = 0; i < REAL_REGION; i += 64) region[i] = 0xEE;
        uint32_t t0 = bbb_pmu_cycles(); int rc = lztb_load(real_bundle, region, scr); uint32_t t1 = bbb_pmu_cycles();
        int bad = 0;
        for (int p = 0; p < REAL_NPARTS; p++) {
            uint32_t s = 0;
            for (uint32_t i = 0; i + 4 <= real_parts[p].size; i += 4) s += (*(uint32_t *)(region + real_parts[p].off + i)) * (i / 4 + 1);
            bad += s != real_parts[p].sum;
        }
        printf("[LZT] run %d: rc=%d %u cycles (%u us), %d parts wrong\n", run, rc, t1 - t0, (t1 - t0) / 1000, bad);
    }
    return 0;
}
