/// @file smoke_exception_dr.c
/// @brief smoke_exception_exec.c's cases through the C-free runtime with the direct-mapped DR
///        cache (the default threshold: guest code above 16MB is cached).
#define PJIT_TEST_INTERP 1
#include "smoke_exception_exec.c"
