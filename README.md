# Pseudo JIT

PJIT is a real-time version of JIT that avoids the traditional issues of dynamic memory allocation and non-determinism.

# Summary of PJIT

## PJIT Organization

PJIT consists of two major tables: the opcode thread cache (simply 'the cache') and the opcode literal table (simply 'the table').

### The Cache

The cache is constructed as an n-way set associative cache where the n-weight is a compile-time option to be determined during performance testing later. It will use roughly 1/8 to 1/2 all of available system memory, with the final actual size to be determined later -- a large cache might be underutilized with the typical size of executables to be found on the Amiga platform.

Each cache page is seaprated by a block of return statements to the outer interpreter to handle the variable opcode lengths of the 68000.

### The Table

The table consists of every opcode permutation as a single, algorithmically generated function. Each function can then assume to know before-hand the sum of all it's registers, addressing modes and operations, save for when the so-called extended addressing modes are used.

## PJIT Processes

PJIT consists of two major processes: the outer interpreter and the inner interpreter.

### The Inner Interpreter

The inner interpreter is subroutine threaded where each opcode is implemented in the cache as a soubroutine call and each operation is terminated with a return statement. Because of branch-prediction and the return-link stack; the overhead of subroutine threading should be negligible and should exceed any other threading model save for true-inlining. However, small subroutines could be inlined at a later date in the outer interpreter. 

### The Outer Interpreter

The outer interpreter[^1] will find the entry point into the cache and if necessary, invalidate the page when a cache miss occurs. In this case, the cache is preloaded with the address of the opcode lookup function. This function will use the current real program counter to determine what the 68000 program counter ought to be, looks up the opcode in the table and substitutes the cache entry with a call to the opcode itself. It then calls the opcode immediately before returning.

### Opcode Handlers

All normal (straight-line) opcodes within the table simply end in a return statement; usually 'ld pc, lr'. 

All non-conditional branching opcodes within the table simple jump to the the outer interpreter.

All conditional branches within the table will either return (branch not-taken) or jump to the outer interpreter (branch taken).

## PJIT Optimizations

The process of the outer interpreter may be broken into several phases, or passes.

On the first pass, the most generic form of the opcode may be used to ensure maximum correctness. Because handling control flow and 68000 processor flags are complicated by architectural differences between the ARM and 68K processors, this results in sub-optimal execution. The outer interpreter will track which flags are set at which points and when those flags are used by any conditional code.

When a condition is encountered, we mark the preceeding instructions that were responsible for setting the corresponding flags.

However, on this first pass, the outer interpreter replaces the cache instruction for another level of optimization instead of the opcode itself. This means the second pass will still involve the outer interpreter on each instruction.

On the second pass through the cache, any instructions that may set flags but have not been marked may be replaced with an optimized version of the instruction that does not bother with setting the flags. If these instructions are a single ARM instruction then they are also a candidate for inlining to improve performance even further. Any instructions marked by the first pass are replaced with their full-flag setting variant and are not inlined.

This optimized approach will not be implemented in the first version of PJIT.

# Notes

[^1]: In classic Forth, the outer interpreter is always constructed of statements of the inner interpreter which means that the true state of the machine never leaves of inner interpreter context; with a machine implementation of the inner interpreter, this model doesn't make sense, so the outer interpreter shall be regular ARM code as compiled by the C compiler.