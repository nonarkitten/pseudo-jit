#!/usr/bin/env python3
"""Boots PJIT from the flash through the bootloader's menu (item 1) and prints the line that says where the guest starts:
  boot_flash.py [--port /dev/cu.usbserial-XXXX]
ESC is sent until the menu shows (a board at the menu; a running payload needs ESC x3, which this also sends)."""
import sys, time, argparse
from uart import add_port_argument, default_port, open_port

ap = argparse.ArgumentParser()
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
t = time.time()
out = b""
while time.time() - t < 15:
    out += s.read(4000)
lines = [l for l in out.decode("latin1").splitlines() if "starting" in l or "vectors" in l]
print("\n".join(lines) if lines else "no start line:\n" + out.decode("latin1")[-600:])
