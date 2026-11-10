#!/usr/bin/env python3
"""Compares two tools/bbb/corpus_scan.sh outputs per program (mean over the code layouts) and as a geometric
mean:
tools/study/corpus_compare.py base.txt pin.txt"""

import sys, collections, math


def load(path):
    d = collections.defaultdict(lambda: collections.defaultdict(list))
    for l in open(path):
        p = l.split()
        if len(p) >= 6 and p[1] == "pad" and p[4].isdigit() and p[5].isdigit():
            d[p[3]]["cold"].append(int(p[4]))
            d[p[3]]["warm"].append(int(p[5]))
            if len(p) >= 9:
                d[p[3]]["arm"].append(int(p[6]))
                d[p[3]]["mis"].append(int(p[7]))
                d[p[3]]["l1d"].append(int(p[8]))
    return d


a, b = load(sys.argv[1]), load(sys.argv[2])
print(
    "%-16s %9s %9s %7s | %9s %9s %7s | layouts"
    % ("program", "base warm", "new warm", "change", "base cold", "new cold", "change")
)
gw, gc, n = 0.0, 0.0, 0
for k in a:
    if k not in b:
        continue
    aw, bw = sum(a[k]["warm"]) / len(a[k]["warm"]), sum(b[k]["warm"]) / len(b[k]["warm"])
    ac, bc = sum(a[k]["cold"]) / len(a[k]["cold"]), sum(b[k]["cold"]) / len(b[k]["cold"])
    extra = ""
    if a[k]["arm"] and b[k]["arm"]:
        m = lambda d, f: sum(d[k][f]) / len(d[k][f])
        extra = "  arm %7.0f->%7.0f  mispredicts %5.0f->%5.0f  L1D misses %5.0f->%5.0f" % (
            m(a, "arm"),
            m(b, "arm"),
            m(a, "mis"),
            m(b, "mis"),
            m(a, "l1d"),
            m(b, "l1d"),
        )
    print(
        "%-16s %9.0f %9.0f %+6.1f%% | %9.0f %9.0f %+6.1f%% | %d/%d%s"
        % (
            k,
            aw,
            bw,
            100 * (bw / aw - 1),
            ac,
            bc,
            100 * (bc / ac - 1),
            len(a[k]["warm"]),
            len(b[k]["warm"]),
            extra,
        )
    )
    gw += math.log(bw / aw)
    gc += math.log(bc / ac)
    n += 1
if n:
    print(
        "geometric mean over %d programs: warm %+.2f%%, cold %+.2f%%"
        % (n, 100 * (math.exp(gw / n) - 1), 100 * (math.exp(gc / n) - 1))
    )
