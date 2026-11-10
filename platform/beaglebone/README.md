# BeagleBone Black (AM335x, Cortex-A8) — the benchmarking host

The BeagleBone Black has the same core as Buffee's OSD335x (Cortex-A8, 1 GHz), so it is where cycle counts are measured: QEMU says nothing about
timing. The translator runs on it from U-Boot; the Cortex-A8 instruction set is also exercised on QEMU's core model
(`make a8-test`). What the core costs, as measured with it, is in [docs/design/CORTEX_A8.md](../../docs/design/CORTEX_A8.md).

## Build

    make bbb-bbb_dhrystone            # -> build/default/bbb/bbb_dhrystone.bin, a flat image for 0x82000000
    make bbb-<any test>               # tests/bbb/* are written for this board; see tests/README.md
    make a8-test                      # the Cortex-A8 ISA check on QEMU

Everything is built with `-mcpu=cortex-a8` (QEMU's A15 accepts a `udiv` the board cannot run).

## What the image does

U-Boot loads it at `0x82000000` ($loadaddr) and starts it with `go 0x82000000` in SVC mode, little-endian, interrupts off.

1. `boot.S` saves what U-Boot needs back: r4-r11, sp, lr, CPSR and SCTLR / ACTLR / TTBR0 / TTBCR / DACR / VBAR.
2. Own vectors (VBAR), per-mode stacks, `.bss` zeroed (U-Boot does not).
3. `bbb_init()`: the **RAM log** goes live; 1 MB-section page tables (peripherals device-type, DDR normal write-back, the log's
   section uncached) and the caches on; the PMU cycle counter and four event counters started; the **watchdog (WDT1) armed**,
   120 s by default (`-DBBB_WDT_SECONDS=n`).
4. `main()`. `printf` goes to the RAM log and, unless built with `-DBBB_UART=0`, to UART0 (U-Boot leaves it at 115200 8N1).
5. **LEDs**: USR0 on while running, USR3 on after a clean exit, USR2 after a caught fault.
6. **Clean exit** (`qemu_exit()` / `bbb_exit(code)` / `main` returning): log marked done, watchdog stopped, data cache cleaned,
   MMU and caches off, U-Boot's CP15 state and registers put back, `bx lr`: **`go` returns to U-Boot** with the exit code in r0.
7. **Hang or crash**: the watchdog resets the board (a global warm reset: DDR keeps its contents). A fault (undefined instruction,
   prefetch / data abort) is caught by `boot.S`, reported into the log, and also returns to U-Boot through the clean path.

So a run can never need a button press: it ends by returning, by the fault path, or by the watchdog.

## The RAM log

At `0x87F00000` (uncached DDR, 64 KB, see `bbb.h`): a 32-byte header then ASCII text.

    +0  magic   0x474C4A50  ('P' 'J' 'L' 'G')
    +4  length  bytes of text
    +8  state   1 = started (still 1 at the next boot: the run hung and the watchdog reset it), 2 = done, 3 = fault
    +12 code    exit code (done / fault)
    +16 cycles  PMU cycle counter at exit
    +32 text...

From a U-Boot prompt: `md.b 0x87F00020 0x400` to look at it, or save it: `fatwrite mmc 0:1 0x87F00000 pjit.log 0x10000` (FAT) /
`tftpput 0x87F00000 0x10000 pjit.log` (network). Read the header's state first. It survives the watchdog reset, so the *next*
boot's script can save a hung run's log before it does anything else.


## Running an image

The BeagleBone runs the Debian image from beagleboard.org on its SD card; the FAT partition (`mmcblk0p1`, `/boot/firmware` under Linux) is readable from U-Boot.

* **`tools/bbb/run.py build/default/bbb/bbb_dhrystone.bin`** does the whole cycle from the development machine: it copies the image over ssh (`Host bbb` in
  `~/.ssh/config`; the board needs passwordless `sudo` for `reboot`), reboots into U-Boot, loads and starts it and prints the console. `BBB_SERIAL`
  names the serial adapter, `BBB_HOST` the ssh host.
* By hand: `sudo cp pjit.bin /boot/firmware/` and `reboot`; on the UART (115200 8N1) press a key in U-Boot's autoboot countdown, then
  `fatload mmc 0:1 0x82000000 pjit.bin` and `go 0x82000000`. The image prints on UART0 and returns to U-Boot (`rc = 0x0`); `boot` goes back to Linux.

Network from the development machine: eth0 has a static 169.254.144.87/16 (drop-in `/etc/systemd/network/eth0.network.d/static.conf`; DHCP still on) and
`arp_ignore=1` (`/etc/sysctl.d/90-arp.conf`) so the USB gadget's `usb0` does not answer ARP for it. `tools/study/native_bogomips.c` is the native baseline;
build it on the board with gcc.

## Known gaps

* The L2 cache is left as U-Boot configured it (ACTLR is printed at boot); enabling it here (ACTLR bit 1) is a one-line change once confirmed.
* The benchmarks time the program with the PMU cycle counter; the 68K Dhrystone also reads DMTIMER1_1MS (24 MHz), which `bbb_init` turns on.
* Timings move 5-10% with code alignment: compare builds over several layouts (`tools/bbb/scan.sh`).
