#!/usr/bin/env python3
"""Validates tools/gen/constfold.py: for every opcode sim_test models, the sequence preceded by a prefix that
loads constant extension values into r1 / r2
(as the block assembler does) is run as it is and after constant folding (and the dead-code pass that follows
it), on random and edge-case states under Unicorn;
every register, the memory area and every flag must be identical. Also reports how many words the folding
removes.

  .venv/bin/python tools/gen/check/fold_test.py [states per opcode, default 4]"""

import sys, os, random, collections

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("..", "../..")]
from gen import Gen, read_offsets
from sim import Sim
import sim_test as st
import armdce, constfold, blockasm


def random_word(rng):
    r = rng.random()
    cond = 14 if rng.random() < 0.85 else rng.randrange(0, 14)
    rd, rn, rm = rng.randrange(4), rng.randrange(4), rng.randrange(4)
    if r < 0.55:  # data processing
        opc = rng.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15])
        s_ = 1 if opc in (8, 9, 10, 11) else rng.randrange(2)
        if s_ and rng.random() < 0.3:
            cond = 14
        if opc in (13, 15):
            rn = 0  # mov / mvn: Rn is not used (should be zero)
        if opc in (8, 9, 10, 11):
            rd = 0  # compares and tests: Rd is not used
        if rng.random() < 0.3:
            return (
                cond << 28
                | 1 << 25
                | opc << 21
                | s_ << 20
                | rn << 16
                | rd << 12
                | rng.randrange(16) << 8
                | rng.randrange(256)
            )
        kind = rng.randrange(4)
        amt = rng.choice([0, 0, 1, 2, 4, 7, 8, 16, 24, 31])
        return cond << 28 | opc << 21 | s_ << 20 | rn << 16 | rd << 12 | amt << 7 | kind << 5 | rm
    if r < 0.65:
        return cond << 28 | rng.choice([0x06AF0070, 0x06BF0070, 0x06EF0070, 0x06FF0070]) | rd << 12 | rm
    if r < 0.72:
        # movw
        return 14 << 28 | 0x03000000 | (rng.getrandbits(16) >> 12) << 16 | rd << 12 | rng.getrandbits(12)
    if r < 0.76:
        # movt
        return 14 << 28 | 0x03400000 | (rng.getrandbits(16) >> 12) << 16 | rd << 12 | rng.getrandbits(12)
    if r < 0.90:  # ldr / str / ldrb / strb [r6, +/-rm] or [r6, #imm]
        load = rng.randrange(2)
        byte = rng.randrange(2)
        u = rng.randrange(2)
        reg_form = rng.random() < 0.7
        base = (
            cond << 28
            | (0x06000000 if reg_form else 0x04000000)
            | 1 << 24
            | u << 23
            | byte << 22
            | load << 20
            | 6 << 16
            | rd << 12
        )
        return base | (rm if reg_form else rng.randrange(0x40))
    load = rng.randrange(2)
    sh = rng.choice([1, 2, 3]) if load else 1
    u = rng.randrange(2)  # ldrh / ldrsb / ldrsh / strh
    if rng.random() < 0.7:
        return cond << 28 | 1 << 24 | u << 23 | load << 20 | 6 << 16 | rd << 12 | 0x90 | sh << 5 | rm
    off = rng.randrange(0x40)
    return (
        cond << 28
        | 1 << 24
        | u << 23
        | 1 << 22
        | load << 20
        | 6 << 16
        | rd << 12
        | (off >> 4) << 8
        | 0x90
        | sh << 5
        | (off & 15)
    )


def fuzz(sim, n, seed=11):
    """Random ARM sequences with known constants flowing through data-processing and load / store forms; the
    original and the folded version must leave every
    register, the flags and the memory area the same."""
    from unicorn.arm_const import UC_ARM_REG_R0

    rng = random.Random(seed)
    bad = 0
    changed = 0
    S, AR = st.AREA, st.AREA_LEN
    rand_word = lambda: random_word(rng)
    for t in range(n):
        if len(sim.slot) > 30000:
            sim.__init__(read_offsets())
        words = []
        for rr in range(4):  # constants first: small ones (safe offsets) and large ones
            v = (
                rng.choice(
                    [
                        0,
                        1,
                        2,
                        3,
                        0x10,
                        0x20,
                        0x3E,
                        -2,
                        -0x20,
                        0x7F,
                        0xFF,
                        0x100,
                        0xFFFF,
                        0x12345678,
                        0x80000000,
                        0xFFFFFFFF,
                        0xFF000000,
                        0x1FE,
                    ]
                )
                if rng.random() < 0.7
                else rng.getrandbits(32)
            )
            words += blockasm.load_const(rr, v & 0xFFFFFFFF)
        words += [rand_word() for _ in range(rng.randrange(2, 10))]
        folded = constfold.fold(words)
        changed += folded != words
        d = [rng.getrandbits(32) for _ in range(8)]
        a = [S + 0x800] * 7 + [S + 0x900]
        ccr = rng.getrandbits(5)
        area = bytes(rng.getrandbits(8) for _ in range(AR))
        outs = []
        for ws in (words, folded):
            try:
                r = sim.run(ws + [0xE12FFF1E], d, a, ccr, area=area)
                regs = [sim.uc.reg_read(UC_ARM_REG_R0 + k) for k in range(4)]
                outs.append((r, regs))
            except Exception as e:
                outs.append(("fault",))
        if outs[0] != outs[1]:
            bad += 1
            if bad <= 3:
                print(
                    "FUZZ MISMATCH",
                    [hex(w) if isinstance(w, int) else w for w in words],
                    "->",
                    [hex(w) for w in folded],
                )
    print("fuzz: %d sequences, %d changed by folding, %d mismatches" % (n, changed, bad))
    return bad


if __name__ == "__main__":
    per_op = int(sys.argv[1]) if len(sys.argv) > 1 else 4
    offs = read_offsets()
    g = Gen(offs)
    sim = Sim(offs)
    rng = random.Random(7)
    words_in = words_out = nops = changed = 0
    bad = collections.Counter()
    first = {}
    for op in range(65536):
        name = g.winner[op]
        rl = st.roles(name, op)
        if rl is None:
            continue
        seq = g.generate(op)
        if seq is None:
            continue
        if len(sim.slot) > 30000:
            sim = Sim(offs)
        cases = [st.make_case(rng, op, rl) for _ in range(per_op)]
        # the prefix is built per case (its constants differ): fold each, compare each
        for c in cases:
            pre = []
            if any(r == "src" for r, *_ in rl) or True:
                pre += blockasm.load_const(1, c.r["src"] & 0xFFFFFFFF) + blockasm.load_const(
                    2, c.r["dst"] & 0xFFFFFFFF
                )
            words = pre + list(seq)
            folded = constfold.fold(words)
            try:
                folded, _ = armdce.dce(folded, 3)
            except ValueError:
                pass
            nops += 1
            words_in += len(words)
            words_out += len(folded)
            changed += folded != words
            full = sim.run(words, c.d, c.a, c.ccr, area=c.area)
            got = sim.run(folded, c.d, c.a, c.ccr, area=c.area)
            if full is None or got is None:
                ok = (full is None) == (got is None)
            else:
                ok = full[0] == got[0] and full[1] == got[1] and full[2] == got[2] and full[3] == got[3]
            if not ok:
                k = name
                bad[k] += 1
                first.setdefault(k, (op, c.ccr, c.r["src"], c.r["dst"]))
    print(
        "%d runs, %d words in, %d words out (%.1f%% fewer), %d changed"
        % (nops, words_in, words_out, 100 * (1 - words_out / words_in), changed)
    )
    if bad:
        print("MISMATCHES between the sequence and its folded form:")
        for k, n in sorted(bad.items()):
            print("  %s: %d, e.g. %04X ccr=%x r1=%x r2=%x" % ((k, n) + first[k]))
    else:
        print("every folded sequence agrees with the original on every output")
    nb = fuzz(sim, 20000)
    sys.exit(1 if bad or nb else 0)
