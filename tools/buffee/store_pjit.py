#!/usr/bin/env python3
"""Stores a PJIT bundle (.lzt) in the SPI flash slot the bootloader autoloads (menu F, 1), over a UART plugged
straight into this machine:
  store_pjit.py [--port /dev/cu.usbserial-XXXX] [--wait 40] bundle.lzt
ESC x3 until the menu ('Ready'), then F, 1 and an xmodem of the bundle; the header (length, CRC-32) is written
last, after a read-back check.
"""

import sys, time, argparse
from uart import add_port_argument, default_port, open_port, xmodem_send

ap = argparse.ArgumentParser()
ap.add_argument("bundle")
add_port_argument(ap)
ap.add_argument("--wait", type=float, default=40)
a = ap.parse_args()
a.port = a.port or default_port()
s = open_port(a.port, timeout=0.05)
buf = ""
t = time.time()
while time.time() - t < a.wait and "Ready" not in buf:
    s.write(b"\x1b\x1b\x1b")
    buf += s.read(2000).decode("latin1")
    time.sleep(0.05)
if "Ready" not in buf:
    sys.exit("no menu")
time.sleep(1.0)
s.read(20000)
s.write(b"F\r")
time.sleep(1.0)
print(s.read(4000).decode("latin1")[-400:])
s.write(b"1\r")
time.sleep(0.5)
s.read(2000)
data = open(a.bundle, "rb").read()


print(
    "xmodem:",
    ("sent" if xmodem_send(s, data) else "FAILED"),
    len(data),
    "bytes",
)
t = time.time()
s.timeout = 0.2
out = ""
while time.time() - t < 30:
    d = s.read(512)
    if d:
        out += d.decode("latin1")
    if "X. Exit" in out[-200:] and time.time() - t > 2:
        break
print(out[-700:])
s.write(b"X\r")
time.sleep(0.5)
