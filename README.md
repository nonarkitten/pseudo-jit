# PJIT and Buffee

PJIT is part of [the Buffee Project](https://www.buffee.ca).

**PJIT** runs Motorola 68000 programs on an ARM processor. It does not emulate the processor instruction by instruction: it keeps a ready-made piece of ARM code for every
possible 68000 instruction, and strings the pieces together as a program runs. Code that runs rarely is interpreted, code that runs often is copied into a straight line, and the
hottest code is optimised as a whole.

**Buffee** is the accelerator board that puts it to work: an OSD335x (a 1 GHz ARM Cortex-A8 with DDR3 memory) that plugs into the CPU socket of an Amiga 500+ and takes the place
of its 68000. A small bootloader brings the board up, PJIT runs the Amiga's own Kickstart, and the Amiga's chipset sees what looks like an ordinary 68000 on the bus.

The board's schematics and Gerber files are in <https://github.com/nonarkitten/buffee>.

```
  Amiga 500+ -- 68000 socket -- [ Buffee ]
                                    |- GPMC + GreenPAK + PRU: a 68000 bus master (AS/UDS/LDS/DTACK, E clock, VPA/VMA, IPL, RESET)
                                    |- AM335x Cortex-A8 at 1 GHz: PJIT, the guest's registers in ARM registers
                                    |- DDR3: fast RAM for the guest, the translation caches, a MapROM copy of Kickstart
                                    '- SPI flash + EEPROM: bootloader, Kickstart bundle, ROM image, settings
```

## What it does

* Boots **Kickstart 3.1 to Workbench** on an Amiga 500+ (PAL) from the stock ROM and a floppy. Code that runs from the board's own DRAM (the ROM copy and, if you add it, fast RAM) runs at about 300 MIPS in SysInfo (see the table below for the benchmarks that are part of this repository).
* Every access to the Amiga's bus takes 6 CLK7 periods (a real 68000 needs 4), about 2.3 MB/s. Code that lives on the bus (chip RAM, a RAM board) is interpreted and paced to the speed of a 68000, never translated, so DMA masters cannot surprise it.
* Is tested against Ted Fried's MC68000 test suite, against an independent 68000 oracle, and with a set of programs under QEMU and on a BeagleBone Black (see [docs/building.md](docs/building.md)).
* Emulates the 68000 only: no 68010 and later, no FPU, no MMU.

## How fast

Measured on a Buffee board (1 GHz Cortex-A8), with the benchmarks of [tests/bbb](tests/bbb) run by [tools/buffee/run_bench.py](tools/buffee/run_bench.py); the guest code is in the board's DDR.
Timings move 5-10% with code alignment, and repeated runs of one image agree within a percent.

| Benchmark | Interpreted | Translated |
|---|---|---|
| Dhrystone 2.1 | 26,356 cycles per run<br/>54.7 per instruction</br></br>21.6 DMIPS | 5,115 cycles, first run</br>1,629 cycles per run<br/>3.4 per instruction<br/>349 DMIPS  |
| BogoMIPS | 18.8 | 287 |
| SysInfo | 0.43 (Chip RAM) | 299 (CPU RAM) |

On the Amiga's own bus every access takes 6 CLK7 periods (about 850 ns, 2.3 MB/s). Code that runs from there is interpreted at the speed of a 68000 (`buffee_kspeed`: 0.66 to 0.98 times).

At 3.4 cycles per 68000 instruction there's not much left on the table for performance. There are plenty of edge cases where more optimization could be performed, but the extra work is unwarranted -- look at the cold versus warm times for translated code.

## Where to start

If you're new to the project, head over to **[Getting Started](https://github.com/nonarkitten/pseudo-jit/wiki/getting-started.md)** or explore  **[the Wiki](https://github.com/nonarkitten/pseudo-jit/wiki)** for more information.

## Building in one minute

```sh
brew install arm-none-eabi-gcc qemu     # macOS; on Linux: gcc-arm-none-eabi and qemu-system-arm
make venv                               # .venv with the Python packages the table generator needs
make test                               # generates the tables, builds the tests and runs them under QEMU (about a minute)
make firmware                           # the Buffee bootloader and the standard Kickstart bundle -> build/firmware/
make help                               # every target
```

Everything is written to `build/<variant>/`; nothing is generated inside the source tree except the files that are checked in on purpose (`pjit/ext_table.c`, the GreenPAK image,
the test data under `tests/data/`).

## Layout

```
pjit/          the translator: interpreter, slot cache, block runtime (ARM assembly) and their glue
platform/      the machines PJIT runs on: QEMU virt, BeagleBone Black, Buffee (+ a freestanding libc shim)
bootloader/    the Buffee bootloader: starts the board and loads PJIT
tools/         the table generator and its checks, board tools, studies, packers
tests/         tests: tests/qemu (QEMU), tests/bbb (BeagleBone benchmarks), tests/buffee (on the board)
golden/        Ted Fried's 68000 interpreter as an oracle, and his test suite
mk/            the pieces of the Makefile
test_progs/    guest programs the tests run (Dhrystone, a BASIC interpreter)
local/         your machine's files: ROM images, traces, known-good firmware (git-ignored)
```

## Licence

Everything the Buffee Project wrote is licensed under **Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International** ([LICENSE](LICENSE)): you may copy, change
and share it with credit and under the same licence, **not for commercial purposes**. A few files are someone else's and keep their own terms: Ted Fried's MC68000 core and
test suite (MIT), Texas Instruments' start-up and board set-up routines (BSD), libbbb's SPI, timer and I2C setup (Apache 2.0), Georges Menie's XMODEM receiver (BSD), and Weicker's
Dhrystone 2.1 source. They are listed, with what is not here at all, in [docs/third-party.md](docs/third-party.md). **No Kickstart ROM or other Commodore/Cloanto material is
distributed**: Buffee copies the ROM out of your own Amiga.
