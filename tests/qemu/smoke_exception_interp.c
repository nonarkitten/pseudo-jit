/// @file smoke_exception_interp.c
/// @brief smoke_exception_exec.c's cases through the C-free runtime, interpretive only
///        (cpu->dr_threshold = ~0: nothing is cached).
#define PJIT_TEST_INTERP 1
#define PJIT_TEST_NO_DR 1
#include "smoke_exception_exec.c"
