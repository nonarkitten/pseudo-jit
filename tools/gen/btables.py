#!/usr/bin/env python3
"""Writes the block translator's tables (build/btables.s): what the asm translator (pjit/blk.s) needs per
opcode, from the same generator sequences the block
prototype (tools/gen/blockasm.py) uses.

  .venv/bin/python tools/gen/btables.py > build/btables.s

  pjit_bt        65536 x 2 words, indexed by the opcode: [leader word (plain: the first word of the sequence, else 0)] [kind << 28 | body length (words) << 20 | body offset (words into pjit_bbody)]
                 kind 0 BT_STOP    the block ends in front of it (the interpreter takes the instruction)
                      1 BT_PLAIN   copy the leader, then the body, after the extension prefix (the prefix is built from the ext-class table, as blockasm.prefix does)
                      2 BT_BCC     Bcc / BRA (the translator emits the branch); 3 BT_DBCC; 4 BT_BSR
                      5 BT_JSR  6 BT_JMP   a static target needs no body; otherwise the body (after the prefix) leaves the target address in r0
                      7 BT_RTS
  pjit_bbody     the interned bodies: the generator's sequence with its final `bx lr` dropped and conditional `bx<cc> lr` turned into branches to the end
                 (blockasm.inline_body). A `bl <helper>` (the DIV / BCD / MOVEM helpers) is stored as
                 `0xEB000000 | helper index` and must be relocated when copied
                 (pc-relative); no other word of a body has the top byte 0xEB. A `b <helper>` (the DIV tails)
                 is stored as `0xEC000000 | helper index`.
  pjit_binfo     a word per word of pjit_bbody (winfo32): the registers r0-r3 it writes, branch / barrier flags, the exact description of a D register slot access, a helper-call mark: what the load forwarder needs without decoding. The leader's
                 info is in its descriptor (bits 22:18).
  pjit_bv        65536 words, indexed by the opcode, for PLAIN ones: the flag-liveness transfer function and the variants. Bits 7:0: for each flag groups wanted AFTER
                 the instruction (L = 0 none, 1 N Z V C, 2 X, 3 both; bit 0 = N Z V C, bit 1 = X) two bits,
                 the groups it wants BEFORE it (L in bits 2L+1:2L). Bits 31:8:
                 0 = the sequence is the same for every L (the entry in pjit_bt), else 1 + the index of the
                 variant set. Bits 29:28: offset folding (PJIT_BLK_FOLD, not with liveness): the leader is a
                 load / store with a register offset r1 (bit 28) / r2 (bit 29) that no later word reads, so a
                 constant displacement from the prefix can become an immediate offset and the prefix for that
                 operand is not emitted. Bit 31: the sequence can raise an exception from inside
                 itself (an svc, a DIV helper): the translator sets r4 = the guest address after the
                 instruction first (the exception entry takes the PC from r4).
  pjit_bvarf     a byte per variant set: the offset-folding masks (2 bits: r1 / r2) of its L = 0, 1, 2 entries (bits 1:0, 3:2, 5:4)
  pjit_bvar      the variant sets, 24 bytes each: for L = 0, 1, 2 the same [leader][descriptor] pair as pjit_bt (L = 3 is the pjit_bt entry itself). These are the
                 generator's sequences cut down by tools/gen/armdce.py for what is wanted after them (a dead
                 flag-setting `adds` loses its S bit, a dead X save goes).
  pjit_bt_helpers  the helper routines' addresses, by index."""

import sys, os

sys.path.insert(0, os.path.dirname(__file__))
from gen import Gen, read_offsets

# the block runtime's forwarder / scheduler / folder know the flat forms (D-slot accesses by address), not the
# split ones
Gen.legacy = True
import blockasm as ba
import armdce
import maketables as mt
import fwd

BT_STOP, BT_PLAIN, BT_BCC, BT_DBCC, BT_BSR, BT_JSR, BT_JMP, BT_RTS = range(8)
KIND = {
    "plain": BT_PLAIN,
    "bcc": BT_BCC,
    "dbcc": BT_DBCC,
    "bsr": BT_BSR,
    "jsr": BT_JSR,
    "jmp": BT_JMP,
    "rts": BT_RTS,
    "stop": BT_STOP,
}
HELPERS = sorted(ba.HELPERS | set(ba.DIV_TAILS))


class Mem:
    def __init__(self, op):
        self.op = op

    def __getitem__(self, a):
        if isinstance(a, slice):
            return bytes(self[x] for x in range(a.start, a.stop))
        return (self.op >> (8 if a == 0 else 0)) & 0xFF if a < 2 else 0

    def __len__(self):
        return 8


def encode_body(body):
    out = []
    for w in body:
        if isinstance(w, tuple):
            assert w[1] == "al", w
            if w[0] == "bl" and w[2] in ba.HELPERS:
                out.append(0xEB000000 | HELPERS.index(w[2]))  # bl <helper>
            else:
                assert w[0] == "b" and w[2] in ba.DIV_TAILS, w
                out.append(0xEC000000 | HELPERS.index(w[2]))  # b <helper> (the DIV tails)
        else:
            w = int(w)
            assert w >> 24 not in (0xEB, 0xEC), hex(w)
            out.append(w)
    return out


def winfo(w):
    """What the load forwarder needs to know about a word without decoding it (tools/gen/fwd.py): bits 3:0 the
    scratch registers r0-r3 it writes, bit 4 it is a
    branch / call / barrier (the mirror map ends there and the rest of the instruction is left alone), bit 5
    it contains a branch, call or svc (a body with one cannot
    have a forwarded load deleted: a branch inside it has to land where it did), bit 29 (pinned D0) it writes
    r4 and is not a D0 store the map knows: the mirrors of D0 are stale.
    """
    a = armdce.analyse(w)
    hasbr = isinstance(w, tuple) or (int(w) >> 25 & 7 == 5) or (int(w) >> 24 & 15 == 15)
    return (
        (a.writes & 0xF)
        | (1 << 4 if (a.barrier or a.branch is not None) else 0)
        | (1 << 5 if hasbr else 0)
        | (1 << 29 if (fwd.PIN_D0 and not isinstance(w, tuple) and a.writes >> 4 & 1) else 0)
    )


PART = {"w": 1, "h": 2, "sh": 3, "b": 4, "sb": 5, "l16": 6, "l8": 7}
D0 = None


def winfo32(w):
    """The word's entry in pjit_binfo (32 bits): bits 5:0 as winfo; bit 6 the word is a load / store of a D
    register slot, then bits 19:8 describe it exactly as pjit/blk.s's
    blk_slot does (n | part << 3 | load << 6 | conditional << 7 | rt << 8); bit 30 it is a `bl` / `b` to a
    helper (relocated when copied). Bit 31 of a body's first word:
    some word of the body contains a branch."""
    v = winfo(w)
    if isinstance(w, tuple):
        return v | 1 << 30
    sl = fwd.slot(w, D0)
    if sl is not None:
        load, rt, n, part, cond = sl
        v |= 1 << 6 | (n | PART[part] << 3 | int(load) << 6 | int(cond) << 7 | rt << 8) << 8
    return v


def split(words):
    """-> (leader, leader info, body, body info) of an encoded sequence."""
    if not words:
        return 0, 0, [], []  # everything was dead: no leader (leader word 0 = none)
    enc = encode_body(words)
    assert enc[0] >> 24 not in (0xEB, 0xEC) and not (enc[0] >> 25 & 7 == 5)
    inf = [winfo32(w) for w in words]
    body = inf[1:]
    if body and any(x & 0x20 for x in body):
        body[0] |= 1 << 31
    return enc[0], inf[0] & 0x1F, enc[1:], body


def variants(seq):
    """-> ([the four word lists for L = 0..3], [live-in groups for L = 0..3]) or None if the sequence cannot
    be a block body.
    """
    ws, ls = [], []
    for L in range(4):
        try:
            v, li = armdce.dce(seq, L)
        except ValueError:
            v, li = list(seq), 3
        b = ba.inline_body(v, div_ok=True)
        if b is None:
            return None
        ws.append(b)
        ls.append(li)
    return ws, ls


def has_trap(b):
    return any(
        (isinstance(w, tuple) and w[2] in ("handle_DIVU_tail", "handle_DIVS_tail"))
        or (not isinstance(w, tuple) and int(w) >> 24 & 15 == 15)
        for w in b
    )


import schedtab


def unschedulable(w):
    if isinstance(w, tuple):
        return True
    info = schedtab.decode(int(w))
    return info is None or schedtab.flags(int(w), info) is None or int(w) >> 28 == 15


LIVE = "--live" in sys.argv  # the flag-liveness variants (pjit_bvar) only with --live: PJIT_BLK_LIVE


def build(g):
    global D0
    D0 = g.D0
    arena, infos, index = [], [], {}

    def intern(words, inf):
        key = tuple(words)
        if key not in index:
            index[key] = len(arena)
            arena.extend(words)
            infos.extend(inf)
        return index[key]

    def desc(kind, lw, n, off):
        assert n < 64 and off < (1 << 18) and lw < 32, (n, off, lw)
        return kind << 29 | n << 23 | lw << 18 | off

    def entry(kind, words):
        """[leader, descriptor] for a PLAIN word list"""
        leader, lw, body, binf = split(words)
        return [leader, desc(kind, lw, len(body), intern(body, binf) if body else 0)]

    bt, bv, bvar, sets = [], [], [], {}
    # the natural-polarity variants (a compare whose Bcc is the only reader of its flags), and 'reads N Z V C'
    # per opcode
    bn, bfr = (
        [],
        bytearray(),
    )
    global BVARF
    BVARF = []
    for op in range(65536):
        kind = KIND[ba.decode(Mem(op), 0).kind]
        body = []
        if kind == BT_PLAIN:
            seq = g.generate(op)
            vs = variants(seq) if seq is not None else None
            # anything that can raise an exception from inside its sequence (an svc, the DIV helpers that svc
            # on a zero divisor) is interpreted one instruction
            # at a time: a block has no map from a host address to the guest PC the exception frame needs
            # (docs/design/BLOCK_RUNTIME.md section 8)
            if vs is None:
                kind = BT_STOP
        if kind == BT_PLAIN:
            ws, ls = vs
            bfr.append(ls[0] & 1)  # what the sequence itself reads of N Z V C (live-in for an empty live-out)
            nat = None
            if not LIVE:
                g.natural = True
                try:
                    sn = g.generate(op)
                finally:
                    g.natural = False
                if sn is not None and [w if isinstance(w, tuple) else int(w) for w in sn] != [
                    w if isinstance(w, tuple) else int(w) for w in seq
                ]:
                    nat = ba.inline_body(list(sn), div_ok=True)
            bn += entry(BT_PLAIN, nat) if nat else [0, 0]
            # bit 31 of pjit_bv: r4 = the guest address after the instruction before it runs
            trap = any(has_trap(w) for w in ws)
            # bit 30: the scheduler (pjit/blksched.s) never reorders it: a branch, a call, an svc, an encoding
            # it does not decode
            nosched = any(unschedulable(w) for v in ws for w in v) or trap
            bt += entry(BT_PLAIN, ws[3])
            tr = sum(ls[L] << (2 * L) for L in range(4))
            # bits 29:28 of pjit_bv: the leader is a load / store [Rn, r1 / r2] whose offset register is dead
            # afterwards
            fm = mt.fold_bits(ws[3]) if (ws[3] and not isinstance(ws[3][0], tuple)) else 0
            if nat:
                # (the natural variant shares the entry: its fold mask must hold for both)
                fm &= mt.fold_bits(nat) if not isinstance(nat[0], tuple) else 0
            if not LIVE:
                # no liveness (PJIT_BLK_LIVE = 0): every instruction keeps all its flags, no variants
                bv.append(0xFF | trap << 31 | nosched << 30 | fm << 28)
            elif all(ws[L] == ws[3] for L in range(3)):
                bv.append(tr | trap << 31 | nosched << 30 | fm << 28)
            else:
                key = tuple(tuple(entry(BT_PLAIN, ws[L])) for L in range(3))
                fms = tuple(
                    (mt.fold_bits(ws[L]) if (ws[L] and not isinstance(ws[L][0], tuple)) else 0)
                    for L in range(3)
                )
                key = key + (fms,)
                if key not in sets:
                    sets[key] = len(sets) + 1
                    bvar += [w for e in key[:3] for w in e]
                    BVARF.append(fms[0] | fms[1] << 2 | fms[2] << 4)
                # (bits 29:28: the base entry's fold mask; the variants' are in pjit_bvarf)
                bv.append(sets[key] << 8 | tr | trap << 31 | nosched << 30 | fm << 28)
            continue
        if kind != BT_PLAIN:
            bn += [0, 0]
            bfr.append(1)  # (anything that is not a plain sequence is taken to read the flags)
        if kind in (BT_JSR, BT_JMP):
            seq = g.generate(0x4EC0 | op & 0x3F)
            b = ba.inline_body(seq, tail_ok=True) if seq is not None else None
            if b is None:
                kind = BT_STOP
            else:
                bw = b[:-1]
                body = encode_body(bw)
                # the JMP's sequence minus its `b pjit_goto`: the target is in r0
                binf = [winfo32(w) for w in bw]
        bt += [0, desc(kind, 0, len(body), intern(body, binf) if body else 0)]
        bv.append(0xFF)  # (control: the translator treats every non-plain item as wanting everything)
    return bt, arena, infos, bv, bvar, bn, bfr


def main():
    g = Gen(read_offsets())
    bt, arena, infos, bv, bvar, bn, bfr = build(g)
    o = [
        "@ build/btables.s -- generated by tools/gen/btables.py; the block translator's tables (see that file)",
        "\t.syntax unified",
        "\t.section .rodata",
        "\t.balign 64",
        "\t.global pjit_bt, pjit_bbody, pjit_bv, pjit_bvar, pjit_bvarf, pjit_binfo, pjit_bt_helpers, pjit_bn, pjit_bfr",
        "pjit_bt:",
    ]
    o += ["\t.word\t" + ", ".join("0x%08X" % w for w in bt[i : i + 8]) for i in range(0, len(bt), 8)]
    o += ["pjit_bn:"] + [
        "\t.word\t" + ", ".join("0x%08X" % w for w in bn[i : i + 8]) for i in range(0, len(bn), 8)
    ]
    o += (
        ["pjit_bfr:"]
        + ["\t.byte\t" + ", ".join("%d" % w for w in bfr[i : i + 32]) for i in range(0, len(bfr), 32)]
        + ["\t.balign 4"]
    )
    o += ["pjit_bbody:"]
    o += ["\t.word\t" + ", ".join("0x%08X" % w for w in arena[i : i + 8]) for i in range(0, len(arena), 8)]
    o += ["pjit_binfo:"] + [
        "\t.word\t" + ", ".join("0x%08X" % w for w in infos[i : i + 8]) for i in range(0, len(infos), 8)
    ]
    o += ["pjit_bv:"] + [
        "\t.word\t" + ", ".join("0x%08X" % w for w in bv[i : i + 8]) for i in range(0, len(bv), 8)
    ]
    o += (
        ["pjit_bvarf:"]
        + ["\t.byte\t" + ", ".join("0x%02X" % w for w in BVARF[i : i + 16]) for i in range(0, len(BVARF), 16)]
        + ["\t.balign 4"]
    )
    o += (
        ["pjit_bvar:"]
        + ["\t.word\t" + ", ".join("0x%08X" % w for w in bvar[i : i + 8]) for i in range(0, len(bvar), 8)]
        + ["\t.word\t0"]
    )
    o += ["pjit_bt_helpers:"] + ["\t.word\t%s" % h for h in HELPERS]
    print("\n".join(o))
    kinds = [0] * 8
    for e in bt[1::2]:
        kinds[e >> 29] += 1
    print(
        "@ opcodes by kind (stop, plain, bcc, dbcc, bsr, jsr, jmp, rts): %s; body arena %d words (%d KB); table %d KB; bv %d KB; %d variant sets (%d KB)"
        % (
            kinds,
            len(arena),
            len(arena) * 4 // 1024,
            len(bt) * 4 // 1024,
            len(bv) * 4 // 1024,
            len(bvar) // 6,
            len(bvar) * 4 // 1024,
        ),
        file=sys.stderr,
    )


if __name__ == "__main__":
    main()
