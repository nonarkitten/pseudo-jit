#!/usr/bin/env python3
"""Unattended stall hunt: resets Buffee through the J-Link, loads a payload bundle over the UART (RAM only,
nothing is written to the flash), watches the video capture of the Amiga's screen for the
picture freezing, and when it does sends ESC x3 and saves the payload's report. A timeline of screen activity
(mean frame difference, 4 fps) is written too.
  stallwatch.py bundle.lzt out_prefix [--minutes 20] [--arm 150] [--still 3.0] [--video 0] [--port
  /dev/cu.usbserial-XXXX]
Files: <out>.timeline.csv (t, diff), <out>.serial.txt (everything the board printed), <out>.report.txt (the
ESC x3 report), <out>.last.png-less: frames are not saved.
"""

import sys, os, time, subprocess, threading, argparse, collections
from uart import add_port_argument, default_port, open_port, xmodem_send

ap = argparse.ArgumentParser()
ap.add_argument("bundle")
ap.add_argument("out")
ap.add_argument("--minutes", type=float, default=20)
ap.add_argument(
    "--arm", type=float, default=150, help="seconds after the payload starts before a freeze counts"
)
ap.add_argument(
    "--still", type=float, default=3.0, help="seconds without a changing picture that count as a freeze"
)
ap.add_argument("--video", default="0")
add_port_argument(ap)
a = ap.parse_args()
a.port = a.port or default_port()
repo = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
W, H = 160, 120
serlog = open(a.out + ".serial.txt", "wb")
tl = open(a.out + ".timeline.csv", "w")
tl.write("t,diff\n")
s = open_port(a.port, timeout=0.05)


def pump(sec):
    t = time.time()
    buf = b""
    while time.time() - t < sec:
        d = s.read(4000)
        if d:
            buf += d
            serlog.write(d)
            serlog.flush()
    return buf


print("reset ...", flush=True)
subprocess.run(["sh", os.path.join(repo, "tools/buffee/reset.sh")], capture_output=True, timeout=30)
buf = b""
t0 = time.time()
while time.time() - t0 < 90 and b"Ready" not in buf:
    s.write(b"\x1b\x1b\x1b")
    buf += pump(0.4)
if b"Ready" not in buf:
    sys.exit("no bootloader menu")
time.sleep(1.5)
s.read(20000)
s.write(b"2\r")  # menu 2: run a bundle sent over XMODEM
time.sleep(0.5)
s.read(20000)
data = open(a.bundle, "rb").read()


ok = xmodem_send(s, data)
print("xmodem", "sent" if ok else "FAILED", flush=True)
if not ok:
    sys.exit(1)
s.timeout = 0.05
t_start = time.time()

ff = subprocess.Popen(
    [
        "ffmpeg",
        "-hide_banner",
        "-loglevel",
        "error",
        "-f",
        "avfoundation",
        "-framerate",
        "30",
        "-pixel_format",
        "uyvy422",
        "-video_size",
        "640x480",
        "-i",
        a.video,
        "-vf",
        "fps=4,scale=%d:%d,format=gray" % (W, H),
        "-f",
        "rawvideo",
        "-",
    ],
    stdout=subprocess.PIPE,
    stderr=subprocess.DEVNULL,
)
frames = collections.deque()


def reader():
    while True:
        b = ff.stdout.read(W * H)
        if not b or len(b) < W * H:
            break
        frames.append((time.time(), b))


threading.Thread(target=reader, daemon=True).start()

prev = None
last_change = time.time()
changes = collections.deque()
freeze = None
THR = 0.7
try:
    while time.time() - t_start < a.minutes * 60:
        d = s.read(4000)
        if d:
            serlog.write(d)
        while frames:
            ts, fr = frames.popleft()
            if prev is not None:
                diff = sum(abs(x - y) for x, y in zip(fr[::7], prev[::7])) / len(fr[::7])
                tl.write("%.2f,%.3f\n" % (ts - t_start, diff))
                if diff > THR:
                    last_change = ts
                    changes.append(ts)
            prev = fr
        while changes and changes[0] < time.time() - 40:
            changes.popleft()
        now = time.time()
        if now - t_start > a.arm and len(changes) > 25 and now - last_change > a.still:
            freeze = now - t_start
            print(
                "FREEZE at t=%.1f s (still for %.1f s): sending ESC x3" % (freeze, now - last_change),
                flush=True,
            )
            break
        time.sleep(0.05)
finally:
    tl.flush()
if freeze is None:
    print("no freeze in %.0f min: asking for the report anyway" % a.minutes, flush=True)
s.write(b"\x1b\x1b\x1b")
rep = pump(14)
ff.terminate()
open(a.out + ".report.txt", "wb").write(rep)
print(
    "report: %d bytes (%s)"
    % (len(rep), "got the menu" if b"Ready" in rep else "NO MENU: the payload did not react to ESC x3"),
    flush=True,
)
