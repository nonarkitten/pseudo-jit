#!/usr/bin/env python3
"""Per-opcode condition-code behaviour, measured on the 68000 oracle (golden/mcl68k.c) instead of written down:
for each opcode a family of
sim_test knows, a handful of random and edge-case states are run once as given and once with each input CCR
bit flipped, and
  reads_x / reads_nzvc  = flipping an input bit of that group changes *any* output (a register, memory, or an
  output flag: so a bit the
                          instruction only preserves counts as read -- the previous instruction's value has to
                          be there),
  writes_x / writes_nzvc = an output flag of that group differs from its input in some state.
Two groups: X, and N Z V C together. A group is *killed* by an instruction
that writes it and does not read it.
Opcodes sim_test has no EA model for (branches, jumps, traps, ...) get the hand-written entries in HAND, or
the safe default (reads everything,
writes nothing). The result is build/flagmeta.json: {opcode: [reads_nzvc, writes_nzvc, reads_x, writes_x]} as
0/1.

  .venv/bin/python tools/gen/check/flagmeta.py [states per opcode, default 24]      (needs build/oracle1, build/asm_offsets.inc)
"""

import sys, os, re, random, subprocess, json, collections

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("..", "../..")]
from paths import build_path
from gen import Gen, read_offsets
import sim_test as st

NZVC, X = 0x0F, 0x10


def group_key(name, op):
    """Opcodes that differ only in their register numbers behave alike for the condition codes: one
    representative per (emitter, size, modes).
    """
    k = op & ~0x0E07
    if (op >> 3) & 7 == 7:
        k |= op & 7  # EA mode 7: the register field picks absolute / PC-relative / immediate
    if (op >> 12) in (1, 2, 3) and (op >> 6) & 7 == 7:
        k |= (op >> 9) & 7  # MOVE to mode 7: the same, for the destination
    return (name, k)


def probe(per_op):
    offs = read_offsets()
    g = Gen(offs)
    rng = random.Random(1989)
    groups = {}
    for op in range(65536):
        name = g.winner[op]
        rl = st.roles(name, op)
        if rl is None or g.generate(op) is None:
            continue
        groups.setdefault(group_key(name, op), []).append(op)
    reps = [ops[0] for ops in groups.values()]
    cases = []
    for op in reps:
        rl = st.roles(g.winner[op], op)
        for _ in range(per_op):
            cases.append(st.make_case(rng, op, rl))
    variants = []
    for c in cases:
        for flip in (0, 1, 2, 4, 8, 16):
            variants.append((c, c.ccr ^ flip))

    def line(c, ccr):
        saved = c.ccr
        c.ccr = ccr
        try:
            return st.line(c)
        finally:
            c.ccr = saved

    def oracle(batch):
        r = (
            subprocess.run(
                [build_path("host", "oracle1")],
                input="\n".join(line(c, ccr) for c, ccr in batch) + "\n",
                capture_output=True,
                text=True,
            )
            .stdout.strip()
            .splitlines()
        )
        if len(r) == len(batch):
            return r
        if len(batch) == 1:
            return [None]
        h = len(batch) // 2
        return oracle(batch[:h]) + oracle(batch[h:])

    out = []
    for i in range(0, len(variants), 6000):
        out += oracle(variants[i : i + 6000])
    rep_meta = {}
    k = 0
    for c in cases:
        rows = out[k : k + 6]
        vin = [c.ccr ^ fl for fl in (0, 1, 2, 4, 8, 16)]
        k += 6
        if any(r is None for r in rows):
            continue
        f = [r.split() for r in rows]
        base = f[0]
        m = rep_meta.setdefault(c.op, [0, 0, 0, 0])
        for flip, fx in zip((1, 2, 4, 8, 16), f[1:]):
            differs = any(base[i] != fx[i] for i in list(range(1, 17)) + [19]) or (
                int(base[17], 16) & 0x1F
            ) != (int(fx[17], 16) & 0x1F)
            if differs:
                m[2 if flip == 16 else 0] = 1
        for ccr_in, r in zip(vin, f):  # writes: an output flag that is not its own input, in any variant
            d = (int(r[17], 16) ^ ccr_in) & 0x1F
            if d & NZVC:
                m[1] = 1
            if d & X:
                m[3] = 1
    meta = {}
    for ops in groups.values():
        if ops[0] in rep_meta:
            for op in ops:
                meta[op] = rep_meta[ops[0]]
    return meta


def cond_reads(cc):
    """The condition codes' own use of NZVC: 0 = true / false (reads nothing)."""
    return 0 if cc in (0, 1) else 1


def hand(op):
    """[reads_nzvc, writes_nzvc, reads_x, writes_x] for what the EA model does not cover, or None."""
    hi = op >> 12
    if hi == 6:
        return [cond_reads((op >> 8) & 15), 0, 0, 0]  # Bcc / BRA / BSR
    if op & 0xF0F8 == 0x50C8:
        return [cond_reads((op >> 8) & 15) if (op >> 8) & 15 != 1 else 0, 0, 0, 0]  # DBcc (DBF reads nothing)
    if op & 0xFF80 == 0x4E80 or op == 0x4E75:
        return [0, 0, 0, 0]  # JSR JMP RTS: the flags pass through (JMP is 0x4EC0)
    if op & 0xFFC0 == 0x4EC0:
        return [0, 0, 0, 0]
    if op == 0x4E71:
        return [0, 0, 0, 0]  # NOP
    return None


if __name__ == "__main__":
    per_op = int(sys.argv[1]) if len(sys.argv) > 1 else 24
    meta = probe(per_op)
    table = {}
    g = Gen(read_offsets())
    for op in range(65536):
        h = hand(op)
        # unknown: reads everything, writes nothing
        table[op] = h if h is not None else meta.get(op, [1, 0, 1, 0])
        # a dependence that random states rarely reach is a read the probe can miss (and a missed read means a
        # needed flag gets dropped):
        # ADDX SUBX NEGX ABCD SBCD NBCD keep Z unless the result is non-zero
        if re.match(r"emit_(ADDX|SUBX|NEGX|ABCD|SBCD|NBCD)", g.winner[op] or ""):
            table[op] = [1, 1, 1, 1]
    os.makedirs(build_path("host"), exist_ok=True)
    json.dump(table, open(build_path("host", "flagmeta.json"), "w"))
    c = collections.Counter(tuple(v) for v in table.values())
    print(
        "%d opcodes measured, %d by hand, %d left at the safe default"
        % (
            len(meta),
            sum(1 for op in range(65536) if hand(op) is not None),
            65536 - len(meta) - sum(1 for op in range(65536) if hand(op) is not None and op not in meta),
        )
    )
    print("[reads_nzvc, writes_nzvc, reads_x, writes_x] -> opcodes:")
    for k, n in c.most_common():
        print("   ", k, n)
