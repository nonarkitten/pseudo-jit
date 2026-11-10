# Dhrystone 2.1 for the PJIT tests

`dhry.h`, `dhry_1.c` and `dhry_2.c` are the C version of **Reinhold P. Weicker's Dhrystone, version 2.1** (Siemens, 1988), unmodified. The runtime around them
(`buffee_rt.c`, `include/`, `link.ld`) and the builder (`tools/data/dhrystone.py`) are ours.

## Where the sources come from

Weicker distributed Dhrystone 2.1 on a floppy disk and through Usenet, asking only that results be sent to him and to Rick Richardson, with the conditions of the
measurement (his "ground rules", in `RATIONALE` of the distribution). The files here are Rick Richardson's 1988 posting of 2.1 with his instrumentation taken out again by
his own `pure2_1.dif` ("to turn what I'm posting as 2.1 back into what Reinhold sent me as 2.1"), so they are what Weicker wrote. The posting was taken from
<https://github.com/Keith-S-Thompson/dhrystone> (`v2.1/`, extracted from the alt.sources posting of 1990-02-14 from TU Vienna).

| file | md5 |
|---|---|
| `dhry.h` | `98014e0d9839ac172d31d3b94f65df39` |
| `dhry_1.c` | `49b957a84aa4858d100b4cece94b48e7` |
| `dhry_2.c` | `280a3ae7a2dd84622fb9ca8781055269` |

The sources carry no licence text. They have been redistributed freely for decades (Weicker's own floppy, Usenet, netlib, U-Boot, the Linux kernel, LLVM's test suite).
Treat them as freely distributable for benchmarking, with the author's request above; they are not covered by this project's licence.

## What is around them

* `buffee_rt.c` — `_start`, `malloc`, `strcpy`, `strcmp`, `printf`, `scanf`, `times` and a 32-bit `__divsi3`. `times()` returns 0, so the program takes its "measured time too small"
  branch and never reaches the floating-point reporting code; the harness (`tests/qemu/smoke_dhrystone.c`) times the run itself, from outside. Its `scanf` hands over the run
  count the harness left in a cell, and `printf` returns at once when another cell says "quiet". The soft-float helpers the unreachable code names are stubs.
* `include/` — the three system headers `dhry.h` includes (`stdio.h`, `sys/types.h`, `sys/times.h`), K&R-style as the 1988 program expects.
* `link.ld` — a flat image at guest address `0x1000`.

## How it was built (Weicker's rules ask for this to be stated with any result)

`m68k-elf-gcc` 16.2.0, `-m68000 -O2 -std=gnu89 -fno-builtin` (so `strcpy` and `strcmp` are calls, as in the original), without the `register` attribute. `strcpy` and `strcmp` are
byte loops (`move.b (a1)+,d1 ; move.b d1,(a0)+ ; bne`), `__divsi3` uses `DIVU.W` when the divisor fits in 16 bits. One iteration of the loop runs 482 68000 instructions.

Rebuild with `python3 tools/data/dhrystone.py` (needs `m68k-elf-gcc`); the result, `tests/data/dhrystone_image.h`, is checked in.
