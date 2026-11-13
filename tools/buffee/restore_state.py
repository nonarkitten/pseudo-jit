#!/usr/bin/env python3
"""Puts the Buffee board's STORED state back after something erased it (flashing the pseudo-jit_1 firmware wipes
the SPI flash slots and resets the EEPROM config):
  restore_state.py [--bundle build/firmware/buffee_boot.lzt] [--region 0] [--check] [--verify]
  [--port P]
The board must already be at the Buffee bootloader menu (run restore_bootloader.py first if it still runs the
pseudo-jit_1 firmware; ESC x3 from a terminal if a guest is running). This script never sends ESC.
Flash writes need Renee's go.
What it does, in order, and nothing else (no sector 0, no bootloader, no GreenPAK):
  1. menu 3 (Flash images): a slot that is 'absent' or bad is filled -- PJIT <- --bundle (xmodem; the CRC the
  bootloader reports must equal the file's, padded to 128 bytes with 0x1A like xmodem does),
     ROM <- a copy of the Amiga's own Kickstart (3, base and size from the settings); a slot that is 'ok' is
     left alone.
  2. menu 4 (Settings): MapROM = flash (1), Fast RAM = --region (4); leaving the page saves them in the EEPROM
  (which is read back), then the page is shown again and checked.
     Pace, the self-test settings and everything else are not touched.
--check prints the slot status and the config and writes nothing.
--verify then resets through the J-Link (tools/buffee/reset.sh), logs the boot for 30 s without sending
anything, sends ESC x3 and prints the key lines of the payload's report
(MEMLIST, EXPANSION.LIBRARY, STATE ...): a bad ROM or bundle shows up there.
The default region is 0: no fast RAM from the board, so the Amiga keeps the RAM it has. 200000/8 adds 8 MB of the
board's DRAM at $200000; 200000/8BIF describes a RAM board on the bus there (B = on the bus, I = interpret only,
F = unpaced).
"""

import sys, time, zlib, argparse, subprocess, os
from uart import add_port_argument, default_port, open_port, xmodem_send
import serial

here = os.path.dirname(os.path.abspath(__file__))
root = os.path.dirname(os.path.dirname(here))
ap = argparse.ArgumentParser()
ap.add_argument("--bundle", default=os.path.join(root, "build/firmware/buffee_boot.lzt"))
ap.add_argument("--region", default="0")
ap.add_argument("--check", action="store_true")
ap.add_argument("--verify", action="store_true")
add_port_argument(ap)
a = ap.parse_args()
a.port = a.port or default_port()
try:
    s = open_port(a.port, timeout=0.2)
except serial.SerialException as e:
    sys.exit("cannot open %s: %s (Serial.app has the port? disconnect it)" % (a.port, e))


def rd(t):
    end = time.time() + t
    out = b""
    while time.time() < end:
        out += s.read(4096)
    return out.decode("latin1").replace("\r", "")


def until(marker, limit):  # reads until marker has been seen and the menu that follows it is back
    t = time.time()
    out = ""
    while time.time() - t < limit:
        d = s.read(512)
        if d:
            out += d.decode("latin1").replace("\r", "")
        if marker in out and "0. Back" in out.split(marker)[-1]:
            break
    return out


def status(txt, name):
    for l in txt.split("\n"):
        if l.strip().startswith(name):
            return l.split(":", 1)[1].strip()
    return "?"


s.reset_input_buffer()
s.write(b"\r")
o = rd(0.8)
if "Ready" not in o and "] " not in o:
    sys.exit("not at the bootloader menu (%r): see the docstring" % o[-160:])

# 1. the flash slots
s.write(b"3\r")
o = rd(2.5)
pj, rom = status(o, "PJIT"), status(o, "ROM")
print("flash slots: PJIT %s, ROM %s" % (pj, rom))
if not a.check:
    if not pj.startswith("ok"):
        data = open(a.bundle, "rb").read()
        padded = data + b"\x1a" * (-len(data) % 128)
        want = zlib.crc32(padded) & 0xFFFFFFFF
        print("PJIT <- %s (%d bytes, CRC with xmodem padding %08X)" % (a.bundle, len(data), want), flush=True)
        s.write(b"1\r")
        time.sleep(0.5)
        s.read(2000)

        if not xmodem_send(s, data):
            sys.exit("xmodem failed (the slot is erased or half written: run again)")
        s.timeout = 0.2
        out = until("Stored", 90)
        print(out)
        if "CRC %08X" % want not in out:
            sys.exit("the bootloader did not report CRC %08X" % want)
    if not rom.startswith("ok"):
        print("ROM <- a copy of the Amiga ROM", flush=True)
        s.write(b"3\r")
        out = until("Stored", 90)
        print(out)
        if "Verify failed" in out or "Stored" not in out:
            sys.exit("ROM store failed")
s.write(b"0\r")
rd(0.8)

# 2. the settings
def region_key(text):
    """A region list as a comparable value: '0' / 'none' -> (), '$200000/8bif' -> (('200000', '8', 'BFI'),)."""
    out = []
    for part in text.strip().upper().split(","):
        if "/" not in part:
            continue
        base, rest = part.lstrip("$").split("/", 1)
        out.append((base.lstrip("0") or "0", "".join(c for c in rest if c.isdigit()), "".join(sorted(c for c in rest if c in "BIF"))))
    return tuple(out)


def show_settings():
    s.write(b"4\r")
    o = rd(1.5)
    s.write(b"0\r")
    rd(0.6)
    return o


if not a.check:
    s.write(b"4\r")
    rd(1.5)
    for item, val in (("1", "flash"), ("4", a.region)):
        s.write(item.encode() + b"\r")
        rd(0.4)
        s.write(val.encode() + b"\r")
        rd(1.2)
    s.write(b"0\r")
    o = rd(0.8)
    if "[y/N]" in o:
        s.write(b"y\r")
        o += rd(2.0)
    sv = [l for l in o.split("\n") if "Settings" in l]
    print(sv[-1] if sv else "settings unchanged")  # 'Settings saved (CRC ....)'
    if "did not read back" in o:
        sys.exit("the EEPROM did not read back what was written")
o = show_settings()
lines = [l for l in o.split("\n") if l[:3].strip().rstrip(".").isdigit()]
for l in lines[:5]:
    print(l[:72])
if not a.check:
    mp = [l for l in lines if "MapROM " in l and l.lstrip().startswith("1.")]
    ram = [l.split("Fast RAM", 1)[1].split()[0] for l in lines if "Fast RAM" in l]
    ok = bool(mp) and mp[0].split("MapROM", 1)[1].split()[0] == "flash" and bool(ram) and region_key(ram[0]) == region_key(a.region)
    print("settings:", "as asked" if ok else "NOT as asked (see above)")
    if not ok:
        sys.exit(1)

# 3. optional boot check
if a.verify:
    s.reset_input_buffer()
    rr = subprocess.run(["sh", os.path.join(here, "reset.sh")], capture_output=True, text=True, cwd=root)
    print(
        "reset.sh rc",
        rr.returncode,
        '(the JTAG "IR capture error" lines are harmless: only nSRST is used)',
        flush=True,
    )
    t0 = time.time()
    buf = ""
    while time.time() - t0 < 30:
        d = s.read(2048)
        if d:
            buf += d.decode("latin1").replace("\r", "")
    print(
        "\n".join(
            l
            for l in buf.split("\n")
            if any(
                k in l
                for k in ("MapROM", "unpacked", "Boot took", "Starting PJIT", "[MEM]", "starting at", "ASn:")
            )
        )
    )
    s.write(b"\x1b\x1b\x1b")
    rep = ""
    t1 = time.time()
    while time.time() - t1 < 40:
        d = s.read(4096)
        if d:
            rep += d.decode("latin1").replace("\r", "")
        if "Ready" in rep and time.time() - t1 > 3:
            break
    for l in rep.split("\n"):
        if any(k in l for k in ("MEMLIST", "EXPANSION", "STATE", "LIVE", "ABORTS")):
            print(l[:240])
