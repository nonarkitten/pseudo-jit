#!/usr/bin/env python3
"""Overnight soak of the stored (autoloaded) bundle across guest memory configurations, with whatever disk is in
the Amiga's drive. For each configuration the region list is set through the bootloader's Settings
page (4, 4, the value, 0, y: saved), then N cycles of: J-Link reset, wait for the payload to start, wait WAIT seconds, ESC
x3, read the report and judge it. The original configuration is restored at the end.
  soak.py out_prefix [--cycles 100] [--wait 12] [--configs 8,4,2,1,0 (MB of host DRAM at $200000, or region strings such as 200000/8BIF)] [--final 0]
Files: <out>.log (one line per cycle), <out>.flags.txt (full reports of every flagged cycle)."""

import subprocess, time, re, os, argparse
from uart import add_port_argument, default_port, open_port

ap = argparse.ArgumentParser()
ap.add_argument("out")
ap.add_argument("--cycles", type=int, default=100)
ap.add_argument("--wait", type=float, default=12)
ap.add_argument("--configs", default="8,4,2,1,0")
ap.add_argument("--final", default="0")
add_port_argument(ap)
a = ap.parse_args()
a.port = a.port or default_port()
repo = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
log = open(a.out + ".log", "a")
flags = open(a.out + ".flags.txt", "a")


def out(t):
    print(t, flush=True)
    log.write(t + "\n")
    log.flush()


s = open_port(a.port, timeout=0.1)


def reset():
    s.read(100000)
    subprocess.run(
        ["sh", os.path.join(repo, "tools/buffee/reset.sh")], capture_output=True, cwd=repo, timeout=40
    )


def to_menu(limit=40):
    buf = ""
    t = time.time()
    while time.time() - t < limit and "Ready" not in buf:
        s.write(b"\x1b\x1b\x1b")
        buf += s.read(4000).decode("latin1")
        time.sleep(0.2)
    return buf if "Ready" in buf else None


def cmd(text, wait=1.2):
    s.read(100000)
    s.write(text.encode() + b"\r")
    time.sleep(wait)
    return s.read(20000).decode("latin1")


def set_regions(val):
    reset()
    if to_menu() is None:
        return False
    time.sleep(1.5)
    s.read(20000)
    cmd("4")
    cmd("4", 1.0)
    o = cmd(val, 1.5)
    ok = "Fast RAM" in o and "rejected" not in o
    o2 = cmd("0", 1.0)
    if "[y/N]" in o2:
        o2 = cmd("y", 2.5)
    out(
        "   config set to %r (%s): %s" % (val, "shown" if ok else "NOT SHOWN", re.sub(r"\s+", " ", o2)[-120:])
    )
    return ok


def cycle(i, expect_mb):
    reset()
    t0 = time.time()
    buf = ""
    started = None
    while time.time() - t0 < 30:
        buf += s.read(4000).decode("latin1")
        if "starting at" in buf:
            started = time.time()
            break
    if started is None:
        return "NO START", buf, None
    time.sleep(a.wait)
    rep = ""
    t1 = time.time()
    while time.time() - t1 < 20 and "Ready" not in rep:
        s.write(b"\x1b\x1b\x1b")
        rep += s.read(5000).decode("latin1")
        time.sleep(0.15)
    if "Ready" not in rep:
        return "NO MENU after ESC x3", buf + rep, None
    time.sleep(2.0)
    rep += s.read(20000).decode("latin1")
    full = buf + rep
    el = time.time() - started
    f = ""
    m = re.search(r"LIVE: (\d+) exceptions delivered", rep)
    if not m:
        return "no LIVE line", full, None
    n = int(m.group(1))
    rate = n / el
    mem = re.search(r"\[MEM\] (\d+) MB of guest RAM", buf)
    if expect_mb is not None and (mem is None or int(mem.group(1)) != expect_mb):
        f += " MEM(%s != %s)" % (mem.group(1) if mem else "none", expect_mb)
    if "[ABT]" in rep or "FATAL" in rep:
        f += " FATAL"
    if not re.search(r"ABORTS: 0 bus errors, 0 address errors", rep):
        f += " ABORTS"
    traps = re.findall(r"^\s+([0-9A-F]+)@([0-9A-F]+):", rep, re.M)
    odd = [t for t in traps if t != ("4", "F80C3A")]
    if odd:
        f += " TRAP(%s)" % ",".join("%s@%s" % t for t in odd[:4])
    if rate < 20:
        f += " LOW-IRQ-RATE"
    sm = re.search(
        r"SMC: (\d+) write faults .*?\((\d+) hit translated code, (\d+) not decoded\), (\d+) flushes", rep
    )
    return (
        f,
        full,
        "%d exc %.0f/s, SMC %s hits %s flushes %s"
        % (n, rate, sm.group(1) if sm else "?", sm.group(2) if sm else "?", sm.group(4) if sm else "?"),
    )


tot = bad = 0
for tag in a.configs.split(","):
    if (
        "/" in tag
    ):  # a region string as the menu takes it, e.g. 200000/8BIF: host DRAM only without the B flag
        val, mb = tag, (
            0
            if "B" in tag.split("/")[1].upper().replace("MB", "")
            else int(re.match(r"\d+", tag.split("/")[1]).group())
        )
    else:
        mb = int(tag)
        val = "200000/%d" % mb if mb else "0"
    expect = mb
    out("=== configuration: %s MB fast RAM (%s), %d cycles ===" % (mb, val, a.cycles))
    if not set_regions(val):
        out("   could not set the configuration: skipping")
        bad += 1
        continue
    nb = 0
    for i in range(1, a.cycles + 1):
        f, full, info = cycle(i, expect)
        tot += 1
        if f:
            nb += 1
            bad += 1
            flags.write("==== %s MB cycle %d: %s ====\n%s\n" % (mb, i, f, full.replace("\r", "")))
            flags.flush()
        out("%s MB %3d: %s%s" % (mb, i, info or "", (" FLAG:" + f) if f else ""))
    out("=== %s MB done: %d flagged of %d ===" % (mb, nb, a.cycles))
out("restoring %s" % a.final)
set_regions(a.final)
f, full, info = cycle(0, None)
out("final check: %s %s" % (f or "ok", info))
out("SOAK DONE: %d cycles, %d flagged" % (tot, bad))
