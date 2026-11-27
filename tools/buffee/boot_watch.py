#!/usr/bin/env python3
"""Boots PJIT from the flash, lets the guest run, stops it with ESC x3 and prints the exit report's summary lines:
  boot_watch.py [seconds, default 100] [--log file] [--port /dev/cu.usbserial-XXXX]
For unattended checks of a boot: the TRAPS lines show every exception that is not an interrupt (an illegal instruction at PC 6 is a jump to address 0)."""
import sys, time, argparse
from uart import add_port_argument, default_port, open_port

ap = argparse.ArgumentParser()
ap.add_argument("seconds", nargs="?", type=float, default=100)
ap.add_argument("--log")
add_port_argument(ap)
a = ap.parse_args()
s = open_port(a.port or default_port(), timeout=0.2)
buf = b""
t = time.time()
while time.time() - t < 30 and b"Ready" not in buf:
    s.write(b"\x1b")
    buf += s.read(2000)
    time.sleep(0.2)
if b"Ready" not in buf:
    sys.exit("no menu")
time.sleep(1.5)
s.read(20000)
s.write(b"1\r")
out = b""
t = time.time()
while time.time() - t < a.seconds:
    out += s.read(4000)
s.write(b"\x1b\x1b\x1b")
t = time.time()
while time.time() - t < 15:
    out += s.read(8000)
text = out.decode("latin1")
if a.log:
    open(a.log, "w").write(text)
keep = ("starting at", "LIVE:", "TRAPS", "  ", "MEMLIST", "LASTEXC", "ABORTS", "STATE:", "TIER:")
show = False
for l in text.splitlines():
    if l.startswith("TRAPS"):
        show = True
    elif show and not l.startswith("  "):
        show = False
    if l.startswith(keep[:1] + keep[1:2] + keep[4:]) or l.startswith("TRAPS") or (show and l.startswith("  ")):
        print(l[:300])
