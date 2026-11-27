/// @file smoke_probe_interp.c
/// @brief smoke_probe.c's cases through the C-free runtime, interpretive only (cpu->dr_threshold = ~0: nothing is cached).
#define PJIT_TEST_NO_DR 1
#include "smoke_probe.c"
