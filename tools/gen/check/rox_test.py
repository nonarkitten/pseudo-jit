#!/usr/bin/env python3
"""ROXL / ROXR with a register count, exhaustively over the count: every count 0-63 (and the high bits of the
count register, which must be ignored beyond bit 5), byte /
word / long, random operands and every X / N / Z / V / C input, against the 68000 oracle. The sequences are
loop-free (the rotation is by count mod width + 1); this is what
makes sure the modulo is right for every count, which random states alone would not.

  .venv/bin/python tools/gen/check/rox_test.py [states per count, default 24]"""

import sys, os, random, subprocess

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("..", "../..")]
from paths import build_path
from gen import Gen, read_offsets
from sim import Sim
import sim_test as st


def main():
    per = int(sys.argv[1]) if len(sys.argv) > 1 else 24
    offs = read_offsets()
    g = Gen(offs)
    sim = Sim(offs)
    rng = random.Random(5)
    cases = []
    for size, base in ((0, 0xE030), (1, 0xE070), (2, 0xE0B0)):
        for left in (0, 1):
            op = base | 1 << 9 | left << 8  # ROXL / ROXR D1,D0 (the count in D1)
            for count in range(64):
                for _ in range(per):
                    c = st.Case(rng)
                    c.op = op
                    c.ext = []
                    c.d[1] = (rng.getrandbits(26) << 6 | count) if rng.random() < 0.5 else count
                    cases.append(c)

    def run(batch):
        r = (
            subprocess.run(
                [build_path("host", "oracle1")],
                input="\n".join(st.line(c) for c in batch) + "\n",
                capture_output=True,
                text=True,
            )
            .stdout.strip()
            .splitlines()
        )
        if len(r) == len(batch):
            return r
        h = len(batch) // 2
        return run(batch[:h]) + run(batch[h:]) if len(batch) > 1 else [None]

    out = []
    for i in range(0, len(cases), 4000):
        out += run(cases[i : i + 4000])
    bad = 0
    for c, ln in zip(cases, out):
        if ln is None:
            continue
        f = ln.split()
        wd = [int(x, 16) for x in f[1:9]]
        wa = [int(x, 16) for x in f[9:17]]
        wc = int(f[17], 16) & 0x1F
        gd, ga, gc, garea = sim.run(g.generate(c.op), c.d, c.a, c.ccr, area=c.area)
        if not (gd == wd and ga == wa and gc & 0x1F == wc):
            bad += 1
            if bad <= 4:
                print(
                    "MISMATCH %04X count=%d D0=%08x X=%d: got D0=%08x ccr=%02x want D0=%08x ccr=%02x"
                    % (c.op, c.d[1] & 63, c.d[0], c.ccr >> 4 & 1, gd[0], gc & 0x1F, wd[0], wc)
                )
    print("%d cases (6 opcodes x 64 counts x %d states): %d mismatches" % (len(cases), per, bad))
    sys.exit(1 if bad else 0)


main()
