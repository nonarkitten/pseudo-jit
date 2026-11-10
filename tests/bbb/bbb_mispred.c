/// @file bbb_mispred.c
/// @brief Where do the mispredicted branches of the cached Dhrystone run happen? (tests/include/sampler.h) `make bbb-bbb_mispred`.
#define BBB 1
#define HIST_TOP 5
#define RUNS_A 1000
#define RUNS_B 3000
#include "sampler.h"
#define SAMPLE 1
#include "smoke_dhrystone.c"
