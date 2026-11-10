#!/usr/bin/env python3
"""The exception decisions the oracle harness cannot follow (its trap path runs off into the vector table): CHK
traps (vector 6) when Dn is
negative or above the bound, DIVU/DIVS trap (vector 5) on a zero divisor, nothing else traps. The sequence is
run under Unicorn on
random operands (both outcomes are common) and the `svc` it executes is compared with the independent model's
verdict.

  .venv/bin/python tools/gen/check/trap_test.py [cases per opcode]"""

import sys, os, random

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("..", "../..")]
from gen import Gen, read_offsets
from sim import Sim
import sim_test as T
import model68k

per_op = int(sys.argv[1]) if len(sys.argv) > 1 else 8
offs = read_offsets()
g = Gen(offs)
sim = Sim(offs)
rng = random.Random(5)
VECTOR = {"emit_CHK": 6, "emit_DIVU": 5, "emit_DIVS": 5}
n = traps = 0
bad = []
for op in range(65536):
    name = g.winner[op]
    if name not in VECTOR:
        continue
    rl = T.roles(name, op)
    if rl is None:
        continue
    for _ in range(per_op):
        c = T.Case(rng)
        for role, mode, reg, w in rl:
            c.add(role, mode, reg, w)
        c.op = op
        if name == "emit_CHK" and rng.random() < 0.5:  # half the cases in range, so both outcomes are common
            m = model68k.Model(c.d, c.a, c.ccr, c.area, T.AREA, c.ext, T.OP_ADDR)
            bound = T.sext(m.read(m.ea(rl[0][1], op & 7, 2), 2), 16)
            k = (op >> 9) & 7
            if bound >= 0 and k not in c.d_set:
                c.d[k] = (c.d[k] & ~0xFFFF) | rng.randrange(0, bound + 1)
        ref = model68k.run_case(c, name, op, rl, T.AREA, T.OP_ADDR)
        r = sim.run(g.generate(op), c.d, c.a, c.ccr, area=c.area, r1=c.r["src"], r2=c.r["dst"])
        got = sim.trapped if r is None else None
        want = VECTOR[name] if ref == "trap" else None
        n += 1
        traps += want is not None
        if got != want:
            bad.append((op, name, got, want))
# privileged instructions in user mode (the SR's S bit clear) trap with vector 8; MOVE SR,<ea> is not
# privileged on a 68000
PRIV = (
    "emit_MOVE_TO_SR",
    "emit_ANDI_TO_SR",
    "emit_ORI_TO_SR",
    "emit_EORI_TO_SR",
    "emit_MOVE_TO_USP",
    "emit_MOVE_USP_TO",
    "emit_STOP",
    "emit_RESET",
    "emit_RTE",
)
np_ = 0
for op in range(65536):
    name = g.winner[op]
    if name not in PRIV + ("emit_MOVE_SR_TO",):
        continue
    rl = T.roles(name, op)
    if rl is None and name in ("emit_STOP", "emit_RESET", "emit_RTE"):
        rl = []
    if rl is None:
        continue
    for _ in range(max(1, per_op // 4)):
        c = T.Case(rng)
        for role, mode, reg, w in rl:
            c.add(role, mode, reg, w)
        r = sim.run(g.generate(op), c.d, c.a, c.ccr, area=c.area, r1=c.r["src"], r2=c.r["dst"], sr_hi=0x07)
        got = sim.trapped if r is None else None
        want = 8 if name in PRIV else None
        n += 1
        np_ += want is not None
        if got != want:
            bad.append((op, name + " (user mode)", got, want))
traps += np_
print(f"{n} cases ({traps} should trap): " + ("all trap decisions right" if not bad else f"{len(bad)} wrong"))
for b in bad[:8]:
    print(f"  {b[0]:04X} {b[1]}: svc {b[2]}, expected {b[3]}")
sys.exit(1 if bad else 0)
