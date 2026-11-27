#!/usr/bin/env python3
"""Installs and updates Buffee's firmware through the JTAG header alone: no serial cable, no menu.

  buffee_jtag.py install [--bootloader FILE] [--bundle FILE] [--set 'ITEM=VALUE;...'] [--copy-rom]
  buffee_jtag.py status          the flash slots and the settings
  buffee_jtag.py settings 'ITEM=VALUE;...'
  buffee_jtag.py erase-update    empties the update slot (the backup bundle boots)
  buffee_jtag.py log             what the bootloader printed in this boot, also under PJIT (no reset)
  buffee_jtag.py wipe            erases all of the flash and the EEPROM: the board is then as one never programmed

It needs Python 3 and OpenOCD (0.12 or later) on the path, and a JTAG adapter OpenOCD drives (a J-Link by default;
--interface names another OpenOCD interface file). Everything but `log` resets the board and starts the bootloader
given with --bootloader in the AM335x's on-chip RAM, from the debugger, so a board with an empty or broken flash can be
installed the same way as a working one. That bootloader brings the board up (its boot report is printed here) and
then waits for requests in the DRAM (bootloader/src/request.h): this script loads an image into the DRAM, names it in
the request page, and the bootloader checks its CRC and writes it into the flash, as menu 3 would. At the end the
board is reset and boots from the flash.

`install` writes the bundle into the backup PJIT slot and then the bootloader into the start of the flash (only when
the flash does not hold the same one already). --set changes settings as on the settings page (menu 4), by item
number: '1=flash;4=200000/8' sets MapROM to the flash and adds 8 MB of fast RAM at $200000; '11=' restores the
defaults. --copy-rom (with install or settings) stores a copy of the Amiga's Kickstart in the flash (for MapROM = flash).

`wipe` erases the bootloader, both bundles, the ROM copy and the settings, and checks that the flash and the EEPROM read
back blank. The board then does nothing at power-on until it is installed again (from here or over the serial port). It
asks for the word `wipe` first, unless --yes is given.

The images default to the files next to this script (a release), else build/firmware/ in the repository.
"""

import argparse
import os
import shutil
import socket
import subprocess
import sys
import tempfile
import time
import zlib

LOAD_ADDR = 0x402F0400          # where the AM335x ROM puts a boot image, and where the bootloader is linked
LOG_PTR = 0x402F0420            # 'BLOG' and the address of the console log (bootloader/clib/init.s)
LOG_MAGIC = 0x474F4C42
LOG_SIZE = 0x2000
JTAG_MAGIC = 0x4A544147         # r0 at the entry: started by a debugger
REQ_PAGE = 0x9F5F0000           # bootloader/src/request.h
REQ_MAGIC = 0x51524642
REQ = dict(store_pjit=1, store_rom=2, store_boot=3, copy_rom=4, settings=5, erase_update=6, reboot=7, status=8, wipe=10)
REQ_READY = 1
ERRORS = {-1: "the slot is empty", -2: "the flash read back with a bad CRC", -3: "too big for its slot", -4: "the flash did not verify",
          -10: "the bootloader does not know that request", -11: "the data is outside the free DRAM", -12: "the data arrived with a bad CRC",
          -13: "not the kind of image the request names", -14: "the settings did not parse or did not save", -15: "the Amiga's bus is not running",
          -16: "the flash or the EEPROM did not read back blank"}
TARGET = "am335x.cpu"
AHB = "am335x.ahb"              # the DAP's AHB-AP on the L3: reads the DRAM and the OCMC RAM without halting the core (not the MPU's own SRAM at 0x402F0400, nor through the caches)
OCMC = 0x40300000               # the bootloader's .bss, with its console log, starts in the OCMC RAM

here = os.path.dirname(os.path.abspath(__file__))


def default_image(name):
    for d in (here, os.path.join(os.path.dirname(os.path.dirname(here)), "build", "firmware")):
        p = os.path.join(d, name)
        if os.path.exists(p):
            return p
    return os.path.join(here, name)


class OpenOCD:
    """An OpenOCD process and its Tcl port: cmd() runs one command and returns its result."""

    def __init__(self, a):
        exe = a.openocd or shutil.which("openocd")
        if not exe:
            sys.exit("OpenOCD is not on the path (macOS: brew install openocd; Debian/Ubuntu: apt install openocd; Windows: see docs/getting-started-jtag.md)")
        s = socket.socket()
        s.bind(("127.0.0.1", 0))
        self.port = s.getsockname()[1]
        s.close()
        argv = [exe, "-c", "gdb_port disabled", "-c", "telnet_port disabled", "-c", "tcl_port %d" % self.port, "-f", a.interface]
        argv += ["-c", "transport select jtag", "-c", "adapter speed %d" % a.speed]
        for c in a.adapter_cmd or []:
            argv += ["-c", c]
        argv += ["-f", "target/am335x.cfg", "-c", "target create %s mem_ap -dap am335x.dap -ap-num 0" % AHB, "-c", "init", "-c", "targets %s" % TARGET,
                 "-c", "reset_config srst_only srst_push_pull"]     # SRST configured at the connect holds the chip in reset
        self.outfile = tempfile.TemporaryFile()
        self.p = subprocess.Popen(argv, stdout=None if a.verbose else self.outfile, stderr=subprocess.STDOUT)
        self.sock = None
        end = time.time() + 15
        while time.time() < end and self.p.poll() is None:
            try:
                self.sock = socket.create_connection(("127.0.0.1", self.port), timeout=1)
                break
            except OSError:
                time.sleep(0.2)
        if not self.sock:
            self.close()
            self.outfile.seek(0)
            out = self.outfile.read().decode(errors="replace")
            sys.exit("OpenOCD did not start:\n" + out + "\nIs the adapter plugged in, and the board powered (the Amiga on)?")
        self.sock.settimeout(120)

    def cmd(self, c):
        self.sock.sendall(c.encode() + b"\x1a")
        buf = b""
        while not buf.endswith(b"\x1a"):
            d = self.sock.recv(65536)
            if not d:
                raise IOError("OpenOCD went away")
            buf += d
        return buf[:-1].decode(errors="replace")

    def words(self, addr, n, phys=False, ahb=False):
        r = self.cmd("%sread_memory 0x%x 32 %d%s" % (AHB + " " if ahb else "", addr, n, " phys" if phys and not ahb else ""))
        try:
            return [int(x, 0) for x in r.split()]
        except ValueError:
            raise IOError("reading %08X: %s" % (addr, r.strip()))

    def bytes_(self, addr, n, phys=False):
        out = b""
        while n:
            k = min(n, 4096)
            r = self.cmd("read_memory 0x%x 8 %d%s" % (addr, k, " phys" if phys else ""))
            try:
                out += bytes(int(x, 0) for x in r.split())
            except ValueError:
                raise IOError("reading %08X: %s" % (addr, r.strip()))
            addr += k
            n -= k
        return out

    def write_words(self, addr, vals):
        r = self.cmd("write_memory 0x%x 32 {%s}" % (addr, " ".join("0x%x" % (v & 0xFFFFFFFF) for v in vals)))
        if r.strip():
            raise IOError("writing %08X: %s" % (addr, r.strip()))

    def halted(self):
        return self.cmd("%s curstate" % TARGET).strip() == "halted"

    def halt(self):
        self.cmd("halt 2000")
        if not self.halted():
            raise IOError("the ARM does not halt")

    def resume(self, pc=None):
        self.cmd("resume" if pc is None else "resume 0x%x" % pc)

    def close(self):
        try:
            if self.sock:
                self.sock.sendall(b"shutdown\x1a")
                self.sock.close()
        except OSError:
            pass
        try:
            self.p.wait(5)
        except Exception:
            self.p.kill()
            self.p.wait(5)


class Log:
    """The bootloader's console log (log_t: magic, size, head, buf): prints what is new since the last look."""

    def __init__(self, ocd, phys=False, addr=None):
        self.ocd, self.phys, self.addr, self.seen = ocd, phys, addr, 0

    def find(self):
        magic, addr = self.ocd.words(LOG_PTR, 2, self.phys)
        if magic != LOG_MAGIC:
            return False
        if self.ocd.words(addr, 1, self.phys)[0] != LOG_MAGIC:
            return False
        self.addr = addr
        return True

    def text(self, ahb=False, peek=False):
        """What is new since the last look; ahb: read through the AHB-AP, the core running (current only once the caches are cleaned, as PJIT's start
        does); peek: do not count it as seen."""
        if self.addr is None and not self.find():
            return ""
        _, size, head = self.ocd.words(self.addr, 3, self.phys, ahb)
        if head < self.seen:                                    # the bootloader started again
            self.seen = 0
        start = max(self.seen, head - size)
        out = b""
        while start < head:
            i = start % size
            k = min(head - start, size - i)
            a0, a1 = (i & ~3), (i + k + 3) & ~3                 # whole words: one JTAG transaction per word, not per byte
            w = self.ocd.words(self.addr + 12 + a0, (a1 - a0) // 4, self.phys, ahb)
            out += b"".join(x.to_bytes(4, "little") for x in w)[i - a0:i - a0 + k]
            start += k
        if not peek:
            self.seen = head
        return out.decode("latin1").replace("\r", "")

    def show(self):
        t = self.text()
        if t:
            sys.stdout.write(t)
            sys.stdout.flush()
        return t


def check_bootloader(data):
    if len(data) < 64 or data[3] != 0xEA or len(data) + 8 > 0x1C000:
        sys.exit("that does not look like a Buffee bootloader image (it starts with a branch and is under 112 KB)")


def check_bundle(data):
    if data[:4] != b"LZTB":
        sys.exit("that does not look like a PJIT bundle (no LZTB header)")
    if len(data) > 0x70000 - 0x100:
        sys.exit("the bundle is bigger than a PJIT slot (448 KB)")


class Board:
    def __init__(self, a):
        self.a = a
        self.ocd = OpenOCD(a)
        self.log = Log(self.ocd)
        self.seq = 0

    def start_bootloader(self, image):
        """Resets the board, stops the ARM as soon as it lets the debugger in (the secure part of the ROM does not; the bootloader in the flash may have
        started), switches its caches and MMU off, puts the bootloader into the on-chip RAM and starts it in JTAG mode. The bootloader invalidates
        the caches before it switches them on, so nothing of what ran before survives."""
        o = self.ocd
        print("Resetting the board through the JTAG header", flush=True)
        o.cmd("adapter assert srst")
        time.sleep(0.1)
        o.cmd("adapter deassert srst")
        end = time.time() + 15
        while time.time() < end:
            o.cmd("catch {%s arp_examine}" % TARGET)
            o.cmd("catch {halt 300}")
            if o.halted():
                break
            time.sleep(0.1)
        else:
            sys.exit("the ARM does not stop after a reset")
        o.cmd("catch disable_watchdog")
        sctlr = int(o.cmd("%s arm mrc 15 0 1 0 0" % TARGET).strip(), 0)
        o.cmd("%s arm mcr 15 0 1 0 0 0x%x" % (TARGET, sctlr & ~0x1805))      # MMU, data and instruction caches, branch prediction off
        o.cmd("catch {%s cache l1 i inval_all}" % TARGET)
        t0 = time.time()
        r = o.cmd("load_image {%s} 0x%x bin" % (os.path.abspath(image).replace("\\", "/"), LOAD_ADDR))
        print("(the bootloader into the on-chip RAM in %.1f s)" % (time.time() - t0), flush=True)
        if "downloaded" not in r:
            sys.exit("loading the bootloader into the on-chip RAM failed: %s" % r.strip())
        want = open(image, "rb").read()
        if b"".join(v.to_bytes(4, "little") for v in o.words(LOAD_ADDR, 64)) != want[:256]:
            sys.exit("the bootloader did not arrive in the on-chip RAM intact")
        o.cmd("reg r0 0x%x" % JTAG_MAGIC)
        o.cmd("reg r1 0")
        o.cmd("reg cpsr 0x1d3")
        o.resume(LOAD_ADDR)
        print("Bootloader started from the debugger; its report:\n", flush=True)

    def wait_ready(self, limit):
        """Polls the log until the bootloader says it waits, then the request page until it is ready."""
        end = time.time() + limit
        page_ok = False
        while time.time() < end:
            time.sleep(0.5)
            self.ocd.halt()
            try:
                t = self.log.show()
                if "JTAG: waiting for requests" in t:
                    page_ok = True
                if page_ok:
                    w = self.ocd.words(REQ_PAGE, 9)
                    if w[0] == REQ_MAGIC and w[8] == REQ_READY and w[5] == w[6]:
                        self.seq = w[5]
                        return w
            except IOError:
                pass                                            # a read in the first moments after the start can fail: look again
            finally:
                self.ocd.resume()
        sys.exit("\nthe bootloader did not get ready within %d s (the log above says how far it got)" % limit)

    def request(self, name, data=None, text=None, limit=180):
        o = self.ocd
        o.halt()
        try:
            addr = length = crc = 0
            if data is not None:
                w = o.words(REQ_PAGE + 36, 2)                   # buf, buf_max
                addr, length = w[0], len(data)
                if length > w[1]:
                    sys.exit("%d bytes do not fit the bootloader's buffer" % length)
                with tempfile.NamedTemporaryFile(suffix=".bin", delete=False) as f:
                    f.write(data)
                t0 = time.time()
                try:
                    r = o.cmd("load_image {%s} 0x%x bin" % (f.name.replace("\\", "/"), addr))
                finally:
                    os.unlink(f.name)
                if "downloaded" not in r:
                    sys.exit("loading into the DRAM failed: %s" % r.strip())
                print("(%d bytes into the DRAM in %.1f s)" % (length, time.time() - t0), flush=True)
                crc = zlib.crc32(data) & 0xFFFFFFFF
            if text is not None:
                b = text.encode("ascii") + b"\0"
                if len(b) > 192:
                    sys.exit("the settings text is longer than 191 characters")
                b += b"\0" * (-len(b) % 4)
                o.write_words(REQ_PAGE + 64, [int.from_bytes(b[i:i + 4], "little") for i in range(0, len(b), 4)])
            self.seq += 1
            o.write_words(REQ_PAGE + 4, [REQ[name], addr, length, crc])
            o.write_words(REQ_PAGE + 20, [self.seq])
        finally:
            o.resume()
        if name == "reboot":
            return 0
        end = time.time() + limit
        while time.time() < end:
            time.sleep(0.5)
            o.halt()
            try:
                self.log.show()
                w = o.words(REQ_PAGE + 20, 3)
                if w[1] == self.seq:
                    rc = w[2] - (1 << 32) if w[2] & 0x80000000 else w[2]
                    return rc
            finally:
                o.resume()
        sys.exit("\nno answer to the request within %d s" % limit)

    def do(self, name, **kw):
        rc = self.request(name, **kw)
        if rc:
            print("\n%s failed: %s (%d)" % (name.replace("_", " "), ERRORS.get(rc, "error"), rc))
        return rc

    def reboot(self, why="it boots from the flash"):
        print("\nResetting the board: %s" % why, flush=True)
        self.request("reboot")
        time.sleep(0.5)
        self.ocd.close()


def find_log(ocd):
    """The console log of the bootloader that ran last, with the core halted; None if there is none (yet)."""
    for phys in (False, True):                                  # PJIT's page tables may not map the on-chip RAM
        log = Log(ocd, phys)
        try:
            if log.find():
                return log
        except IOError:
            pass
    return None


def find_log_ahb(ocd):
    """The console log through the AHB-AP, the core running: its header (magic, size) is searched for at the start of the OCMC RAM. None if not there."""
    for base in range(OCMC, OCMC + 0x2000, 0x400):
        try:
            w = ocd.words(base, 0x100, ahb=True)
        except IOError:
            return None
        for i in range(len(w) - 1):
            if w[i] == LOG_MAGIC and w[i + 1] == LOG_SIZE:
                return Log(ocd, addr=base + 4 * i)
    return None


def cmd_log(a):
    """Prints the bootloader's console log of this boot without resetting; it is kept while PJIT runs, which prints to the UART only."""
    ocd = OpenOCD(a)
    try:
        try:
            ocd.halt()
        except IOError:                                         # a running PJIT often does not take the halt: read past the core (PJIT cleaned the caches when it started)
            log = find_log_ahb(ocd)
            if not log:
                sys.exit("the ARM does not stop for the debugger and no log was found through the AHB-AP")
            sys.stdout.write(log.text(ahb=True))
            return
        try:
            log = find_log(ocd)
            if not log:
                sys.exit("no bootloader log in the on-chip RAM (a bootloader older than the JTAG installer, or the ROM's UART boot)")
            sys.stdout.write(log.text())
        finally:
            ocd.resume()
    finally:
        ocd.close()


BOOT_ENDS = ("Starting PJIT", "stopped at the menu", "Ready\n] ")     # the last: the menu, whatever stopped the boot


FIRST_LOOK = 5                  # seconds after the reset before the first look at the boot


def follow_boot(a, limit=90):
    """Prints the boot from the flash (a first boot tests all of the DDR for about 15 s), until PJIT starts or the bootloader stops at its menu. Each look
    reads the log through the AHB-AP first, with the core running: a running PJIT often does not take a halt, and it has cleaned the caches when it starts,
    so that read is current from then on. While the bootloader still runs (or waits at its menu) the log may sit in its data cache: the core is halted and
    the log read through it."""
    time.sleep(FIRST_LOOK)
    ocd = OpenOCD(a)
    try:
        end, log, seen = time.time() + limit, None, ""
        while time.time() < end:
            log = log or find_log_ahb(ocd)
            if log:
                try:
                    t = log.text(ahb=True, peek=True)
                except IOError:
                    t = ""
                if any(m in t for m in BOOT_ENDS):
                    sys.stdout.write(log.text(ahb=True))
                    sys.stdout.flush()
                    return "Starting PJIT" in t
            try:
                ocd.halt()
            except IOError:
                time.sleep(3)
                continue
            try:
                log = log or find_log(ocd)
                if log:
                    seen = (seen + log.show())[-4096:]
            except IOError:
                pass
            finally:
                ocd.resume()
            if any(m in seen for m in BOOT_ENDS):
                return "Starting PJIT" in seen
            time.sleep(3)
        print("\n(the boot did not reach PJIT or the menu within %d s)" % limit)
        return False
    finally:
        ocd.close()


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[0], formatter_class=argparse.RawDescriptionHelpFormatter, epilog=__doc__.split("\n", 1)[1])
    ap.add_argument("command", choices=["install", "status", "settings", "erase-update", "log", "wipe"])
    ap.add_argument("value", nargs="?", help="settings: 'ITEM=VALUE;...'")
    ap.add_argument("--bootloader", default=default_image("bootloader.bin"), help="the bootloader image (default: %(default)s)")
    ap.add_argument("--bundle", default=default_image("buffee_boot.lzt"), help="the PJIT bundle (default: %(default)s)")
    ap.add_argument("--set", dest="settings", help="install: settings to change, 'ITEM=VALUE;...' as on the settings page")
    ap.add_argument("--copy-rom", action="store_true", help="install, settings: store a copy of the Amiga's Kickstart in the flash")
    ap.add_argument("--no-boot", action="store_true", help="install: leave the bootloader in the flash alone")
    ap.add_argument("--stay", action="store_true", help="leave the board waiting in the bootloader instead of resetting it at the end")
    ap.add_argument("--interface", default="interface/jlink.cfg", help="OpenOCD interface file (default: %(default)s)")
    ap.add_argument("--adapter-cmd", action="append", help="more OpenOCD commands for the adapter (repeatable)")
    ap.add_argument("--speed", type=int, default=100, help="JTAG clock in kHz (default: %(default)s; the Cortex-A8's debug port on a Buffee "
                    "answered up to 150 kHz through a J-Link and a TC2050 cable)")
    ap.add_argument("--openocd", help="the OpenOCD program")
    ap.add_argument("--ready-timeout", type=int, default=90, help="seconds the bootloader may take to come up (a first boot tests all of the DDR)")
    ap.add_argument("--yes", action="store_true", help="wipe: do not ask")
    ap.add_argument("--verbose", action="store_true")
    a = ap.parse_args()

    if a.command == "log":
        return cmd_log(a)
    if a.command == "settings" and not a.value:
        ap.error("settings needs 'ITEM=VALUE;...'")
    if a.command == "wipe" and not a.yes:
        print("This erases ALL of the board's flash (its bootloader, both Kickstart bundles, the ROM copy) and its settings.\n"
              "Afterwards the board does nothing at power-on until it is installed again.")
        if input("Type wipe to go on: ").strip() != "wipe":
            sys.exit("nothing done")
    boot = open(a.bootloader, "rb").read()
    check_bootloader(boot)
    boot += b"\0" * (-len(boot) % 4)
    bundle = None
    if a.command == "install":
        bundle = open(a.bundle, "rb").read()
        check_bundle(bundle)
        print("bootloader %s (%d bytes, CRC %08X)\nbundle     %s (%d bytes, CRC %08X)" % (a.bootloader, len(boot), zlib.crc32(boot) & 0xFFFFFFFF,
              a.bundle, len(bundle), zlib.crc32(bundle) & 0xFFFFFFFF), flush=True)

    b = Board(a)
    failed = 0
    try:
        b.start_bootloader(a.bootloader)
        b.wait_ready(a.ready_timeout)
        if a.command == "install":
            failed |= b.do("store_pjit", data=bundle)
            if a.copy_rom and not failed:
                failed |= b.do("copy_rom")
            if a.settings and not failed:
                failed |= b.do("settings", text=a.settings)
            if not a.no_boot and not failed:
                failed |= b.do("store_boot", data=boot)
            b.do("status")
        elif a.command == "settings":
            if a.copy_rom:
                failed |= b.do("copy_rom")
            if not failed:
                failed |= b.do("settings", text=a.value)
            b.do("status")
        elif a.command == "erase-update":
            failed |= b.do("erase_update")
            b.do("status")
        elif a.command == "status":
            b.do("status")
        elif a.command == "wipe":
            failed |= b.do("wipe", limit=300)
            b.do("status")
        if a.stay:
            print("\nThe board waits in the bootloader (ESC on its UART for the menu, or reset it)")
            b.ocd.close()
        else:
            b.reboot("its flash is empty" if a.command == "wipe" and not failed else "it boots from the flash")
            if a.command == "wipe":
                if not failed:
                    print("\nThe board is blank: it waits in its ROM for a debugger or the serial port (`install` brings it back)")
            elif not failed:
                print("\nThe board's boot from the flash:\n")
                try:
                    if not follow_boot(a):
                        failed = 1
                        print("\nThe boot from the flash did not start PJIT: see its report above")
                except (IOError, SystemExit) as e:
                    print("(could not read its log: %s)" % e)
    except KeyboardInterrupt:
        b.ocd.close()
        sys.exit("\ninterrupted: the board is in an unknown state; run the command again")
    except IOError as e:
        b.ocd.close()
        sys.exit("\nJTAG: %s" % e)
    print("\n" + ("FAILED: see above. The flash's backup is untouched unless the message says otherwise; run the command again." if failed else "Done."))
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
