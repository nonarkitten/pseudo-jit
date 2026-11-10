#!/usr/bin/env python3
"""What happens when, relative to CLK7 and the colour clock (CCK): reduces a Logic 2 digital export (digital.csv:
time, one column per enabled channel, a row per change)
of tests/buffee/buffee_bus.c (or any bus traffic) to per-CCK-phase statistics.
  phase_cck.py digital.csv [--clk7 0] [--as 1] [--dtack 2] [--cck 4] [--uds 7] [--rate-check]
Channels are the CSV columns in order of the enabled channels (--cols 0,1,2,3,4,5,7 if the export has those);
the default roles are the capture used so far:
CLK7 echo = ch0, ASn = ch1, DTACKn = ch2, D0 = ch3, CCK = ch4, UDSn = ch7.  Roles are checked against the edge
rates (CLK7 7.09 MHz, CCK 3.55 MHz) and the script says so if a channel is not what it was told.
For each ASn fall it reports where in the CCK period (0..2 CLK7 periods since the last CCK rising edge,
quantised to half periods) the fall happened, how many CLK7
periods later DTACK fell, ASn rose, and UDS/D0 moved; the tables are grouped by that phase."""

import argparse, collections, sys
import numpy as np

ap = argparse.ArgumentParser()
ap.add_argument("csv")
ap.add_argument(
    "--cols",
    default=None,
    help="comma list of the channel numbers the CSV columns are, in order (default: 0,1,2,... as many as there are)",
)
for n, d in (("clk7", 0), ("as", 1), ("dtack", 2), ("cck", 4), ("uds", 7), ("d0", 3)):
    ap.add_argument("--" + n, type=int, default=d)
a = ap.parse_args()
raw = np.genfromtxt(a.csv, delimiter=",", skip_header=1)
t = raw[:, 0]
ncol = raw.shape[1] - 1
cols = [int(x) for x in a.cols.split(",")] if a.cols else list(range(ncol))
col = {c: i + 1 for i, c in enumerate(cols)}


def sig(ch):
    return raw[:, col[ch]].astype(np.int8) if ch in col else None


def edges(ch, level=None):
    v = sig(ch)
    if v is None:
        return np.array([])
    i = np.nonzero(v[1:] != v[:-1])[0] + 1
    if level is not None:
        i = i[v[i] == level]
    return t[i]


span = t[-1] - t[0]
print("capture: %.3f ms, %d rows" % (span * 1e3, len(t)))
for nm, ch, want in (("CLK7", a.clk7, 7.09379e6), ("CCK", a.cck, 3.54690e6)):
    e = edges(ch, 1)
    if len(e) > 10:
        f = len(e) / span
        print(
            "  %-5s ch%d: %.3f MHz rising-edge rate%s"
            % (
                nm,
                ch,
                f / 1e6,
                (
                    ""
                    if abs(f / want - 1) < 0.05
                    else "   <-- NOT the expected %.2f MHz: check --%s" % (want / 1e6, nm.lower())
                ),
            )
        )
    else:
        print("  %-5s ch%d: only %d rising edges: not a clock" % (nm, ch, len(e)))
CLK = 1 / 7.09379e6
cck_r, cck_f, clk_r, clk_f = edges(a.cck, 1), edges(a.cck, 0), edges(a.clk7, 1), edges(a.clk7, 0)
# the fixed relation between the two clocks
if len(cck_r) > 10 and len(clk_r) > 10:
    k = np.searchsorted(clk_r, cck_r) - 1
    ok = (k >= 0) & (k < len(clk_r))
    ph = (cck_r[ok] - clk_r[k[ok]]) / CLK
    print(
        "CCK rising edge, in CLK7 periods after the last CLK7 rising edge: median %.2f  (p5 %.2f p95 %.2f)"
        % (np.median(ph), np.percentile(ph, 5), np.percentile(ph, 95))
    )
    k = np.searchsorted(clk_f, cck_r) - 1
    ok = (k >= 0) & (k < len(clk_f))
    ph = (cck_r[ok] - clk_f[k[ok]]) / CLK
    print(
        "CCK rising edge, in CLK7 periods after the last CLK7 FALLING edge: median %.2f  (p5 %.2f p95 %.2f)"
        % (np.median(ph), np.percentile(ph, 5), np.percentile(ph, 95))
    )
asf, asr, dkf, dkr = (
    edges(a.__dict__["as"], 0),
    edges(a.__dict__["as"], 1),
    edges(a.dtack, 0),
    edges(a.dtack, 1),
)
udf = edges(a.uds, 0) if a.uds in col else np.array([])
d0 = edges(a.d0) if a.d0 in col else np.array([])


def nxt(arr, x):
    i = np.searchsorted(arr, x, side="right")
    return arr[i] if i < len(arr) else None


rows = []
for x in asf:
    r = nxt(asr, x)
    if r is None or r - x > 40 * CLK:
        continue
    d = nxt(dkf, x)
    d = d if d is not None and d < r else None
    u = nxt(udf, x) if len(udf) else None
    u = u if u is not None and u < r else None
    i = np.searchsorted(cck_r, x, side="right") - 1
    j = np.searchsorted(clk_f, x, side="right") - 1
    if i < 0 or j < 0:
        continue
    cph = ((x - cck_r[i]) / CLK) % 2.0  # CLK7 periods since the last CCK rising edge
    kph = (x - clk_f[j]) / CLK  # CLK7 periods since the last CLK7 falling edge
    tr = d0[(d0 > x) & (d0 < r - 0.5 * CLK)] if len(d0) else []
    rows.append(
        (
            cph,
            kph,
            (d - x) / CLK if d else None,
            (r - x) / CLK,
            (u - x) / CLK if u else None,
            (tr[-1] - x) / CLK if len(tr) else None,
        )
    )
print("\n%d ASn cycles" % len(rows))
if not rows:
    sys.exit(
        "no ASn cycles: ASn (--as %d) or DTACK (--dtack %d) never moved while the bus loop ran: check those probes"
        % (a.__dict__["as"], a.dtack)
    )


def q(v, p):
    return np.percentile(v, p)


def line(nm, v):
    v = [y for y in v if y is not None]
    if not v:
        return "   %-26s -" % nm
    return "   %-26s n=%-6d min %5.2f  p10 %5.2f  med %5.2f  p90 %5.2f  max %5.2f" % (
        nm,
        len(v),
        min(v),
        q(v, 10),
        q(v, 50),
        q(v, 90),
        max(v),
    )


print(
    "ASn fall relative to the CLK7 falling edge (CLK7 periods): "
    + ", ".join("%s %.2f" % (n, q([r[1] for r in rows], p)) for n, p in (("p5", 5), ("med", 50), ("p95", 95)))
)
bins = collections.defaultdict(list)
for r in rows:
    bins[int(r[0] * 2 + 0.25) % 4].append(r)
print(
    "\nBy the position of the ASn fall inside the CCK period (0 = CCK rising edge; one CCK period = 2 CLK7 periods; bins of half a CLK7 period):"
)
for b in sorted(bins):
    rs = bins[b]
    print(
        "\n CCK phase %.1f..%.1f CLK7 after the CCK rising edge: %d cycles (%.0f%%)"
        % (b / 2, b / 2 + 0.5, len(rs), 100 * len(rs) / len(rows))
    )
    print(line("DTACK fall after AS fall", [r[2] for r in rs]))
    print(line("AS rise after AS fall", [r[3] for r in rs]))
    print(line("UDS fall after AS fall", [r[4] for r in rs]))
    print(line("last D0 change (data settle)", [r[5] for r in rs]))
    h = collections.Counter(round(r[2] * 2) / 2 for r in rs if r[2] is not None)
    print(
        "   DTACK histogram (CLK7 periods after AS fall): "
        + " ".join("%.1f:%d" % (k2, h[k2]) for k2 in sorted(h))
    )
