#!/usr/bin/env python3
"""Updates the bootloader in Buffee's SPI flash over the UART (FTDI plugged into this machine):
  update_boot.py [--port /dev/cu.usbserial-XXXX] [--reboot] bootloader.bin
ESC x3 until the menu ('Ready'), then 3 (Flash images), 6 + xmodem of bootloader.bin (the bootloader checks that it
looks like one before it writes). --reboot then reboots (0, 0, y) and prints the cold-boot messages.
A bootloader with the older lettered menu (RC1: 'U. Update the bootloader') gets 'U' and 'X', 'y' instead.
"""

import sys, time, argparse
from uart import add_port_argument, default_port, open_port, xmodem_send

ap = argparse.ArgumentParser()
ap.add_argument("image")
add_port_argument(ap)
ap.add_argument("--reboot", action="store_true")
ap.add_argument(
    "--wait", type=float, default=40, help="seconds to keep sending ESC x3 (power-cycle the board meanwhile)"
)
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
    sys.exit("no menu (power-cycle the board, then run this again)")
time.sleep(1.0)
s.read(20000)
s.write(b"\r")  # an empty line shows the menu (both kinds)
time.sleep(1.0)
lettered = "U. Update the bootloader" in s.read(20000).decode("latin1")
data = open(a.image, "rb").read()
print("image", len(data), "bytes, first word", data[:4].hex())
if lettered:
    s.write(b"U\r")
else:
    s.write(b"3\r")
    time.sleep(1.0)
    s.read(20000)
    s.write(b"6\r")
time.sleep(0.5)
s.read(20000)


ok = xmodem_send(s, data)
print("xmodem:", "sent" if ok else "FAILED")
t = time.time()
s.timeout = 0.2
out = ""
while time.time() - t < 25:
    d = s.read(512)
    if d:
        out += d.decode("latin1")
    if "Ready" in out and time.time() - t > 3:
        break
print(out[-600:])
if not lettered:
    s.write(b"0\r")  # back from Flash images
    time.sleep(0.5)
if a.reboot:
    s.write(b"X\r" if lettered else b"0\r")
    time.sleep(0.5)
    s.write(b"y\r")
    print("--- reboot ---")
    t = time.time()
    out = ""
    while time.time() - t < 12:
        d = s.read(512)
        if d:
            out += d.decode("latin1")
    print(out)
