#!/usr/bin/env python3
"""Fuzzes tools/gen/known.py against the 68000 oracle: random short programs (a call-like push of a return
address that points at random "inline data", then
instructions the evaluator models -- moves from the data through an address register, MOVEQ, CLR, ADDA, ADDQ,
LEA, popping the return address into a register,
adjusting it in place -- mixed with ones it must invalidate on: stores, NEG, EXG, MOVEM, indexed modes, pushes
and pops of unknown values). After every instruction each
register and stack slot the evaluator says it knows must equal the oracle's real value.

  .venv/bin/python tools/gen/check/known_test.py [programs, default 3000]"""

import sys, os, random, subprocess

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("..", "../..")]
import block_test as bt, blockasm, known, a7

AREA = bt.AREA
DATA = AREA + 0x700  # where the "inline data" (the bytes after the call) lives


def build_program(rng):
    """-> (list of words per instruction, in order)."""
    prog = []
    n = rng.randrange(4, 14)
    for _ in range(n):
        r = rng.random()
        a = rng.choice([1, 2, 3])
        d = rng.choice([1, 2, 4])
        d2 = rng.choice([1, 2, 4])
        if r < 0.10:
            # MOVEA.L (A7)+,An
            prog.append([0x205F | rng.choice([1, 2, 3]) << 9 if False else (0x2000 | (a << 9) | 0x40 | 0x1F)])
        elif r < 0.16:
            prog.append([0x201F | d << 9])  # MOVE.L (A7)+,Dn
        elif r < 0.30:
            prog.append([0x1018 | d << 9 | a])  # MOVE.B (An)+,Dn
        elif r < 0.36:
            prog.append([0x1010 | d << 9 | a])  # MOVE.B (An),Dn
        elif r < 0.44:
            prog.append([0x1019 if False else (0x1010 | d << 9 | a)])  # MOVE.B (An),Dn
        elif r < 0.50:
            prog.append([0x7000 | d << 9 | rng.randrange(256)])  # MOVEQ #n,Dn
        elif r < 0.55:
            prog.append([0x4280 | d])  # CLR.L Dn
        elif r < 0.58:
            prog.append([0x4240 | d])  # CLR.W Dn
        elif r < 0.60:
            prog.append([0x4200 | d])  # CLR.B Dn
        elif r < 0.66:
            prog.append([0xD1C0 | a << 9 | d])  # ADDA.L Dn,An
        elif r < 0.70:
            prog.append([0xD0C0 | a << 9 | d])  # ADDA.W Dn,An
        elif r < 0.73:
            prog.append([0x5288 | rng.randrange(8) << 9 | a])  # ADDQ.L #n,An
        elif r < 0.76:
            prog.append([0x5188 | rng.randrange(8) << 9 | a])  # SUBQ.L #n,An
        elif r < 0.80:
            prog.append([0x41E8 | a << 9 | a2(rng), rng.randrange(0, 12)])  # LEA d16(Am),An
        elif r < 0.83:
            prog.append([0x2008 | d << 9 | a2(rng)])  # MOVE.L Am,Dn
        elif r < 0.86:
            prog.append([0x2040 | a << 9 | d])  # MOVEA.L Dn,An
        elif r < 0.88:
            prog.append([0x5297])  # ADDQ.L #1,(A7)  (adjust the return address in place)
        elif r < 0.90:
            prog.append([0x2F00 | d])  # MOVE.L Dn,-(A7)  (push an unknown value)
        elif r < 0.91:
            prog.append([0x4878, 0x0004])  # PEA 4.W
        # instructions the evaluator does not model: it must invalidate correctly
        elif r < 0.93:
            prog.append([0xD080 | d << 9 | d2])  # ADD.L Dn,Dm
        elif r < 0.94:
            prog.append([0x4480 | d])  # NEG.L Dn
        elif r < 0.95:
            prog.append([0x2080 | d])  # MOVE.L Dn,(An)  (a store through a pointer)
        elif r < 0.96:
            prog.append([0xC188 | d << 9 | a2(rng)])  # EXG Dn,An
        elif r < 0.97:
            prog.append([0x4C9F, 1 << rng.randrange(1, 5)])  # MOVEM.W (A7)+,regs
        elif r < 0.975:
            prog.append([0x2E80 | d])  # MOVE.L Dn,(A7): a store onto the top of the stack
        elif r < 0.98:
            prog.append([0x4E71])
        else:
            prog.append([0x48C0 | d])  # EXT.L Dn
    return prog


def a2(rng):
    return rng.choice([1, 2, 3])


def oracle_step(st, words):
    d, a, ccr, area = st
    op = words[0]
    ext = words[1:]
    line = (
        "%x %x " % (op, len(ext))
        + " ".join("%x" % e for e in ext)
        + " "
        + " ".join("%x" % x for x in d)
        + " "
        + " ".join("%x" % x for x in a)
        + " %x " % ccr
        + area.hex()
    )
    ln = bt.oracle_batch([line])[0]
    if ln is None:
        return None
    f = ln.split()
    return (
        [int(x, 16) for x in f[1:9]],
        [int(x, 16) for x in f[9:17]],
        int(f[17], 16) & 0x1F,
        bytes.fromhex(f[19]),
    )


class FlatMem:
    """The evaluator's view of memory: the oracle's area (the data bytes), and the program's own bytes."""

    def __init__(self, code, area):
        self.code = code
        self.area = area

    def __getitem__(self, x):
        if isinstance(x, slice):
            return bytes(self[k] for k in range(x.start, x.stop))
        if AREA <= x < AREA + len(self.area):
            return self.area[x - AREA]
        return self.code[x] if 0 <= x < len(self.code) else 0


def main():
    n = int(sys.argv[1]) if len(sys.argv) > 1 else 3000
    rng = random.Random(7)
    bad = 0
    steps = 0
    claims = 0
    from gen import Gen, read_offsets

    for t in range(n):
        prog = build_program(rng)
        code = bytearray(0x10000)
        pc = 0x2000
        insns = []
        for w in prog:
            insns.append(pc)
            pc = bt.put(code, pc, w)
        area = bytearray(rng.getrandbits(8) for _ in range(bt.AREA_LEN))
        d = [rng.getrandbits(32) for _ in range(8)]
        a = [AREA + 0x800 + 0x10 * k for k in range(7)] + [AREA + 0xE00]
        ret = DATA + 2 * rng.randrange(0, 16)  # the return address: points at the inline data
        # a1-a3 start unknown to the evaluator but we also give it known registers via MOVEA at the start
        pre = []
        for k in rng.sample([1, 2, 3], rng.randrange(0, 3)):
            v = DATA + rng.randrange(0, 12)
            pre.append(([0x207C | k << 9, v >> 16, v & 0xFFFF], k, v))
        st = (d, a, rng.getrandbits(5), bytes(area))
        # push the return address on the real stack
        A7 = a[7] - 4
        ar = bytearray(st[3])
        ar[A7 - AREA : A7 - AREA + 4] = ret.to_bytes(4, "big")
        a2_ = list(a)
        a2_[7] = A7
        st = (d, a2_, st[2], bytes(ar))
        mem = FlatMem(code, st[3])
        K = known.Known()
        K.push_ret(ret, 0)

        # the preamble of MOVEA #imm
        class I:
            pass

        cur = st
        for words, k, v in pre:
            nxt = oracle_step(cur, words)
            if nxt is None:
                break
            cur = nxt
            K.a[k] = (v, 0)
        else:
            for idx, (ipc, w) in enumerate(zip(insns, prog)):
                i = blockasm.decode(code, ipc)
                if i.kind != "plain":
                    break
                nxt = oracle_step(cur, w)
                if nxt is None:
                    break
                K.step(FlatMem(code, cur[3]), i, idx + 1)
                cur = nxt
                steps += 1
                dd, aa, ccr, area_now = cur
                A7_0 = a[7] - 4 + 4  # the original A7: before the push
                for r, (v, di) in K.a.items():
                    claims += 1
                    if aa[r] != v:
                        bad += 1
                        print(
                            "MISMATCH a%d: claimed %x, real %x after %s (instruction %d); program %s; preamble %s"
                            % (
                                r,
                                v,
                                aa[r],
                                [hex(x) for x in w],
                                idx,
                                [[hex(x) for x in q] for q in prog],
                                [(k, hex(v_)) for _, k, v_ in pre],
                            )
                        )
                        break
                for r, (v, di) in K.d.items():
                    claims += 1
                    if dd[r] != v:
                        bad += 1
                        print(
                            "MISMATCH d%d: claimed %x, real %x after %s" % (r, v, dd[r], [hex(x) for x in w])
                        )
                        break
                if K.sp is not None:
                    claims += 1
                    if aa[7] != A7_0 + K.sp:
                        bad += 1
                        print(
                            "MISMATCH sp: claimed %d, real %d after %s"
                            % (K.sp, aa[7] - A7_0, [hex(x) for x in w])
                        )
                    for x, (v, di) in K.stk.items():
                        claims += 1
                        real = int.from_bytes(area_now[A7_0 + x - AREA : A7_0 + x - AREA + 4], "big")
                        if real != v:
                            bad += 1
                            print(
                                "MISMATCH stack slot %d: claimed %x, real %x after %s"
                                % (x, v, real, [hex(y) for y in w])
                            )
                else:
                    pass
        if bad > 5:
            break
    print(
        "%d programs, %d instructions, %d claims checked against the oracle, %d wrong"
        % (n, steps, claims, bad)
    )
    sys.exit(1 if bad else 0)


main()
