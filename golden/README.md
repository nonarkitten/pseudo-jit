# golden/ — an independent 68000 as the reference

PJIT is checked against Ted Fried's MC68000 implementation (MicroCore Labs MCL68, MIT licence): a portable, pure-C interpreter that shares no code or
design with this project. Three uses:

| use | files | run with |
|---|---|---|
| **Ted Fried's opcode test suite**, a self-checking 68000 program, through PJIT's interpreter (`GOLDEN_INTERP`) and through the slot cache and blocks (`GOLDEN_DR`) under QEMU | `mcl68k_testsuite.c`, `mcl68k_test_image.h` | `make golden` |
| **random cases** run on the host through the oracle and on PJIT under QEMU, printed in one format and diffed | `oracle.c`, `pjit_side.c`, `cases.h`, `shim.h` | `make golden-compare` |
| **the generator checks'** oracle: one instruction at a time from a given register state, in batch | `mcl68k.c` via `tools/gen/check/oracle1.c` | `make check-gen` |

The same suite runs on the board: `tests/buffee/buffee_fried.c` is `mcl68k_testsuite.c` with the board's memory map and start-up (`make bundle-fried`). `tb_oracle.c` runs the BASIC interpreter image (`test_progs/tinybasic.c`) on the oracle, an independent check of
`tests/qemu/smoke_tinybasic.c` (`make tb-oracle`).

## Notes

* The suite needs no oracle at run time: its checks are inside the 68000 program (`ALL_DONE` is reached only if every opcode-family subroutine passed). Where the
  suite and PJIT disagreed, two judgment calls were made, both documented in the header of `mcl68k_testsuite.c`: ABCD/SBCD on invalid BCD digits (the harness patches
  eight `cmpi.l` constants to PJIT's hardware-derived model) and DIVU/DIVS overflow N/Z (undefined by Motorola; "unchanged").
* PJIT leaves an ordinary ALU op's flags live in the ARM CPSR instead of writing `cpu->sr`, so `pjit_side.c` ends each case with a real `MOVE SR,D2` and reads *that*;
  reading `cpu->sr` after an arbitrary opcode shows whatever it last held for unrelated reasons.
* `oracle.c` takes `<seed-hex> <count>`; `pjit_side.c`'s seed and case count are compile-time constants (there is no argv under semihosting), so to compare another seed
  change both files the same way.
* The oracle has a few known departures from the 68000 (BTST Dn,#imm bit numbers modulo 32; LINK A7; MOVE An,(An)+ stored value). `tools/gen/check/sim_test.py` lists them as
  "oracle limits" and an independent model (`tools/gen/check/model68k.py`, written from the manual) settles each disagreement.
* The random cases cover register-direct ADD/SUB/AND/OR/CMP in B/W/L. More families need only new cases in `cases.h`; the checks of `make check-gen` cover the rest.
