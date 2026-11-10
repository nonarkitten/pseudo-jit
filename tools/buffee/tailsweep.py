#!/usr/bin/env python3
"""GPMC tail sweep: shorten the GPMC cycle that runs AFTER the GreenPAK has released WAIT,
at 10 ns granularity.

Why (AM335x TRM 7.1.3.3.8.3.2 / .3, 7.1.3.3.9.8): with WAIT monitored, an active WAIT FREEZES the GPMC
cycle-time counter at RDACCESSTIME (reads) / WRACCESSTIME (writes);
WAIT must be valid two GPMC_FCLK cycles (20 ns) before that point. When WAIT goes inactive the counter runs
again after a two-cycle pipeline (+ WAITMONITORINGTIME extra cycles),
the data is captured at that moment, and every later event (OE/WE off, CS off, end of cycle) happens at its
programmed count: so the time from the WAIT release to the next request is
    2 + WMT + (CYCLETIME - ACCESSTIME) units, + ~15 ns to start the next cycle,
independent of when the bus is "really" done. WRACCESSTIME does NOT say when data is written (WEOFFTIME does);
for async writes it is only the point where WAIT is looked at.
The shipped timing (x2 granularity, RDACCESS 12, CYCLETIME 28) leaves 16 units = 320 ns of tail after every
release, the whole reason a stalled read takes 6 CLK7.
Here: TIMEPARAGRANULARITY off (10 ns units, fields max 31 = 310 ns), the freeze point early, and the tail
(CYCLETIME - RDACCESS, WRCYCLE - WRACCESS) swept.

  tailsweep.py --model                      predicted periods for the grid (no board)
  tailsweep.py --run [--bundle B] [--out results.json] [--extra "RB5:AA,R94:90"]   needs the Buffee bootloader (build/firmware/bootloader.bin) in the flash
                                            and the UART free (Serial.app closed); every variant restores the
                                            baseline afterwards; a hung bus is recovered through the payload's
                                            watchdog (the script waits passively 8 s, then ESC x3: never ESC
                                            right after a reset).
Each variant runs the shotgun payload's read / write / CIA test; 'rcyc' is ns per read, 'wcyc' ns per (write +
2 read-backs). 'done=1' also ends READS on DONE (D5q) like writes:
LUT4_0 (0xA1 / 0xA2) = X | D5q | WRrel = 0xFFFA instead of 0xBBBA."""

import sys, time, json, argparse, itertools, re
from uart import add_port_argument, default_port, open_port, xmodem_send

CLK = 140.97  # ns per CLK7 (7.0937 MHz)
U = 10.0  # ns per GPMC_FCLK cycle
# what the bootloader programs (x2 units)
BASE = dict(g=2, wmt=1, racc=12, oeoff=17, rcyc=28, wacc=16, weoff=19, wcyc=20, tr=16, tw=4, done=0)


def words(g, wmt, racc, doe, tr, wacc, dwe, tw, weon=4):
    """CONFIG1, 2, 4, 5, 6 for CS2. Fields in granules (g = 1: 10 ns). OEON = CSON = 0, CS pad is released
    (any CSOFF below the cycle times will do).
    """
    oeoff, rcyc = racc + doe, racc + tr
    weoff, wcyc = wacc + dwe, wacc + tw
    csoff = max(oeoff, weoff)
    assert rcyc <= 31 and wcyc <= 31 and oeoff < rcyc and weoff < wcyc and csoff < min(rcyc, wcyc), (
        rcyc,
        wcyc,
        oeoff,
        weoff,
    )
    c1 = 0x00601000 | (wmt << 18) | (0x10 if g == 2 else 0)
    c2 = (csoff << 16) | (csoff << 8)
    c4 = (weoff << 24) | (weon << 16) | (oeoff << 8)
    c5 = (racc << 16) | (wcyc << 8) | rcyc
    c6 = wacc << 24
    return c1, c2, c4, c5, c6


def model(g, wmt, racc, oeoff, rcyc, wacc, weoff, wcyc, done, **_):
    """Predicted period (CLK7) of back-to-back reads / writes. Constants are the measured timeline of the shipped
    timing:
    D5q (release flop) rises 1.72 clk after the ASn fall, WAIT0 is up 0.5 clk later (V, S, OR LUTs + pins),
    the GPMC unfreezes (2 + WMT) cycles after that and captures,
    the next request comes (CYCLETIME - RDACCESS) units after the unfreeze + 0.11 clk; ASn only rises / falls
    on echo edges (rise: falling CLK7 edges, 0.62 + k after the fall),
    falls only on every other rising edge (parity gate); the new request must find D5q cleared (ASn rise +
    0.2) or WAIT is not asserted for it (merged cycles: hang / bad data).
    """
    u = U * g / CLK
    sync = (2 + wmt) * U / CLK
    d5q = 1.72
    edge = lambda t: 0.62 + int(t - 0.62) + 1  # first echo edge strictly after t
    out = {}
    for kind in ("r", "w"):
        if kind == "r":
            wait_up = d5q + 0.5
            if done:
                asr = edge(d5q) + 0.1
            else:
                cap = wait_up + sync
                xr = cap + (oeoff - racc) * u + 0.2
                asr = edge(xr + 0.1) + 0.1
            cap = wait_up + sync
            nxt = cap + (rcyc - racc) * u + 0.11
        else:
            asr = edge(d5q) + 0.1
            wait_up = asr + 0.1 + 0.4 + 0.1
            cap = wait_up + sync
            nxt = cap + (wcyc - wacc) * u + 0.11
        safe = asr + 0.2 + 0.25  # D5q cleared (CSp reset) + WAIT0 logic
        edge_fall = 2 * (int((nxt + 0.25) / 2) + 1)  # next allowed ASn fall (even clocks after this one)
        out[kind] = dict(
            cap=cap,
            asr=asr,
            nxt=nxt,
            safe=safe,
            period=edge_fall if nxt >= safe else None,
            slack=edge_fall - 2 - (nxt + 0.25) if edge_fall == 4 else None,
        )
    return out


def predicted_ns(m, rd=True):
    pr, pw = m["r"]["period"], m["w"]["period"]
    if pr is None or pw is None:
        return None
    return (pr * CLK, (pw + 2 * pr) * CLK + 30)


def grid(a):
    tr_l = [int(x) for x in a.tr.split(",")]
    tw_l = [int(x) for x in a.tw.split(",")]
    for done, wmt in itertools.product(
        [int(x) for x in a.done.split(",")], [int(x) for x in a.wmt.split(",")]
    ):
        for tr in tr_l:
            for tw in tw_l:
                yield dict(
                    g=a.g, wmt=wmt, racc=a.racc, doe=a.doe, tr=tr, wacc=a.wacc, dwe=a.dwe, tw=tw, done=done
                )


ap = argparse.ArgumentParser(description=__doc__.split("\n")[0])
ap.add_argument("--model", action="store_true")
ap.add_argument("--run", action="store_true")
ap.add_argument("--tr", default="16,12,9,7")
ap.add_argument("--tw", default="8,5,3,2")
ap.add_argument("--wmt", default="1,0")
ap.add_argument("--done", default="0,1")
ap.add_argument(
    "--g",
    type=int,
    default=1,
    choices=(1, 2),
    help="granularity: 1 = 10 ns units (default), 2 = the shipped 20 ns units",
)
ap.add_argument("--racc", type=int, default=None)
ap.add_argument("--wacc", type=int, default=16)
ap.add_argument("--doe", type=int, default=1)
ap.add_argument("--dwe", type=int, default=1)
ap.add_argument(
    "--extra",
    default="",
    help="live ops added to every variant, e.g. RB5:AA (parity gate off), R94:90 (ASn_out as a latch)",
)
add_port_argument(ap)
ap.add_argument("--bundle", default="build/buffee/buffee/buffee_shotgun.lzt")
ap.add_argument("--out", default="tailsweep_results.json")
ap.add_argument("-n", type=int, default=40000, help="reads and writes per variant")
ap.add_argument(
    "--ops",
    default="",
    help='run these labelled live-op strings instead of the grid: "label=op,op,...;label=..." (same ops as the shotgun payload: G<n>:<hex>, R<reg>:<hex>, M<out>:<src>)',
)
ap.add_argument(
    "--show-cfg",
    action="store_true",
    help="print the live CS2 CONFIG1..7 the payload read back for each variant (CONFIG1 bit 4 = TIMEPARAGRANULARITY)",
)
ap.add_argument(
    "--timeout", type=float, default=20, help="seconds to wait for a result before declaring a hang"
)
a = ap.parse_args()
a.port = a.port or default_port()
if a.racc is None:
    a.racc = 14 if a.g == 1 else 12


def build(v):
    c = words(v["g"], v["wmt"], v["racc"], v["doe"], v["tr"], v["wacc"], v["dwe"], v["tw"])
    ops = ["G1:%08X" % c[0], "G2:%08X" % c[1], "G4:%08X" % c[2], "G5:%08X" % c[3], "G6:%08X" % c[4]]
    if v["done"]:
        ops += ["RA1:FA", "RA2:FF"]
    if a.extra:
        ops.append(a.extra)
    return ops


def label(v):
    return "tr%02d_tw%02d_wmt%d_done%d" % (v["tr"], v["tw"], v["wmt"], v["done"])


if a.model or not a.run:
    base = model(**BASE)
    print(
        "shipped timing (x2): predicted read period %.2f clk, write %.2f clk (measured: reads 6.1, writes ~6)"
        % (base["r"]["period"], base["w"]["period"])
    )
    print(
        "%-26s %-10s %-10s  read: capture / ASn rise / next request / safe  write: same"
        % ("variant", "rd,wr clk", "rcyc,wcyc")
    )
    for v in grid(a):
        w = words(v["g"], v["wmt"], v["racc"], v["doe"], v["tr"], v["wacc"], v["dwe"], v["tw"])
        m = model(
            v["g"],
            v["wmt"],
            v["racc"],
            v["racc"] + v["doe"],
            v["racc"] + v["tr"],
            v["wacc"],
            v["wacc"] + v["dwe"],
            v["wacc"] + v["tw"],
            v["done"],
        )
        p = predicted_ns(m)
        f = lambda k: "%.2f/%.2f/%.2f/%.2f" % (m[k]["cap"], m[k]["asr"], m[k]["nxt"], m[k]["safe"])
        print(
            "%-26s %-10s %-10s  %s   %s"
            % (
                label(v),
                "%s,%s" % (m["r"]["period"] or "HANG?", m["w"]["period"] or "HANG?"),
                ("%d,%d" % p) if p else "-",
                f("r"),
                f("w"),
            )
        )
    if not a.run:
        sys.exit(0)


s = open_port(a.port, timeout=0.05)


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
    print("payload up:", next((l.strip() for l in out.split("\n") if l.startswith("SGGPMC")), ""), flush=True)


def recover():
    t = time.time()
    while time.time() - t < 8:
        # the watchdog resets the SoC within 4 s; stay silent through the bootloader's 30 ms erase window
        s.read(2000)
    load()


def run(n, ops):
    s.write(("S %d %s\n" % (n, ",".join(ops + ["N%d" % a.n]))).encode())
    t = time.time()
    out = ""
    done_re = re.compile(r"SGRES %d [^\n]*w2=\d+\r?\n" % n)
    while time.time() - t < a.timeout and not done_re.search(out):
        out += s.read(500).decode("latin1")
    m = done_re.search(out)
    cfg = next((l.strip() for l in out.split("\n") if l.startswith("SGCFG")), "")
    if a.show_cfg and cfg:
        print("   ", cfg.split("  (")[0], flush=True)
    return dict(kv.split("=") for kv in m.group(0).split()[2:]) if m else None


load()
results = []
hung = []
run(0, [])  # warm-up
base = run(1, [])
print("baseline (shipped timing): %s" % base, flush=True)
n = 2
if a.ops:
    for item in a.ops.split(";"):
        label, _, ops = item.partition("=")
        r = run(n, [o for o in ops.split(",") if o])
        n += 1
        if r:
            print(
                "%-10s rcyc %4d (%.2f clk) wcyc %4d  STREAM %4d ns (%.2f clk) sbad %-4s | rbad %-4s wbad %-4s w2 %-4s cbad %-4s"
                % (
                    label,
                    int(r["rcyc"]),
                    int(r["rcyc"]) / CLK,
                    int(r["wcyc"]),
                    int(r["scyc"]),
                    int(r["scyc"]) / CLK,
                    r["sbad"],
                    r["rbad"],
                    r["wbad"],
                    r["w2"],
                    r["cbad"],
                ),
                flush=True,
            )
        else:
            print("%-10s HANG -> recovering" % label, flush=True)
            recover()
    s.write(b"Q\n")
    sys.exit(0)
for v in grid(a):
    if any(
        h["done"] == v["done"] and h["wmt"] == v["wmt"] and v["tr"] <= h["tr"] and v["tw"] <= h["tw"]
        for h in hung
    ):
        print(label(v), "skipped (shorter than a hung one)")
        continue
    r = run(n, build(v))
    n += 1
    m = model(
        v["g"],
        v["wmt"],
        v["racc"],
        v["racc"] + v["doe"],
        v["racc"] + v["tr"],
        v["wacc"],
        v["wacc"] + v["dwe"],
        v["wacc"] + v["tw"],
        v["done"],
    )
    p = predicted_ns(m)
    if r:
        rc, wc = int(r["rcyc"]), int(r["wcyc"])
        print(
            "%-26s rcyc %4d (%.2f clk) wcyc %4d stream %4d (%.2f clk) sbad %-3s rbad %-3s wbad %-3s w2 %-3s cbad %-3s | model %s"
            % (
                label(v),
                rc,
                rc / CLK,
                wc,
                int(r["scyc"]),
                int(r["scyc"]) / CLK,
                r["sbad"],
                r["rbad"],
                r["wbad"],
                r["w2"],
                r["cbad"],
                ("%d,%d" % p) if p else "HANG?",
            ),
            flush=True,
        )
        results.append(
            dict(v, **{k: int(x, 16) if k in ("cxor", "cfirst") else int(x) for k, x in r.items()})
        )
    else:
        print("%-26s HANG -> recovering" % label(v), flush=True)
        hung.append(v)
        results.append(dict(v, hang=True))
        recover()
    json.dump(results, open(a.out, "w"), indent=1)
s.write(b"Q\n")
ok = [
    r
    for r in results
    if not r.get("hang")
    and r["rbad"] == 0
    and r["wbad"] == 0
    and r["w2"] == 0
    and r["cbad"] == 0
    and r["sbad"] == 0
]
print("clean variants, fastest first:")
for r in sorted(ok, key=lambda r: (r["scyc"], r["rcyc"]))[:12]:
    print("  %s stream %d rcyc %d wcyc %d" % (label(r), r["scyc"], r["rcyc"], r["wcyc"]))
