#!/usr/bin/env python3
"""Runs the benchmarks of tests/bbb on a Buffee board, one after the other, and keeps the console of each:
  run_bench.py BUNDLE_DIR [--out DIR] [--region 0] [--seconds 420] [test ...]
BUNDLE_DIR holds the bundles of `make ONBUFFEE=1 VARIANT=onbuffee onbuffee-all` (build/onbuffee/buffee). Without test names all of them run
(the default list below). Each console goes to DIR/<test>.txt (default local/bench); the last line printed per test says whether the report ended in PASS.
The board must be at the bootloader menu, booting, or running a payload that answers ESC x3. The settings are the bootloader's RAM-only ones (tools/buffee/run_region.py): no guest RAM region, the benchmarks' guest memory is DDR of their own.
"""

import argparse, os, subprocess, sys, time

here = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, here)
from uart import add_port_argument, default_port, open_port

ALL = (
    "dhrystone tinybasic bogomips micro branch opcost cadence wcet claimcost window bcache mispred tbsample tbcycles "
    "blocks blocks_cf blocks_sched dhcycles irq chain lzt"
).split()

ap = argparse.ArgumentParser()
ap.add_argument("bundles")
ap.add_argument("tests", nargs="*")
ap.add_argument("--out", default="local/bench")
ap.add_argument("--region", default="0")
ap.add_argument("--seconds", type=int, default=420)
add_port_argument(ap)
a = ap.parse_args()
port = a.port or default_port()
os.makedirs(a.out, exist_ok=True)


def to_menu():
    s = open_port(port, timeout=0.3)
    for _ in range(8):
        s.write(b"\x1b\x1b\x1b")
        time.sleep(0.6)
        if "Ready" in s.read(20000).decode("latin1"):
            s.close()
            return True
    s.close()
    return False


for t in a.tests or ALL:
    bundle = os.path.join(a.bundles, "bbb_%s.lzt" % t)
    if not os.path.exists(bundle):
        print("%-12s no bundle %s" % (t, bundle))
        continue
    if not to_menu():
        sys.exit("%s: the board did not come back to the menu (reset it, wait 20 s, run again)" % t)
    cmd = [
        sys.executable,
        os.path.join(here, "run_region.py"),
        a.region,
        bundle,
        str(a.seconds),
        "--end",
        "BUFFEE] exit",
    ]
    if a.port:
        cmd += ["--port", a.port]
    t0 = time.time()
    out = subprocess.run(cmd, capture_output=True, text=True).stdout
    open(os.path.join(a.out, t + ".txt"), "w").write(out)
    lines = out.splitlines()
    verdict = "PASS" if "PASS" in lines else ("FAULT" if "[FAULT]" in out else "no PASS line")
    print("%-12s %-12s %4.0f s" % (t, verdict, time.time() - t0), flush=True)
