#!/usr/bin/env python3
"""GreenPAK netlist shotgun (tests/buffee/buffee_shotgun.c): tries combinations of clock sources for the ASn
release chain, live, and reports bus errors and cycle times.
  shotgun.py [--port P] [--bundle build/buffee/buffee/buffee_shotgun.lzt] [--out results.json] [--quick]
Chain: ASn falls -> ARM (DFF4, OUT11 clock) -> pipe delay (OUT28 clock, tap 0x9C) ->
LUT5 (OUT23 = its IN0) -> DFF5 (OUT14 clock) -> DFF7 (OUT20 clock) = DONE -> WAIT0.
Clock choices: 15 = CLK7En (inverted CLK7E, one rising edge per CLK7), 1 = the raw CLK7E pin, 40 = MF1 as
XOR(CLK7En, CLK7E): a short pulse on both edges.
A hung bus resets the SoC (watchdog in the payload): the script gets back to the menu with ESC x3, reloads the
payload and carries on; combinations that contain a hung one are skipped.
"""

import time, json, argparse, itertools
from uart import add_port_argument, default_port, open_port, xmodem_send

ap = argparse.ArgumentParser()
add_port_argument(ap)
ap.add_argument("--bundle", default="build/buffee/buffee/buffee_shotgun.lzt")
ap.add_argument("--out", default="shotgun_results.json")
ap.add_argument("--quick", action="store_true")
ap.add_argument(
    "--capture",
    default="",
    help="dir for Logic captures; then only the combos in --only are run, each with a long loop and a capture of the reads and one of the writes",
)
ap.add_argument("--only", default="")
ap.add_argument(
    "--soak", type=int, default=0, help="run only the combos of --only with this many reads and writes each"
)
ap.add_argument(
    "--ops",
    default="",
    help="label=op,op,...;label=... : run these op strings (see the payload) instead of the generated combinations; with --capture also capture reads and writes",
)
ap.add_argument(
    "--phases",
    default="r:1,w:2",
    help="with --ops --capture: tag:T-value pairs, one capture each (T1 reads, T2 random writes, T4 the write stream)",
)
ap.add_argument(
    "--cnt",
    action="store_true",
    help="clock choices 29 = the CNT/DLY one-shot (function 3, data 1) instead of 40 = the XOR",
)
a = ap.parse_args()
a.port = a.port or default_port()
s = open_port(a.port, timeout=0.05)


def readall(t=0.3):
    end = time.time() + t
    out = b""
    while time.time() < end:
        out += s.read(2000)
    return out.decode("latin1")


def to_menu(limit=120):
    buf = ""
    t = time.time()
    while time.time() - t < limit and "Ready" not in buf:
        s.write(b"\x1b\x1b\x1b")
        buf += s.read(2000).decode("latin1")
        time.sleep(0.05)
    if "Ready" not in buf:
        raise SystemExit("no menu")
    time.sleep(1.2)
    s.read(20000)


def load():
    to_menu()
    s.write(b"L\r")
    time.sleep(0.5)
    s.read(20000)
    data = open(a.bundle, "rb").read()

    if not xmodem_send(s, data):
        raise SystemExit("xmodem failed")
    s.timeout = 0.2
    out = ""
    t = time.time()
    while time.time() - t < 30 and "SGREADY" not in out:
        out += s.read(500).decode("latin1")
    if "SGREADY" not in out:
        raise SystemExit("payload did not start: " + out[-300:])
    print(
        "payload up:",
        out[out.find("SGREADY") :].split("\n")[0].strip(),
        "|",
        next((l.strip() for l in out.split("\n") if l.startswith("SGGPMC")), ""),
        flush=True,
    )


XORCFG = ["RA7:00", "RA8:66", "M34:15", "M35:1"]
# MF1 as a both-edge one-shot of the raw CLK7E (matrix 29)
CNTCFG = [
    "RA7:42",
    "RA9:30",
    "RAA:01",
    "R81:03",
    "M36:1",
]
DEFAULT = ("en0", "15", "1", "15")  # pipe, DFF5 clock, DFF7 clock, ARM clock


def ops_for(c):
    pipe, d5, d7, arm = c
    ops = []
    if "x" in pipe or d5 == "40" or d7 == "40" or arm == "40":
        ops += XORCFG
    if "29" in (d5, d7, arm):
        ops += CNTCFG
    if pipe.startswith("x"):
        ops += ["M28:40"]
    if pipe == "byp":
        ops += ["M23:19"]
    if d5 != "15":
        ops += ["M14:" + d5]
    if d7 != "1":
        ops += ["M20:" + d7]
    if arm != "15":
        ops += ["M11:" + arm]
    if pipe.startswith("x") or pipe.startswith("en"):
        ops += ["R9C:0" + pipe[-1]]
    return ",".join(ops)


pipes = ["en0", "x0", "x1", "x2", "x3", "byp", "en1"] if not a.quick else ["en0", "x0", "byp"]
if a.cnt:
    combos = [c for c in itertools.product(["en0", "byp"], ["15", "29"], ["1", "29", "15"], ["15", "29"])]
else:
    combos = [c for c in itertools.product(pipes, ["15", "40"], ["1", "40", "15"], ["15", "40"])]
combos.sort(key=lambda c: (sum(x != d for x, d in zip(c, DEFAULT)), c))


def contains(c, h):
    return all(c[i] == h[i] for i in range(4) if h[i] != DEFAULT[i])


if a.ops:
    import subprocess

    load()
    for n, item in enumerate(a.ops.split(";")):
        label, _, ops = item.partition("=")
        phases = (
            [("", 3)]
            if not a.capture
            else [(x.split(":")[0], int(x.split(":")[1])) for x in a.phases.split(",")]
        )
        for tag, ph in phases:
            sep = "," if ops else ""
            cmd = "%s%sT%d%s" % (ops, sep, ph, ",N2000000" if a.capture else "")
            s.write(("S %d %s\n" % (n, cmd)).encode())
            t = time.time()
            out = ""
            if a.capture:
                while time.time() - t < 20 and ("SGSET %d" % n) not in out:
                    out += s.read(500).decode("latin1")
                time.sleep(0.8)
                d = "%s/%s_%s" % (a.capture, label, tag)
                subprocess.run(
                    [
                        ".venv/bin/python",
                        "tools/buffee/logic_capture.py",
                        d,
                        "--manual",
                        "0.03",
                        "--rate",
                        "24000000",
                        "--timeout",
                        "60",
                    ],
                    capture_output=True,
                    text=True,
                )
            while time.time() - t < 60 and not __import__("re").search(
                r"SGRES %d [^\n]*w2=\d+\r?\n" % n, out
            ):
                out += s.read(500).decode("latin1")
            line = next((l for l in out.split("\n") if l.startswith("SGRES")), None)
            idle = next((l.strip() for l in out.split("\n") if l.startswith("SGIDLE")), "")
            print(label, tag, line or "NO RESULT (hang?)", "|", idle, flush=True)
            if not line:
                load()
    s.write(b"Q\n")
    print(readall(2)[-60:])
    raise SystemExit
if a.soak:
    wanted = [tuple(x.split("/")) for x in a.only.split(";")]
    load()
    for n, c in enumerate(wanted):
        s.write(("S %d %s,N%d\n" % (n, ops_for(c), a.soak)).encode())
        t = time.time()
        out = ""
        while time.time() - t < 600 and ("SGRES %d " % n) not in out:
            out += s.read(500).decode("latin1")
        print(c, next((l for l in out.split("\n") if l.startswith("SGRES")), "NO RESULT (hang?)"), flush=True)
        if "SGRES" not in out:
            load()
    s.write(b"Q\n")
    print(readall(2)[-60:])
    raise SystemExit
if a.capture:
    import subprocess

    wanted = [tuple(x.split("/")) for x in a.only.split(";")]
    load()
    for n, c in enumerate(wanted):
        s.write(("S %d %s,N1500000\n" % (n, ops_for(c))).encode())
        t = time.time()
        out = ""
        while time.time() - t < 20 and ("SGSET %d" % n) not in out:
            out += s.read(500).decode("latin1")
        for tag, delay in (("r", 0.5), ("w", 2.6)):
            time.sleep(delay if tag == "r" else 2.0)
            d = "%s/c%d_%s" % (a.capture, n, tag)
            r = subprocess.run(
                [
                    ".venv/bin/python",
                    "tools/buffee/logic_capture.py",
                    d,
                    "--manual",
                    "0.03",
                    "--rate",
                    "24000000",
                    "--timeout",
                    "60",
                ],
                capture_output=True,
                text=True,
            )
            print("captured", c, tag, d, r.stdout.strip()[-30:], r.stderr.strip()[-100:], flush=True)
        t = time.time()
        while time.time() - t < 20 and ("SGRES %d " % n) not in out:
            out += s.read(500).decode("latin1")
        print(next((l for l in out.split("\n") if l.startswith("SGRES")), "no result"), flush=True)
    s.write(b"Q\n")
    print(readall(2)[-100:])
    raise SystemExit
load()
results = []
hung = []
for n, c in enumerate(combos):
    h = next((x for x in hung if contains(c, x)), None)
    if h:
        results.append({"id": n, "combo": c, "status": "skipped (contains hung %s)" % (h,)})
        continue
    s.write(("S %d %s\n" % (n, ops_for(c))).encode())
    t = time.time()
    out = ""
    while time.time() - t < 20 and ("SGRES %d " % n) not in out:
        out += s.read(500).decode("latin1")
    line = next((l for l in out.split("\n") if l.startswith("SGRES %d " % n)), None)
    if line:
        r = dict(kv.split("=") for kv in line.split()[2:])
        r = {k: int(v) for k, v in r.items()}
        r.update({"id": n, "combo": c, "status": "ok"})
        results.append(r)
        print(
            "%3d %-26s rbad %-5d rcyc %-5d wbad %-5d wcyc %-5d p40 %-3d p24 %d"
            % (n, c, r["rbad"], r["rcyc"], r["wbad"], r["wcyc"], r["p40"], r["p24"]),
            flush=True,
        )
    else:
        print("%3d %-26s HANG -> recovering" % (n, c), flush=True)
        hung.append(c)
        results.append({"id": n, "combo": c, "status": "hang"})
        load()
    json.dump(results, open(a.out, "w"), indent=1)
s.write(b"Q\n")
print(readall(2)[-200:])
print(
    "done:",
    sum(1 for r in results if r["status"] == "ok"),
    "ok,",
    sum(1 for r in results if r["status"] == "hang"),
    "hung,",
    sum(1 for r in results if r["status"].startswith("skipped")),
    "skipped",
)
