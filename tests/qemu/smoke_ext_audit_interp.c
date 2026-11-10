/// @file smoke_ext_audit_interp.c
/// @brief smoke_ext_audit.c through the interpretive runtime instead of the DR cache.
#define PJIT_TEST_NO_DR 1
#include "smoke_ext_audit.c"
