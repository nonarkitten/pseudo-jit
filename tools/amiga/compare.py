#!/usr/bin/env python3
"""compare.py pjit.log oracle.log -- the first snapshot (instruction count, hardware hash, PC, D0-D7, A0-A7,
SR) at which the two differ.
"""

import sys


def load(p):
    d = {}
    for l in open(p, errors="replace"):
        if l.startswith("S "):
            f = l.split()
            if len(f) >= 21:
                d[int(f[1])] = f
    return d


a = load(sys.argv[1])
b = load(sys.argv[2])
names = "S icount hash PC D0 D1 D2 D3 D4 D5 D6 D7 A0 A1 A2 A3 A4 A5 A6 A7 SR M0 M1 M2 M3 M4 M5 M6 M7".split()
common = sorted(set(a) & set(b))
for k in common:
    if a[k][2:] != b[k][2:]:
        print("first difference at instruction %d (%d snapshots equal before it)" % (k, common.index(k)))
        for i in range(2, min(len(a[k]), len(b[k]), len(names))):
            if a[k][i] != b[k][i]:
                print("  %-4s pjit %s  oracle %s" % (names[i], a[k][i], b[k][i]))
        prev = [c for c in common if c < k]
        if prev:
            print("  last equal snapshot: %d  PC %s" % (prev[-1], a[prev[-1]][3]))
        sys.exit(1)
print("no difference in %d common snapshots (pjit %d, oracle %d)" % (len(common), len(a), len(b)))
