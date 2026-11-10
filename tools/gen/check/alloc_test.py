#!/usr/bin/env python3
"""Fuzzes tools/gen/regalloc.py: random ARM sequences over r0-r3 (constants first, then random data-processing /
extend / load / store words, then stores of r0-r3 so
their final values are observable) are run as they are and after allocation into a shuffled pool of 3-5
registers that does NOT prefer the registers the words
came with, so values really move and share registers. Registers, flags and memory (including what was stored)
must agree.

  .venv/bin/python tools/gen/check/alloc_test.py [sequences, default 20000]"""

import sys, os, random

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("..", "../..")]
from gen import read_offsets
from emitters.base import PIN_D0
from sim import Sim
import sim_test as st, blockasm, regalloc, fold_test


def main():
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 20000
    offs = read_offsets()
    sim = Sim(offs)
    rng = random.Random(5)
    S, AR = st.AREA, st.AREA_LEN
    bad = refused = used_pool = 0
    peaks = {}
    for t in range(n):
        if len(sim.slot) > 30000:
            sim = Sim(offs)
        words = []
        for rr in range(4):
            v = (
                rng.choice(
                    [
                        0,
                        1,
                        2,
                        3,
                        0x10,
                        0x20,
                        -2,
                        0x7F,
                        0xFF,
                        0x100,
                        0xFFFF,
                        0x12345678,
                        0x80000000,
                        0xFFFFFFFF,
                    ]
                )
                if rng.random() < 0.7
                else rng.getrandbits(32)
            )
            words += blockasm.load_const(rr, v & 0xFFFFFFFF)
        words += [fold_test.random_word(rng) for _ in range(rng.randrange(4, 16))]
        # str rk,[r6,#0x40+4k]: the final values are observable
        words += [0xE5860040 | k << 12 | 4 * k for k in range(4)]
        # with D0 pinned in r4 (the default) the allocator's pool is r0-r3; the unpinned design may also use r4
        candidates = [0, 1, 2, 3] if PIN_D0 else [0, 1, 2, 3, 4]
        size = rng.randrange(3, len(candidates) + 1)
        pool = rng.sample(candidates, size)
        out, peak = regalloc.allocate(words, pool=tuple(pool), prefer=False)
        if out is None:
            refused += 1
            continue
        peaks[peak] = peaks.get(peak, 0) + 1
        d = [rng.getrandbits(32) for _ in range(8)]
        a = [S + 0x800] * 7 + [S + 0x900]
        ccr = rng.getrandbits(5)
        area = bytes(rng.getrandbits(8) for _ in range(AR))
        res = []
        for ws in (words, out):
            try:
                res.append(sim.run(ws + [0xE12FFF1E], d, a, ccr, area=area))
            except Exception as e:
                res.append("fault")
        if res[0] != res[1]:
            bad += 1
            if bad <= 3:
                print("MISMATCH", [hex(w) for w in words], "->", [hex(w) for w in out])
    print(
        "%d sequences, %d refused (pressure above the pool, or an encoding it does not know), %d mismatches; peak live values: %s"
        % (n, refused, bad, dict(sorted(peaks.items())))
    )
    sys.exit(1 if bad else 0)


main()
