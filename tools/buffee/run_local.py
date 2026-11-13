#!/usr/bin/env python3
"""Runs a PJIT payload bundle on Buffee with the UART (FTDI) plugged straight into this machine (no bridge BBB):
  run_local.py [--port /dev/cu.usbserial-XXXX] bundle.lzt [seconds to watch, default 60]
Power-cycle the board first: ESC x3 is sent until the bootloader's menu ('Ready') shows, then menu 2 + the bundle,
then the payload's output is streamed.
"""

import sys, os, time, argparse
from uart import add_port_argument, default_port, open_port, xmodem_send

ap = argparse.ArgumentParser()
ap.add_argument("bundle")
ap.add_argument("seconds", nargs="?", type=float, default=60)
add_port_argument(ap)
a = ap.parse_args()
a.port = a.port or default_port()
s = open_port(a.port, timeout=0.05)
buf = ""
t = time.time()
while time.time() - t < float(os.environ.get("RUN_WAIT", 30)) and "Ready" not in buf:
    s.write(b"\x1b\x1b\x1b")
    buf += s.read(2000).decode("latin1")
    time.sleep(0.05)
if "Ready" not in buf:
    sys.exit("no menu (power-cycle the board; a running payload cannot be interrupted)")
time.sleep(1.5)
s.read(20000)
s.write(b"2\r")  # menu 2: run a bundle sent over XMODEM
time.sleep(0.5)
s.read(20000)
data = open(a.bundle, "rb").read()


t0 = time.time()
ok = xmodem_send(s, data)
print(
    "xmodem: %s, %d bytes in %.1f s" % ("sent" if ok else "FAILED", len(data), time.time() - t0), flush=True
)
# keystrokes typed here go to the board (ESC x3 = the break that makes a running payload print its report and
# exit)
import select, termios, tty

tty_ok = sys.stdin.isatty()
if tty_ok:
    old = termios.tcgetattr(sys.stdin)
    tty.setcbreak(sys.stdin.fileno())
try:
    t1 = time.time()
    s.timeout = 0.05
    while time.time() - t1 < a.seconds:
        d = s.read(512)
        if d:
            sys.stdout.write(d.decode("latin1"))
            sys.stdout.flush()
        if tty_ok and select.select([sys.stdin], [], [], 0)[0]:
            s.write(os.read(sys.stdin.fileno(), 16))
finally:
    if tty_ok:
        termios.tcsetattr(sys.stdin, termios.TCSADRAIN, old)
