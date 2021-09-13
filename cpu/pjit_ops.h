/*
 * Copyright (c) 2020-2021 Renee Cousins, the Buffee Project
 * See pjit.h for details
 *
 * The core interpreter of PJIT; finds 68K opcodes and does things
 * with them like stuff them back in the cache, run them, etc
 *
 */

#ifndef __PJIT_OPS_H_
#define __PJIT_OPS_H_

#include "pjit.h"

// return the ARM32 opcode for an unconditional branch to the target
extern uint32_t cpu_branch_offset(void* target, void* current);

// look up opcode and execute it, but never replace it
extern void cpu_lookup_nojit(void);

// look up opcode and replace our call with the opcode
// and then also execute it
extern void cpu_lookup_safe(void);

// look up code and if it's small enough, replace the
// caller with the function body, otherwise, replace with
// the branch to it and then execute it
extern void cpu_lookup_inline(void);

#endif /* __PJIT_OPS_H_ */

