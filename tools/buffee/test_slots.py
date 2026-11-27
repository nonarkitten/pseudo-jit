#!/usr/bin/env python3
"""Tests the two PJIT slots of a Buffee's flash on the board: which bundle boots, and the fallback when one is damaged.

  tools/buffee/test_slots.py [--only CASE,...] [--dry-run]

The update slot is written from the Amiga, which this test does not need: it starts the bootloader from the debugger
(as buffee_jtag.py does) and has the debugger call the bootloader's own functions (PJITStore, StoreWrite, StoreMarkBad,
WriteSPIBlock) with the core halted in the bootloader's request loop, so every slot state is made by the code that makes
it in use. Each case then resets the board, follows its boot from the flash and reads what the bootloader handed the
payload (the slot it started, each slot's state and sequence number, bootinfo_t in bootloader/src/payload.c).

  newer-update   the update written after the backup boots
  crc            a cleared word in the update's data: its CRC fails, it is marked bad and the backup boots; the next boot
                 goes straight to the backup
  rewrite        writing the update again clears the mark: the update boots
  unpack         an update with a good CRC that is not a bundle: marked bad, the backup boots
  newer-backup   the backup written after a good update boots
  both-bad       both slots marked bad: no PJIT, the bootloader stops at its menu
  restore        the backup written again and the update erased: the backup boots (the state of an installed board)

It writes both PJIT slots (the backup is rewritten from its own contents), never the bootloader, the ROM copy or the
EEPROM. The symbols come from the ELF of the bootloader it starts (make bootloader).
"""

import argparse
import os
import struct
import subprocess
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import buffee_jtag as bj                                                # noqa: E402

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
ELF = os.path.join(ROOT, "build", "bootloader", "asn", "bootloader.elf")
BOOTINFO = 0x9F5FFF00                                                   # bootinfo_t (payload.c)
SCRATCH = bj.REQ_PAGE + 0x800                                           # free words after the request page
PJIT_OFF, PJIT_UPD, PJIT_MAX, MAGIC_PJIT, BAD_WORD = 0x20000, 0x90000, 0x70000, 0x544A4A50, 0x10
STATE = {0: "empty", 1: "good", 2: "bad", 0xFF: "none"}
SETTLE = 10                                                             # seconds from the reset to the one look at the boot
CASES = ["newer-update", "crc", "rewrite", "unpack", "newer-backup", "both-bad", "restore"]


def symbols(elf):
    nm = os.environ.get("CROSS", "arm-none-eabi-") + "nm"
    out = subprocess.run([nm, elf], capture_output=True, text=True, check=True).stdout
    return {p[2]: int(p[0], 16) for p in (l.split() for l in out.splitlines()) if len(p) == 3 and p[1] in "Tt"}


class Caller:
    """Calls a function of the bootloader with the core halted in its request loop, and puts every register back afterwards. The function returns
    to a hardware breakpoint on JTAGService's entry, which nothing else reaches while the loop runs."""

    REGS = ["r%d" % i for i in range(13)] + ["sp", "lr", "pc", "cpsr"]

    def __init__(self, board, sym):
        self.b, self.o, self.sym = board, board.ocd, sym
        self.stop = sym["JTAGService"]

    def reg(self, name, value=None):
        if value is not None:
            self.o.cmd("reg %s 0x%x" % (name, value & 0xFFFFFFFF))
            return value
        r = self.o.cmd("reg %s" % name)
        return int(r.split(":")[-1].split()[0], 0)

    def call(self, name, *args, limit=120):
        o = self.o
        o.halt()
        saved = {r: self.reg(r) for r in self.REGS}
        sp = (saved["sp"] - 64) & ~7
        if len(args) > 4:
            o.write_words(sp, list(args[4:]))
        for i, v in enumerate(args[:4]):
            self.reg("r%d" % i, v)
        self.reg("sp", sp)
        self.reg("lr", self.stop)
        self.reg("pc", self.sym[name])
        o.cmd("bp 0x%x 4 hw" % self.stop)
        try:
            o.resume()
            end = time.time() + limit
            while True:
                time.sleep(0.5)
                o.halt()
                self.b.log.show()
                if self.reg("pc") == self.stop:
                    break
                if time.time() > end:
                    raise IOError("%s did not return within %d s" % (name, limit))
                o.resume()
            rc = self.reg("r0")
        finally:
            o.cmd("rbp 0x%x" % self.stop)
        for r in self.REGS:
            self.reg(r, saved[r])
        o.resume()
        return rc - (1 << 32) if rc & 0x80000000 else rc

    def peek(self, addr, n):
        self.o.halt()
        try:
            return self.o.words(addr, n)
        finally:
            self.o.resume()

    def poke(self, addr, vals):
        self.o.halt()
        try:
            self.o.write_words(addr, vals)
        finally:
            self.o.resume()


class Test:
    def __init__(self, a):
        self.a, self.sym, self.failures = a, symbols(a.elf), []
        self.b = self.c = None

    def check(self, what, ok):
        print("%s %s" % ("  ok  " if ok else "  FAIL", what), flush=True)
        if not ok:
            self.failures.append(what)

    def bootloader(self):
        """The bootloader from the debugger, waiting for requests, with a Caller on it."""
        self.b = bj.Board(self.a)
        self.b.start_bootloader(self.a.bootloader)
        w = self.b.wait_ready(self.a.ready_timeout)
        self.c = Caller(self.b, self.sym)
        return w

    def status(self):
        self.b.do("status")

    def backup_to_ram(self):
        """The backup slot's bundle into RX_BUF (the request page's buf), read by StoreLoad (which ignores the bad mark); returns its length."""
        buf = self.c.peek(bj.REQ_PAGE + 36, 1)[0]
        self.c.poke(SCRATCH, [0])
        rc = self.c.call("StoreLoad", PJIT_OFF, PJIT_MAX, MAGIC_PJIT, buf, SCRATCH, 0)
        n = self.c.peek(SCRATCH, 1)[0]
        if rc != 0 or not n:
            sys.exit("the backup slot does not load (rc %d): install a bundle first (buffee_jtag.py install)" % rc)
        return buf, n

    def store(self, slot, buf, n):
        rc = self.c.call("PJITStore", slot, buf, n)
        if rc:
            sys.exit("PJITStore(%d) failed: %d" % (slot, rc))

    def flash_word(self, off):
        self.c.call("ReadSPIBlock", off, SCRATCH, 4)
        return self.c.peek(SCRATCH, 1)[0]

    def reboot(self):
        """Leaves the request loop through SRST and looks at the boot from the flash; returns its log and bootinfo_t (slot, states, seqs)."""
        self.b.ocd.close()
        return self.reset_only()

    def reset_only(self):
        """A reset through SRST, then the boot from the flash, read SETTLE seconds later through the AHB-AP with the core running (a running PJIT often
        does not take a halt; it cleans the caches when it starts): the boot's report (quiet: only what went wrong and 'Starting PJIT') and bootinfo_t."""
        o = bj.OpenOCD(self.a)
        try:
            o.cmd("adapter assert srst")
            time.sleep(0.1)
            o.cmd("adapter deassert srst")
        finally:
            o.close()
        print("\nReset through SRST: the boot from the flash decides (read %d s later)" % SETTLE, flush=True)
        time.sleep(SETTLE)
        o = bj.OpenOCD(self.a)
        try:
            log = bj.find_log_ahb(o)
            text = log.text(ahb=True) if log else ""
            if log and not any(m in text for m in bj.BOOT_ENDS):         # the bootloader at its menu: its last lines may still be in its data cache
                o.halt()
                try:
                    log.seen = 0
                    text = log.text()
                finally:
                    o.resume()
            sys.stdout.write(text)
            w = o.words(BOOTINFO, 12, ahb=True)
        finally:
            o.close()
        return text, self.bootinfo(w)

    def bootinfo(self, w):
        b = struct.pack("<12I", *w)
        if b[:4] != b"BOOT":
            print("bootinfo: none (%s)" % " ".join("%08X" % x for x in w[:4]), flush=True)
            return None
        slot, s0, s1 = b[36], b[37], b[38]
        seq0, seq1 = struct.unpack("<2I", b[40:48])
        print("bootinfo: started the %s slot; backup %s #%d, update %s #%d" % ({0: "backup", 1: "update"}.get(slot, "no"), STATE.get(s0, s0), seq0,
                                                                             STATE.get(s1, s1), seq1), flush=True)
        return dict(slot=slot, state=(s0, s1), seq=(seq0, seq1))

    def clear_bootinfo(self):
        """bootinfo_t from an earlier boot must not pass for this one's."""
        self.c.poke(BOOTINFO, [0])

    # ---- the cases ------------------------------------------------------------------------------------------------------------------------

    def case_newer_update(self):
        self.bootloader()
        buf, n = self.backup_to_ram()
        self.store(1, buf, n)
        self.status()
        self.clear_bootinfo()
        log, bi = self.reboot()
        self.check("newer-update: the update boots", bi and bi["slot"] == 1 and bi["state"] == (1, 1) and bi["seq"][1] > bi["seq"][0])
        self.check("newer-update: PJIT starts", "Starting PJIT" in log)

    def case_crc(self):
        self.bootloader()
        off = PJIT_UPD + 0x100 + 0x8000
        while self.flash_word(off) == 0:
            off += 4
        self.poke_zero(off)
        self.check("crc: the word at %06X reads 0" % off, self.flash_word(off) == 0)
        self.status()
        self.clear_bootinfo()
        log, bi = self.reboot()
        self.check("crc: the update is reported damaged (CRC) and marked bad", "update slot is damaged (CRC): marked bad" in log)
        self.check("crc: the backup boots", bi and bi["slot"] == 0 and bi["state"] == (1, 2))
        self.check("crc: PJIT starts", "Starting PJIT" in log)
        log, bi = self.reset_only()
        self.check("crc, next boot: no second report", "update slot is damaged" not in log)
        self.check("crc, next boot: the backup boots", bi and bi["slot"] == 0 and bi["state"] == (1, 2))

    def poke_zero(self, off):
        self.c.poke(SCRATCH, [0])
        self.c.call("WriteSPIBlock", off, SCRATCH, 4)

    def case_rewrite(self):
        self.bootloader()
        buf, n = self.backup_to_ram()
        self.store(1, buf, n)
        self.check("rewrite: the bad mark is gone", self.flash_word(PJIT_UPD + BAD_WORD) == 0xFFFFFFFF)
        self.status()
        self.clear_bootinfo()
        log, bi = self.reboot()
        self.check("rewrite: the update boots", bi and bi["slot"] == 1 and bi["state"] == (1, 1))

    def case_unpack(self):
        w = self.bootloader()
        buf, n = self.backup_to_ram()
        seq = self.c.call("PJITNextSeq")
        self.c.poke(buf, [0x58585858])                                  # 'XXXX' for 'LZTB': written by StoreWrite itself, so the CRC holds
        rc = self.c.call("StoreWrite", PJIT_UPD, PJIT_MAX, MAGIC_PJIT, buf, n, seq)
        self.check("unpack: StoreWrite of a non-bundle as #%d" % seq, rc == 0)
        self.status()
        self.clear_bootinfo()
        log, bi = self.reboot()
        self.check("unpack: the update is reported as not unpacking and marked bad", "update slot is damaged (it does not unpack): marked bad" in log)
        self.check("unpack: the backup boots", bi and bi["slot"] == 0 and bi["state"] == (1, 2))
        self.check("unpack: PJIT starts", "Starting PJIT" in log)

    def case_newer_backup(self):
        self.bootloader()
        buf, n = self.backup_to_ram()
        self.store(1, buf, n)
        self.store(0, buf, n)
        self.status()
        self.clear_bootinfo()
        log, bi = self.reboot()
        self.check("newer-backup: the backup boots over a good update", bi and bi["slot"] == 0 and bi["state"] == (1, 1) and bi["seq"][0] > bi["seq"][1])

    def case_both_bad(self):
        self.bootloader()
        self.c.call("StoreMarkBad", PJIT_OFF)
        self.c.call("StoreMarkBad", PJIT_UPD)
        self.status()
        self.clear_bootinfo()
        log, bi = self.reboot()
        self.check("both-bad: 'no good bundle' and the menu", "no good bundle in the flash" in log and "Starting PJIT" not in log and "Ready" in log)
        self.check("both-bad: nothing handed to a payload", bi is None)

    def case_restore(self):
        self.bootloader()
        buf, n = self.backup_to_ram()
        self.store(0, buf, n)
        self.c.call("PJITEraseSlot", 1)
        self.status()
        self.clear_bootinfo()
        log, bi = self.reboot()
        self.check("restore: the backup boots, the update is empty", bi and bi["slot"] == 0 and bi["state"] == (1, 0))
        self.check("restore: PJIT starts", "Starting PJIT" in log)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0], formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__.split("\n", 1)[1])
    ap.add_argument("--only", help="the cases to run, ',' between them (default: all, in the order above)")
    ap.add_argument("--elf", default=ELF)
    ap.add_argument("--bootloader", help="default: the .bin next to the ELF")
    ap.add_argument("--dry-run", action="store_true", help="start the bootloader and call FlashStatus only (writes nothing)")
    ap.add_argument("--interface", default="interface/jlink.cfg")
    ap.add_argument("--adapter-cmd", action="append")
    ap.add_argument("--speed", type=int, default=100)
    ap.add_argument("--openocd")
    ap.add_argument("--ready-timeout", type=int, default=90)
    ap.add_argument("--verbose", action="store_true")
    a = ap.parse_args()
    a.bootloader = a.bootloader or os.path.splitext(a.elf)[0] + ".bin"
    t = Test(a)
    if a.dry_run:
        t.bootloader()
        rc = t.c.call("FlashStatus")
        print("\nFlashStatus returned %d through the debugger; the board waits in the bootloader" % rc)
        t.b.ocd.close()
        return 0
    cases = a.only.split(",") if a.only else CASES
    for name in cases:
        if name not in CASES:
            ap.error("no case %s (%s)" % (name, ", ".join(CASES)))
    try:
        for name in cases:
            print("\n==== %s ====" % name, flush=True)
            getattr(t, "case_" + name.replace("-", "_"))()
    finally:
        if t.b:
            t.b.ocd.close()
    print("\n%s" % ("FAILED: " + "; ".join(t.failures) if t.failures else "All %d cases passed" % len(cases)))
    return 1 if t.failures else 0


if __name__ == "__main__":
    sys.exit(main())
