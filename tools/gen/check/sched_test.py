#!/usr/bin/env python3
"""Checks tools/study/schedbench.py output: each scheduled + allocated segment must leave the same D slots,
address registers, CCR (incl. X) and guest memory as the original, on random states.
.venv/bin/python tools/gen/check/sched_test.py orig.json sched.json [runs]"""

import sys, os, random, json

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("..", "../..")]
from gen import read_offsets
from sim import Sim
import sim_test as st

offs = read_offsets()
sim = Sim(offs)
rng = random.Random(7)
A = json.load(open(sys.argv[1]))
B = json.load(open(sys.argv[2]))
runs = int(sys.argv[3]) if len(sys.argv) > 3 else 50
S, AR = st.AREA, st.AREA_LEN
bad = tested = faults = 0
for k, (a, b) in enumerate(zip(A, B)):
    if not b.get("sched") or a["words"] == b["words"]:
        continue
    tested += 1
    for t in range(runs):
        d = [rng.getrandbits(32) for _ in range(8)]
        ar = [S + 0x800] * 7 + [S + 0x900]
        ccr = rng.getrandbits(5)
        area = bytes(rng.getrandbits(8) for _ in range(AR))
        r1 = rng.getrandbits(32)
        r2 = rng.getrandbits(32)
        res = []
        for ws in (a["words"], b["words"]):
            try:
                res.append(sim.run([int(w) for w in ws] + [0xE12FFF1E], d, ar, ccr, area=area, r1=r1, r2=r2))
            except Exception as e:
                res.append("fault")
        if res[0] == "fault":
            faults += 1
            continue
        if res[0] != res[1]:
            bad += 1
            print("MISMATCH seg", k)
            break
print("%d segments tested, %d mismatches, %d faulting runs skipped" % (tested, bad, faults))
sys.exit(1 if bad else 0)
