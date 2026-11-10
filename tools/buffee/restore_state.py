#!/usr/bin/env python3
"""Puts the Buffee board's STORED state back after something erased it (flashing the pseudo-jit_1 firmware wipes
the SPI flash slots and resets the EEPROM config):
  restore_state.py [--bundle build/firmware/buffee_boot.lzt] [--region 0] [--check] [--verify]
  [--port P]
The board must already be at the Buffee bootloader menu (run restore_bootloader.py first if it still runs the
pseudo-jit_1 firmware; ESC x3 from a terminal if a guest is running,
but never right after a reset: the first 30 ms are the emergency-erase window). This script never sends ESC.

What it does, in order, and nothing else (no sector 0, no bootloader, no GreenPAK):
  1. menu F: a slot that is 'absent' or bad is filled -- PJIT <- --bundle (xmodem; the CRC the bootloader
  reports must equal the file's, padded to 128 bytes with 0x1A like xmodem does),
     ROM <- a copy of the Amiga's own Kickstart (menu F 5, base and size from the EEPROM config); a slot that
     is 'ok' is left alone.
  2. menu E 3: MapROM = the SPI flash image (G F), guest RAM regions = --region, autoload and autorun on (S E,
  T E); E 2 saves, E 1 reloads and the config is printed and checked.
     Pace, the POST flags and everything else are not touched.
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
        if marker in out and "X. Exit" in out.split(marker)[-1]:
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
s.write(b"F\r")
o = rd(2.5)
pj, rom = status(o, "PJIT"), status(o, "ROM")
print("flash slots: PJIT %s, ROM %s" % (pj, rom))
if not a.check:
    if pj != "ok":
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
    if rom != "ok":
        print("ROM <- a copy of the Amiga ROM", flush=True)
        s.write(b"5\r")
        out = until("Stored", 90)
        print(out)
        if "Verify failed" in out or "Stored" not in out:
            sys.exit("ROM store failed")
s.write(b"X\r")
rd(0.8)

# 2. the EEPROM config
s.write(b"E\r")
rd(0.6)
s.write(b"3\r")
o = rd(1.5)
if not a.check:
    for key, val in (("G", "F"), ("R", a.region), ("S", "E"), ("T", "E")):
        s.write(key.encode() + b"\r")
        rd(0.4)
        s.write(val.encode() + b"\r")
        o = rd(1.2)
s.write(b"X\r")
rd(0.6)
if not a.check:
    s.write(b"2\r")
    sv = [l for l in rd(2.0).split("\n") if "Saving" in l]
    print(sv[0] if sv else "no save message")  # 'Saving and verifying settings (CRC=....)'
    s.write(b"1\r")
    rd(1.5)
    s.write(b"3\r")
    o = rd(1.5)
    s.write(b"X\r")
    rd(0.6)
for l in o.split("\n"):
    if l[:2] in ("G.", "R.", "S.", "T.", "V.") or l.startswith("P."):
        print(l[:60] + (" ... " + l[-24:] if len(l) > 60 else ""))
if not a.check:
    r = [l for l in o.split("\n") if l.startswith("R.")][0]

    def region_ok(line, region):
        """Does the R. line of the reloaded config show the region that was asked for?"""
        if "," in region:  # a list of regions: shown as asked, not parsed here
            return True
        if "/" not in region:  # '0': no region
            return line.rstrip().endswith("none")
        base, rest = region.split("/", 1)
        mb = "".join(ch for ch in rest if ch.isdigit())
        flags = "".join(c for c in "BIF" if c in rest.upper())
        return line.endswith(" %s/%sMB%s" % (base.upper().lstrip("0") or "0", mb, flags))

    ok = (
        "SPI flash image" in o
        and "Autoload PJIT from the SPI flash: Enabled" in o
        and "Autorun PJIT after loading: Enabled" in o
        and region_ok(r, a.region)
    )
    print("config after the reload from the EEPROM:", "as asked" if ok else "NOT as asked (see above)")
    if not ok:
        sys.exit(1)
s.write(b"X\r")
rd(0.6)

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
                for k in ("MapROM", "Unpacked", "Completed", "Starting PJIT", "[MEM]", "starting at", "ASn:")
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
