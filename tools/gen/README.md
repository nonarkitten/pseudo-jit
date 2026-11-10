# The generator: how the tables are made

`make` runs two scripts from this directory (needs the virtual environment, `make venv`) and nothing else here:

```
tools/gen/maketables.py  > build/<variant>/gen/tables.s    interpreter + slot cache tables (DT, OT, DRSR, ...)
tools/gen/btables.py     > build/<variant>/gen/btables.s   block translator tables (per-opcode leader + body, kind, flag info)
```

Everything in `tools/gen/` (this directory, not `check/`) is what those two need. Tests are in `check/`, measurement and modelling studies in
[`../study/`](../study), the data files the generator reads in [`../data/`](../data). The generated assembly is readable: mnemonics with the 68K instruction as a
comment (`make listing` writes the whole thing in one file). The tables are byte-for-byte reproducible: the same sources give the same `tables.s`.

## The pipeline

```
optab_rows.py   which emitter handles which opcode           (data: (mask, value, 'emit_NAME') rows, later rows win)
optab.py        applies the rows -> winner[opcode] = 'emit_NAME'   (numpy)
arm32.py        ARM instruction encoders: add(), ldr(), bx(), ... -> 32-bit words (no assembler)
gen.py          Gen.generate(opcode) -> the ARM word sequence for one opcode (dispatch, D0 coalescing)
emitters/       the emitters: one mixin per instruction group, assembled into Gen
maketables.py   Gen over all 65,536 opcodes -> layout: DT (16-byte slot per opcode), OT class bits, ext-class table,
                  DRSR (shared bodies, interned, 16-byte units), MOVEM table, inline table -> text on stdout
abstract.py     procedural abstraction of the DRSR bodies (shared tails / runs become routines; weights.py = the budget)
btables.py      the same sequences for the block translator: split into leader + body, flag-liveness variants
                  (blockasm.py: decode, prefix, inline_body ...; armdce.py: ARM-level dead flag elimination;
                   fwd.py / constfold.py / regalloc.py / schedtab.py: the block optimisers the asm translator mirrors)
listing.py      human-readable dump of every opcode's sequence (make listing)
paths.py        where the build puts files (PJIT_BUILD, set by make)
```

### The emitters (`emitters/`)

`gen.Gen` is these mixins plus the dispatcher. A method named `emit_<NAME>` is an emitter that `optab_rows.py` names; the others are helpers shared inside a group.

| module | holds |
|---|---|
| `ea.py` | effective-address load and store (Dn, An, (An), (An)+, -(An), d(An), d(An,Xn), abs, PC-relative, immediate), the D registers (`dload` / `dstore`, D0 is pinned in r4) |
| `alu.py` | the X flag, the ALU core with its flag fix-ups, MOVE / MOVEA / MOVEQ and the ADD / SUB / AND / OR / EOR / CMP families (Dn, An and immediate forms) |
| `shift.py` | ASL / ASR / LSL / LSR / ROL / ROR / ROXL / ROXR by immediate, register and in memory |
| `misc.py` | CLR NEG NOT, ADDQ SUBQ, EXT SWAP TST, MULU MULS, CMPM, LEA, EXG, the bit operations, Scc |
| `xops.py` | ADDX SUBX NEGX |
| `flow.py` | Bcc BRA BSR DBcc JMP JSR RTS RTR TRAP TRAPV and the illegal / line-A / line-F cases |
| `sr.py` | status register, CCR, USP, ANDI / ORI / EORI to SR and CCR, RTE, STOP, RESET (and the function-code GPIO on Buffee) |
| `complex.py` | DIVU DIVS CHK TAS, LINK UNLK PEA, MOVEP, MOVEM, ABCD SBCD NBCD |
| `base.py` | the sequence type (`Seq`: words plus the split between an opcode's own words and the shared routine) and the environment switches |

A sequence is a list of 32-bit ARM words (or `('b', cond, 'symbol')` tuples for branches that leave the table), always ending in `bx lr`
or a branch. **To change what an instruction compiles to, change its `emit_*` method (or `ea_load` / `ea_store` / `alu` if it is
shared); nothing else is hand-edited.** `Gen.D0` is the offset of D0 in `cpu_t`; D registers are read and written
with `ldr/str [r5, #D0 + 4n]` (big-endian: byte at +3, halfword at +2) — except D0, which is pinned in r4 (`Gen.dload / dstore / ea_load(inplace=True)`;
`PJIT_PIN_D0=0` for the unpinned design) — A0-A7 are the ARM registers r6-r13 (`ea_load` returns the register itself, no load), flags N Z V C are the
live ARM flags, X is in GE[3:0].

The environment switches the Makefile passes (`PJIT_PIN_D0`, `PJIT_X_LAZY`, `PJIT_FC`) are read in `emitters/base.py`; the others (`PJIT_SPLIT_LONG`,
`PJIT_SPLIT_ALU`, `PJIT_LEGACY`) are experiments, off by default.

## Checking a change

```
make check-gen          check-asm (GNU as re-derives every word from the text) + check_rules + sim_test + trap_test
make test               the QEMU smoke tests
tools/gen/check/*.py    the rest, run by hand: flagmeta + dce_test (dead-flag elimination), fold_test, alloc_test, rox_test, known_test, block_test, audit
```

`check/sim_test.py` runs every family it models under Unicorn against the 68000 oracle (`golden/mcl68k.c`, via `check/oracle1.c`) and an independent
model (`check/model68k.py`). `PJIT_STATS=1 tools/gen/maketables.py` prints the DRSR size per family.

## Directory map

| directory | contents |
|---|---|
| `tools/gen/` | the generator proper (above) + `known.py` / `a7.py` (static stack analysis the block assembler uses) + `sched_asm.py` (runs the asm scheduler under Unicorn) |
| `tools/gen/emitters/` | the instruction emitters |
| `tools/gen/check/` | tests: `sim.py` (Unicorn harness), `sim_test.py`, `trap_test.py`, `rox_test.py`, `dce_test.py`, `fold_test.py`, `alloc_test.py`, `known_test.py`, `sched_test.py`, `block_test.py`, `check_rules.py`, `audit.py`, `flagmeta.py`, `model68k.py`, `oracle1.c` |
| `tools/study/` | trace-driven models and measurements ([../../docs/tools.md](../../docs/tools.md)) |
