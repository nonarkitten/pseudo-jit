/// @file bbb_tbcycles.c
/// @brief Where do the cycles of the cached Tiny BASIC run go? bbb_tbsample.c with the PMU's cycle event. `make bbb-bbb_tbcycles`.
#define SAMPLE_EVENT 0x11
#define SAMPLE_N 2048
#include "bbb_tbsample.c"
