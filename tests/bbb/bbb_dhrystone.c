/// @file bbb_dhrystone.c
/// @brief The Dhrystone measurement on a BeagleBone: interpretive and cached cycles per run from the ARM cycle counter, plus the
///        performance monitor's events (instructions, mispredicted branches, L1D / L1I misses) for the cached run.
///        `make bbb-bbb_dhrystone`; see platform/beaglebone/README.md.
#define BBB 1
// The guest prints nothing (smoke_dhrystone.c sets the QUIET cell, so its printf returns at once). No UART polling falls inside the timed window.
#define HIST_TOP 400     // the opcode histogram, as complete as the program needs: tools/study/profile_cost.py weights per-opcode costs by it
#define RUNS_A 1000
#define RUNS_B 3000
#include "smoke_dhrystone.c"
