/// @file bbb_chain_prof.c
/// @brief bbb_chain.c with the PMU cycle sampler on the first cold pass: where the cycles of a first run go (`make [BLK=1] bbb-bbb_chain_prof`; the [S] lines are `PC count` pairs: aggregate them by symbol with the ELF's symbol table).
#define SAMPLE 1
#include "bbb_chain.c"
