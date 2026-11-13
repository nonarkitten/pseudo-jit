#!/usr/bin/env python3
"""Runs a PJIT payload bundle with a region / pace setting that lives in the bootloader's RAM only (nothing is
saved to the EEPROM; a reboot drops it), UART plugged straight into this machine:
  run_region.py REGION BUNDLE.lzt [seconds] [--pace N] [--esc-after S] [--end MARKER] [--port P]
REGION  the menu's Settings, Fast RAM syntax: '0' = no region (all guest RAM on the bus), '200000/8' = host DRAM at $200000,
'200000/8BF' = the Kipper's RAM on the bus, translated, unpaced, '200000/8BIF' = bus,
        interpret-only (the setting for a Kipper2k); '-' = leave the stored setting alone.
--pace N  Settings, Pace (0 = no pacing governor).  --esc-after S  sends ESC x3 after S seconds (the payload's report,
e.g. for buffee_boot).  --end MARKER  stop when the payload printed MARKER and the menu is back.
Unlike run_local.py this never sends ESC to find the menu: the board must already be at the bootloader menu
(ESC during a boot also stops it at the menu).
"""

import time, sys, argparse
from uart import add_port_argument, default_port, open_port, xmodem_send

ap = argparse.ArgumentParser()
ap.add_argument("region")
ap.add_argument("bundle")
ap.add_argument("seconds", nargs="?", type=float, default=90)
ap.add_argument("--pace", type=int)
ap.add_argument("--esc-after", type=float)
ap.add_argument("--end")
add_port_argument(ap)
a = ap.parse_args()
a.port = a.port or default_port()
s = open_port(a.port, timeout=0.2)


def rd(t):
    end = time.time() + t
    out = b""
    while time.time() < end:
        out += s.read(4096)
    return out.decode("latin1").replace("\r", "")


s.reset_input_buffer()
s.write(b"\r")
o = rd(0.8)
if "Ready" not in o and "] " not in o:
    sys.exit("no menu (is a payload still running? is Serial.app holding the port?): " + repr(o[-200:]))


def setting(item, value):
    """Menu 4 (Settings), the item, the value, 0 (back), n (not saved: the change lasts until a reset)."""
    s.write(b"4\r")
    rd(0.8)
    s.write(item.encode() + b"\r")
    rd(0.4)
    s.write(str(value).encode() + b"\r")
    o = rd(1.0)
    s.write(b"0\r")
    o += rd(0.5)
    if "[y/N]" in o:
        s.write(b"n\r")
        rd(0.5)
    return o


if a.region != "-":
    setting("4", a.region)
if a.pace is not None:
    setting("5", a.pace)
s.write(b"2\r")  # menu 2: run a bundle sent over XMODEM
rd(0.6)
data = open(a.bundle, "rb").read()


ok = xmodem_send(s, data)
print("xmodem:", "sent" if ok else "FAILED", len(data), "bytes", flush=True)
s.timeout = 0.1
t0 = time.time()
buf = ""
esc = False
while time.time() - t0 < a.seconds:
    d = s.read(1024)
    if d:
        t = d.decode("latin1").replace("\r", "")
        sys.stdout.write(t)
        sys.stdout.flush()
        buf += t
        if a.end and a.end in buf and "Ready" in buf.split(a.end)[-1]:
            break
    if a.esc_after is not None and not esc and time.time() - t0 > a.esc_after:
        s.write(b"\x1b\x1b\x1b")
        esc = True
        print("\n<<< ESC x3 >>>", flush=True)
    if esc and "Ready" in buf.split("<<< ESC")[-1] and time.time() - t0 > (a.esc_after or 0) + 3:
        break
print("\n--- end (settings were RAM-only: reboot the board to drop them) ---")
