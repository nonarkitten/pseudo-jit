# Tests

Three families, by where they run. Every test is a bare-metal program built from the translator's own objects;
its source file starts with a comment that says what it checks. Build output goes to `build/<variant>/` (see
[../docs/building.md](../docs/building.md)).

| directory | runs on | how | what |
|---|---|---|---|
| [`qemu/`](qemu) | `qemu-system-arm` (Cortex-A15 "virt"; A8 model for `make a8-test`) | `make test` | correctness: 73 programs, 58 of them in `make test` |
| [`bbb/`](bbb) | a BeagleBone Black (Cortex-A8, 1 GHz) | `make bbb-<name>`, `tools/bbb/run.py` | cycle counts, mispredicts, latency: the performance lab |
| [`buffee/`](buffee) | a Buffee in an Amiga | `make bundle-<name>`, `tools/buffee/run_local.py` | the bus, the chipset, Kickstart: the board's tests |
| `include/`, `data/`, `kcmp/` | — | — | shared headers; checked-in generated data (`tools/data/`); C kernels compiled for the 68000 |
| `run.sh`, `run_a8.sh` | — | called by `make` | run the built QEMU tests in parallel and print a report |

## tests/qemu — `make test`

A test prints a report and ends with a line `PASS` or `FAIL`; `run.sh` runs them `JOBS` at a time and fails if any
did not print a clean `PASS`. `make run-<name>` builds and runs one. **A new test is added by creating the file and
adding its name to `TEST_NAMES` in [`mk/qemu.mk`](../mk/qemu.mk)** — that list is the only one.

Naming: `smoke_<what>.c`. A pair `smoke_x_interp.c` / `smoke_x_dr.c` runs the same body through the interpreter alone and
through the cache (`_dr`; the `_dr` file is three lines that define `PJIT_TEST_DR` and include the other). The kinds:

* **single instructions and addressing modes** — `smoke_*_exec` (ALU, BCD, BTST, branches, calls, multiply/divide, MOVEP, supervisor mode, user traps, extension words, `smoke_ea_alu_*`, `smoke_xmodes_*`, `smoke_index*`, `smoke_alias_*`, `smoke_movem_*`), `smoke_div`, `smoke_exception_*`, `smoke_control_*`, `smoke_irq_*`;
* **the generated tables** — `smoke_tables`, `smoke_ot`, `smoke_ext_audit*`;
* **the caches** — `smoke_links`, `smoke_rts`, `smoke_defer`, `smoke_fuse`, `smoke_bsrb`, `smoke_loops`, `smoke_bcache`, `smoke_blkfast_empty`, `smoke_evict_stress`, `smoke_fill_spill`, `smoke_fill_pad`, `smoke_lzt`, `smoke_xlazy`;
* **whole programs** — `smoke_tinybasic` (a BASIC interpreter, `test_progs/tinybasic.c`, runs a program), `smoke_kernels` (C kernels), and, when their data is present, `smoke_kcmp` (the same C compiled for the 68000 and run natively on the ARM, compared), `smoke_corpus`, `smoke_doynax`;
* **not in `make test`**: `smoke_dhrystone*`, `smoke_bogomips*`, `smoke_native_bogomips` (the programs the benchmarks build on), `smoke_dhrystone_trace`, `trace_tinybasic`, `hist_tinybasic` (write instruction traces and opcode histograms for `tools/study/`), `amiga_diff` + `amiga_hook` (the differential harness, `tools/amiga/`).

Beyond `make test`: `make check-gen` (the generator against a 68000 oracle), `make golden` (Ted Fried's MC68000 suite), `make a8-test`
(20 of these tests on the Cortex-A8 model).

## tests/bbb — the performance lab

Run on a real Cortex-A8: QEMU cannot say how many cycles anything takes. On a Buffee board (the same core): `make ONBUFFEE=1 VARIANT=onbuffee onbuffee-all`, then
`tools/buffee/run_bench.py build/onbuffee/buffee` runs them in turn and keeps each console in `local/bench/`. The images are the benchmarks linked with the Buffee board layer: their
static arrays and the block ring live in the guest pool of the DDR (`platform/buffee/bench_dram.ld`, within the 32 MB a `bl` reaches from the runtime), the cache is at the platform's
place, and the platform's translate map and write detection are off, as on a BeagleBone. On a BeagleBone Black `tools/bbb/run.py` copies an image, runs it and prints the console.
Timings move 5–10% with code alignment, so `tools/bbb/scan.sh` runs a build over several layouts.

| test | measures |
|---|---|
| `bbb_dhrystone`, `bbb_tinybasic` | the headline numbers: cycles per Dhrystone run, a BASIC sieve, interpreted and cached |
| `bbb_bogomips`, `bbb_branch`, `bbb_micro` | a delay loop; cycles and mispredicts per control-flow construct; the building blocks of the emitted sequences |
| `bbb_opcost`, `bbb_wcet`, `bbb_claimcost`, `bbb_cadence` | the cached cost of single opcodes; best and worst latency of one instruction; where a line claim's cycles go; run-to-run and alignment determinism |
| `bbb_chain`, `bbb_chain_prof` | first-run latency of the slot cache and the block runtime (with a PMU sampler) |
| `bbb_blocks`, `bbb_blocks_cf`, `bbb_blocks_sched` | hot straight-line segments of generated code as loops: raw, constant-folded, scheduled |
| `bbb_dhcycles`, `bbb_mispred`, `bbb_tbsample`, `bbb_tbcycles` | PMU sampling profilers: where the cycles / mispredicts of Dhrystone and the BASIC interpreter happen |
| `bbb_corpus` | the 68000 corpus (`tools/data/corpus.py`), cold and warm cycles per program |
| `bbb_bcache` | the instruction-cache maintenance after patching a block stub — the one thing QEMU cannot check |
| `bbb_irq`, `bbb_window` | interrupts through the block runtime; what an interrupt window costs |
| `bbb_lzt` | the speed of the table-bundle decoder (the bundle is packed from the image's own tables when it is built) |

## tests/buffee — on the board

Payloads for the bootloader (`make bundle-<name>`, then `tools/buffee/run_local.py <bundle>`). They need an Amiga
with the board in it; most print one summary line and a `PASS`/`FAIL`.

| test | checks |
|---|---|
| `buffee_hello` | the first program: a loop, chipset registers, VERTB interrupts, held interrupt levels, MapROM, unaligned longs, MOVEM, blits, DMA integrity, CIA write/read-back |
| `buffee_boot` | **boots Kickstart** on PJIT (`make firmware` bundles it); with a PC sampler and state dump unless built `-DCLEAN` |
| `buffee_fried` | Ted Fried's MC68000 suite on the board |
| `buffee_endian`, `buffee_abort`, `buffee_smc`, `buffee_fc`, `buffee_regtest` | lane/endianness sanity; data aborts become address/bus errors; self-modifying code; the function-code GPIOs; a guest RAM region behaves as RAM |
| `buffee_chipfull` | **all of chip RAM** — marches, display DMA on, weights, random frames; run it after any bus-timing change |
| `buffee_top`, `buffee_weight`, `buffee_cia4`, `buffee_blit` | regression kernels from the bus bring-up: top-of-RAM writes, write errors vs the number of high address lines, CIA read-back, blitter health |
| `buffee_chips`, `buffee_floppy`, `buffee_peekrom` | bring-up probes: chip IDs and register stability, the floppy path without Kickstart, ROM words through the window |
| `buffee_speed`, `buffee_kspeed`, `buffee_bus` | throughput by access type; guest loops on the bus vs in host DRAM; a bus-cycle generator for logic-analyser captures |
| `buffee_shotgun` | the payload of `tools/buffee/shotgun.py`: GreenPAK and GPMC edits applied live while a read/write test runs |
| `buffee_blitdur`, `buffee_crumbs`, `buffee_reroll` | blit duration under CPU polling; the post-mortem ring that survives a watchdog reset; which board-side piece holds per-boot state |
