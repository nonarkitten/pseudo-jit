/// @file bbb_dhcycles.c
/// @brief Where do the cycles of the cached Dhrystone run go, per translated block? (tests/include/sampler.h with the cycle event, plus the block table.) `make bbb-bbb_dhcycles`.
#define BBB 1
#define HIST_TOP 5
#define RUNS_A 1000
#define RUNS_B 3000
#define SAMPLE_EVENT 0x11
#define SAMPLE_N 1024
#include "sampler.h"
#define SAMPLE 1
#include "smoke_dhrystone.c"
