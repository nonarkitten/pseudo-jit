#!/usr/bin/env python3
"""Validates tools/gen/armdce.py: for every opcode sim_test models, the sequence cut down for each live-out flag
set (none, N Z V C, X, both) must leave every
register, the memory area and every *wanted* flag exactly as the full sequence does, on random and edge-case
states under Unicorn. Also reports how much it cuts,
and checks the flag groups it says are wanted before the sequence (with both groups wanted after) against
build/flagmeta.json, which came from the 68000 oracle.

  .venv/bin/python tools/gen/check/dce_test.py [states per opcode, default 4]       (needs build/helpers.bin, build/flagmeta.json)
"""

import sys, os, random, json, collections

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("..", "../..")]
from paths import build_path
from gen import Gen, read_offsets
from sim import Sim
import sim_test as st
import armdce

if __name__ == "__main__":
    per_op = int(sys.argv[1]) if len(sys.argv) > 1 else 4
    offs = read_offsets()
    g = Gen(offs)
    sim = Sim(offs)
    meta = json.load(open(build_path("host", "flagmeta.json")))
    rng = random.Random(1990)
    words_full = words_var = [0] * 4
    total_full = 0
    total_var = [0] * 4
    shrunk = [0] * 4
    nops = 0
    bad = collections.Counter()
    first = {}
    meta_bad = collections.Counter()
    meta_first = {}
    for op in range(65536):
        name = g.winner[op]
        rl = st.roles(name, op)
        if rl is None:
            continue
        seq = g.generate(op)
        if seq is None:
            continue
        if len(sim.slot) > 40000:
            sim = Sim(offs)  # the simulator hands out one code address per distinct sequence
        nops += 1
        total_full += len(seq)
        variants = []
        for L in range(4):
            try:
                v, live_in = armdce.dce(seq, L)
            except ValueError:
                v, live_in = list(seq), 3  # backward branch: leave it alone
            variants.append((v, live_in))
            total_var[L] += len(v)
            shrunk[L] += len(v) < len(seq)
        # the oracle-derived table: with both groups wanted afterwards, what is wanted before is "read or
        # preserved"
        m = meta[str(op)]
        want = (1 if m[0] else 0) | (2 if m[2] else 0)
        if variants[3][1] != want:
            k = (name, variants[3][1], want)
            meta_bad[k] += 1
            meta_first.setdefault(k, op)
        for _ in range(per_op):
            c = st.make_case(rng, op, rl)
            full = sim.run(seq, c.d, c.a, c.ccr, area=c.area, r1=c.r["src"], r2=c.r["dst"])
            for L in range(4):
                got = sim.run(variants[L][0], c.d, c.a, c.ccr, area=c.area, r1=c.r["src"], r2=c.r["dst"])
                if full is None or got is None:
                    ok = (full is None) == (got is None)
                else:
                    mask = 0xFF00 | (armdce.GROUP_NZCV and (0x0F if L & 1 else 0)) | (0x10 if L & 2 else 0)
                    ok = (
                        full[0] == got[0]
                        and full[1] == got[1]
                        and full[3] == got[3]
                        and (full[2] & mask) == (got[2] & mask)
                    )
                if not ok:
                    k = (name, L)
                    bad[k] += 1
                    first.setdefault(
                        k, (op, c.ccr, [hex(x) for x in c.d], full and hex(full[2]), got and hex(got[2]))
                    )
    print("%d opcodes, %d words in the full sequences" % (nops, total_full))
    for L in range(4):
        print(
            "  live-out %d (%s%s): %d words (%.1f%%), %d opcodes shorter"
            % (
                L,
                "NZVC " if L & 1 else "",
                "X" if L & 2 else "",
                total_var[L],
                100 * total_var[L] / total_full,
                shrunk[L],
            )
        )
    if bad:
        print("MISMATCHES against the full sequence:")
        for k, n in sorted(bad.items()):
            print("  %s live-out %d: %d, e.g. %04X ccr=%x d=%s full=%s got=%s" % (k[0], k[1], n, *first[k]))
    else:
        print("every variant agrees with the full sequence on every wanted output")
    if meta_bad:
        print("live-in flags that differ from the oracle-derived table (dce said, table says):")
        for k, n in sorted(meta_bad.items()):
            print("  %s: %s vs %s, %d opcodes, e.g. %04X" % (k[0], k[1], k[2], n, meta_first[k]))
    else:
        print("the live-in flag groups agree with the oracle-derived table for every opcode")
    sys.exit(1 if bad else 0)
