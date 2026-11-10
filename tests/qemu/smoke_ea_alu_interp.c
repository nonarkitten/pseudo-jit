/// @file smoke_ea_alu_interp.c
/// @brief smoke_ea_alu_exec.c's cases through the C-free runtime, interpretive only
///        (cpu->dr_threshold = ~0: nothing is cached).
#define PJIT_TEST_INTERP 1
#define PJIT_TEST_NO_DR 1
#include "smoke_ea_alu_exec.c"
