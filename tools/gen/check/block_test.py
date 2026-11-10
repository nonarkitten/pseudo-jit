#!/usr/bin/env python3
"""Tests the block assembler (tools/gen/blockasm.py) against the 68000 oracle. A random program -- straight-line
instructions, conditional branches out of the block,
DBF loops, calls (BSR, JSR) to small routines some of which can be inlined and some not, JSR through a
register, RTS -- is
   * run through the 68000 oracle (tools/gen/check/oracle1.c, one instruction at a time, the state passed on)
   with control flow done here: the 68000's own answer,
     stopping where the block does (it left the addresses the block translated, or reached an instruction the
     block ends with),
   * assembled into one block with every flag assumed wanted, and under Unicorn,
   * assembled with flag liveness (dead flags cut out) and call inlining, and under Unicorn,
and all three must end in the same registers, memory area (the stack lives in it), flags and exit address. It
also reports how much the passes save.

  .venv/bin/python tools/gen/check/block_test.py [programs, default 300] [seed]      (needs build/oracle1, build/helpers.bin, build/asm_offsets.inc)
"""

import sys, os, random, subprocess, re, collections

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("..", "../..")]
from paths import build_path
from gen import Gen, read_offsets
from sim import Sim, SENTINEL
import sim_test as st
import blockasm, fwd
from blockasm import CC68
from unicorn.arm_const import UC_ARM_REG_R0

AREA, AREA_LEN = st.AREA, st.AREA_LEN
MAIN_AT, ROUT_AT = 0x2000, 0x3000
OP_BASE = st.OP_ADDR + 2  # where the oracle puts the first extension word
EXCLUDE = re.compile(
    r"emit_(ADDA|SUBA|MOVEA|LEA|LINK|UNLK|PEA|EXG|MOVEM|MOVEP|MOVE_USP_TO|MOVE_TO_USP|MOVE_TO_SR|MOVE_SR_TO|ANDI_TO_SR|ORI_TO_SR|EORI_TO_SR|CHK|DIVU|DIVS|ABCD|SBCD|NBCD|ADDQAW|SUBQAW|TRAP|ALINE|FLINE|ILLEGAL|RESET|STOP|RTE|RTR|RTS|JMP|JSR|BSR|BRA|Bcc|DBcc)"
)
MODES = {"dn", "an", "(an)", "(an)+", "-(an)", "d16", "idx", "absw", "absl", "imm", "pcd16"}
EDGE = st.EDGE


def candidate_ops(g):
    ops = []
    for op in range(65536):
        name = g.winner[op]
        if not name or EXCLUDE.match(name):
            continue
        rl = st.roles(name, op)
        if rl is None or g.generate(op) is None:
            continue
        if (op >> 9) & 7 > 4:
            continue  # D5 is the loop counter
        if not rl and (op & 7) > 4 and not (name in ("emit_MOVEQ",)):
            continue
        ok = True
        for role, mode, reg, w in rl:
            if mode not in MODES:
                ok = False
                break
            if mode == "dn" and reg > 4:
                ok = False
                break
            if mode in ("an", "(an)", "(an)+", "-(an)", "d16", "idx") and reg > 3:
                ok = False
                break
            if w == 1 and mode in ("(an)+", "-(an)"):
                ok = False
                break
            if mode == "an" and (role == "dst" or w == 1):
                ok = False
                break
            if mode == "pcd16" and role == "dst":
                ok = False
                break
        if not ok:
            continue
        if op & 0xF1FF == 0x013C or (op >> 12) in (2, 3) and (op >> 3) & 7 == 1:
            continue  # the oracle's own limits (see sim_test.oracle_limit)
        if re.match(r"emit_MOVE(B|W|L)$", name) and ((op >> 6) & 7 == 1):
            continue
        ops.append((op, name, rl))
    return ops


def hot_pool(ops):
    """The instructions whose data registers are all D0 / D1: in a program of them most instructions read what
    the one before wrote.
    """
    out = []
    for op, name, rl in ops:
        if (op >> 9) & 7 > 1:
            continue
        if not rl and (op & 7) > 1:
            continue
        if any(m == "dn" and r > 1 for _, m, r, _ in rl):
            continue
        out.append((op, name, rl))
    return out


def rnd_ext(rng, rl):
    """Extension words as tokens: an int, or ('pc', target) for a PC-relative displacement (rendered per
    placement).
    """
    ext = []
    for role, mode, reg, w in rl:
        if mode == "imm":
            v = rng.choice(EDGE) if rng.random() < 0.5 else rng.getrandbits(32)
            if w == 1:
                ext.append(v & 0xFF)
            elif w == 2:
                ext.append(v & 0xFFFF)
            else:
                ext += [v >> 16 & 0xFFFF, v & 0xFFFF]
        elif mode == "d16":
            ext.append(rng.randrange(-0x40, 0x40, 2) & 0xFFFF)
        elif mode == "idx":
            x = rng.choice([6, 7])
            long_ = x == 6 and rng.random() < 0.5
            ext.append(x << 12 | long_ << 11 | (rng.randrange(-0x20, 0x20, 2) & 0xFF))
        elif mode in ("absw", "absl"):
            t = AREA + rng.randrange(0x400, 0xC00, 2)
            ext += [t & 0xFFFF] if mode == "absw" else [t >> 16, t & 0xFFFF]
        elif mode == "pcd16":
            ext.append(("pc", AREA + rng.randrange(0x400, 0xC00, 2)))
    return ext


def render(tokens, first_ext_addr):
    return [
        ((t[1] - (first_ext_addr + 2 * k)) & 0xFFFF) if isinstance(t, tuple) else t
        for k, t in enumerate(tokens)
    ]


def initial_state(rng, routines):
    d = [rng.choice(EDGE) if rng.random() < 0.6 else rng.getrandbits(32) for _ in range(8)]
    d[6] = 0x10
    d[7] = 0xABCD0020
    d[5] = rng.randrange(0, 4)  # index registers; the DBF counter
    a = [AREA + 0x800 + 0x100 * k for k in range(4)] + [
        routines[0],
        routines[-1],
        rng.getrandbits(32),
        AREA + 0xE00,
    ]
    return d, a, rng.getrandbits(5), bytes(rng.getrandbits(8) for _ in range(AREA_LEN))


def cond_true(c, ccr):
    n, z, v, cy = ccr >> 3 & 1, ccr >> 2 & 1, ccr >> 1 & 1, ccr & 1
    return [
        True,
        False,
        not cy and not z,
        cy or z,
        not cy,
        cy,
        not z,
        z,
        not v,
        v,
        not n,
        n,
        n == v,
        n != v,
        not z and n == v,
        z or n != v,
    ][c]


def put(mem, pc, words):
    for k, w in enumerate(words):
        mem[pc + 2 * k] = w >> 8 & 255
        mem[pc + 2 * k + 1] = w & 255
    return pc + 2 * len(words)


def movem_regs(rng, stores):
    """A register set that is safe to move: D0-D4 (and A0-A3 when storing), as bit numbers 0-7 = D0-D7, 8-15 =
    A0-A7.
    """
    pool = [0, 1, 2, 3, 4] + ([8, 9, 10, 11] if stores else [])
    return sorted(rng.sample(pool, rng.randrange(1, min(6, len(pool)) + 1)))


def movem_mask(regs, predec):
    return sum(1 << (15 - r) for r in regs) if predec else sum(1 << r for r in regs)


def make_routine(rng, ops, mem, at, lower, kind):
    """A small routine at `at`; `lower` = addresses of routines it may call; kind: 'plain', 'stack', 'bad',
    'leak', 'frame'.
    """
    pc = at
    saved = None
    if kind == "frame":
        pc = put(mem, pc, [0x4E55, (-rng.randrange(1, 16) * 2) & 0xFFFF])  # LINK.W A5,#-n
        if rng.random() < 0.7:
            saved = movem_regs(rng, True)
            # MOVEM.L regs,-(A7): the compiler's prologue
            pc = put(mem, pc, [0x48E7, movem_mask(saved, True)])
    for _ in range(rng.randrange(1, 5)):
        op, name, rl = rng.choice(ops)
        ext = rnd_ext(rng, rl)
        pc = put(mem, pc, [op] + render(ext, pc + 2))
    if kind == "stack":  # a balanced push / pop pair
        pc = put(mem, pc, [0x2F00 | rng.randrange(5)])
        op, name, rl = rng.choice(ops)
        ext = rnd_ext(rng, rl)
        pc = put(mem, pc, [op] + render(ext, pc + 2))
        pc = put(mem, pc, [0x201F | rng.randrange(5) << 9])
    if rng.random() < 0.35:  # a conditional skip over one instruction
        cc = rng.randrange(2, 16)
        op, name, rl = rng.choice(ops)
        ext = rnd_ext(rng, rl)
        ins = [op] + render(ext, pc + 4 + 2)
        pc = put(mem, pc, [0x6000 | cc << 8, (2 + 2 * len(ins)) & 0xFFFF])
        pc = put(mem, pc, ins)  # Bcc.W over the instruction: its target is the word after it
    if rng.random() < 0.3:  # an early return: Bcc.B over the RTS
        pc = put(mem, pc, [0x6002 | rng.randrange(2, 16) << 8])
        pc = put(mem, pc, [0x4E75])
        op, name, rl = rng.choice(ops)
        ext = rnd_ext(rng, rl)
        pc = put(mem, pc, [op] + render(ext, pc + 2))
    if lower and rng.random() < 0.4:
        pc = put(mem, pc, [0x6100, (rng.choice(lower) - (pc + 2)) & 0xFFFF])  # BSR.W to a lower routine
    if kind == "leak":
        pc = put(mem, pc, [0x2F00 | rng.randrange(5)])  # MOVE.L Dn,-(A7) and no pop: the RTS returns to Dn
    if kind == "bad":
        pc = put(mem, pc, [0x588F])  # ADDQ.L #4,A7: the routine pops its own return address
    if kind == "frame":
        if saved:
            pc = put(mem, pc, [0x4CDF, movem_mask(saved, False)])  # MOVEM.L (A7)+,regs
        pc = put(mem, pc, [0x4E5D])  # UNLK A5
    put(mem, pc, [0x4E75])


class Mem(bytearray):
    pass


TSTC, TSTC2, INCA4, SUBW, TSTC3 = 0x3800, 0x3880, 0x38A0, 0x3900, 0x3920


def put_idiom_routines(mem):
    put(mem, TSTC, [0x2C5F, 0x181E, 0xB810, 0x6708, 0x4284, 0x1816, 0xDDC4, 0x4ED6, 0x5488, 0x528E, 0x4ED6])
    # MOVEA.L (A7)+,A6 ; MOVE.B (A6)+,D4 ; CMP.B (A0),D4 ; BEQ.B L1 ; CLR.L D4 ; MOVE.B (A6),D4 ; ADDA.L D4,A6
    # ; JMP (A6) ; L1: ADDQ.L #2,A0 ; ADDQ.L #1,A6 ; JMP (A6)
    # MOVEA.L (A7),A6 ; MOVE.B (A6),D4 ; ADDQ.L #2,(A7) ; RTS : reads the inline data, returns past it
    put(mem, TSTC2, [0x2C57, 0x1816, 0x5497, 0x4E75])
    put(mem, INCA4, [0xD8FC, 0x0100, 0x4E75])  # ADDA.W #$100,A4 ; RTS : changes a register a caller may know
    # MOVE.L A1,4(A7) ; RTS : overwrites its caller's stacked return address
    put(mem, SUBW, [0x2F49, 0x0004, 0x4E75])
    # BSR SUBW ; then TSTC's popping of the (now changed) return address
    put(mem, TSTC3, [0x6100, (SUBW - (TSTC3 + 2)) & 0xFFFF, 0x2C5F, 0x181E, 0x4284, 0x1816, 0xDDC4, 0x4ED6])


def idiom_site(rng, ops, mem, pc, routines):
    kind = rng.randrange(5)
    if kind == 0:  # TSTC: BSR TSTC ; dc.b char, off ; match code ; RTS ; mismatch code ; RTS
        ch = rng.randrange(1, 256)
        mem.hint.append(ch)
        ret = pc + 4
        m_at = ret + 2
        q = m_at
        mm = []
        for _ in range(rng.randrange(1, 3)):
            op, name, rl = rng.choice(ops)
            ext = rnd_ext(rng, rl)
            ww = [op] + render(ext, q + 2)
            mm.append(ww)
            q += 2 * len(ww)
        off = (q + 2) - (ret + 1)
        if off >= 256:
            return pc
        pc = put(mem, pc, [0x6100, (TSTC - (pc + 2)) & 0xFFFF])
        pc = put(mem, pc, [ch << 8 | off])
        for ww in mm:
            pc = put(mem, pc, ww)
        pc = put(mem, pc, [0x4E75])  # the match path ends
        for _ in range(rng.randrange(1, 3)):  # the mismatch code, at ret + 1 + off
            op, name, rl = rng.choice(ops)
            ext = rnd_ext(rng, rl)
            pc = put(mem, pc, [op] + render(ext, pc + 2))
        return put(mem, pc, [0x4E75])
    if kind == 1:  # TSTC2: BSR ; dc.w data ; code (the routine returns past the data)
        pc = put(mem, pc, [0x6100, (TSTC2 - (pc + 2)) & 0xFFFF])
        pc = put(mem, pc, [rng.getrandbits(16)])
        op, name, rl = rng.choice(ops)
        ext = rnd_ext(rng, rl)
        return put(mem, pc, [op] + render(ext, pc + 2))
    if kind == 2:  # LEA r,A4 ; BSR INCA4 ; JSR (A4): A4 is r + $100 by then
        t = rng.choice(routines[:-1])
        pc = put(mem, pc, [0x49F9, t >> 16, t & 0xFFFF])
        pc = put(mem, pc, [0x6100, (INCA4 - (pc + 2)) & 0xFFFF])
        return put(mem, pc, [0x4E94])
    if kind == 3:  # TSTC3: the routine's nested call overwrites the return address before it pops it
        pc = put(mem, pc, [0x6100, (TSTC3 - (pc + 2)) & 0xFFFF])
        return put(mem, pc, [rng.getrandbits(16), 0x4E75])
    return pc


def make_program(rng, ops, routines):
    mem = Mem(0x10000)
    kinds = ["plain", "plain", "stack", "bad", "leak", "frame", "frame"]
    for k, at in enumerate(routines):
        make_routine(rng, ops, mem, at, routines[:k], rng.choice(kinds))
    put_idiom_routines(mem)
    mem.hint = []
    pc = MAIN_AT
    loop_start = None
    # touched: A4 / A5 once the program has loaded them (JSR (A4) / (A5) assumes their initial values)
    touched = set()
    for _ in range(rng.randrange(3, 12)):
        r = rng.random()
        if r < 0.10:  # a conditional branch out of the block
            cc = rng.randrange(2, 16)
            disp = rng.randrange(0x40, 0x100, 2)
            pc = put(mem, pc, [0x6000 | cc << 8, disp])
        elif r < 0.26:  # a call
            t = rng.choice(routines)
            form = rng.randrange(5)
            if form == 0 or (form == 1 and abs(t - (pc + 2)) >= 126):
                pc = put(mem, pc, [0x6100, (t - (pc + 2)) & 0xFFFF])  # BSR.W
            elif form == 1:
                pc = put(mem, pc, [0x6100 | (t - (pc + 2)) & 0xFF])  # BSR.B
            elif form == 2:
                pc = put(mem, pc, [0x4EB9, t >> 16, t & 0xFFFF])  # JSR abs.l
            elif form == 3:
                pc = put(mem, pc, [0x4EBA, (t - (pc + 2)) & 0xFFFF])  # JSR d16(PC)
            else:  # JSR (A4) / (A5): the first / last routine
                fresh = [q for q in (4, 5) if q not in touched]
                if fresh:
                    pc = put(mem, pc, [0x4E90 | rng.choice(fresh)])
                else:
                    pc = put(mem, pc, [0x6100, (t - (pc + 2)) & 0xFFFF])
        elif r < 0.17 + 0.0 and False:
            pass
        elif (
            r < 0.34 and loop_start is None and rng.random() < 0.6
        ):  # BRA.B / JMP abs.l over a few dead instructions; a Bcc over an RTS island
            kind = rng.randrange(3)
            dead = []
            for _ in range(rng.randrange(1, 3)):
                op, name, rl = rng.choice(ops)
                dead.append((op, rl))
            # (an upper bound is fine: the extension words below are rendered after)
            nbytes = sum(2 + 2 * len(rnd_ext(random.Random(0), rl)) for op, rl in dead)
            if kind == 0:
                pcs = pc + 2
                size = 0
                body = []
                for op, rl in dead:
                    ext = rnd_ext(rng, rl)
                    w = [op] + render(ext, pcs + size + 2)
                    body += w
                    size += 2 * len(w)
                if size < 127:
                    pc = put(mem, pc, [0x6000 | size])
                    pc = put(mem, pc, body)
            elif kind == 1:
                size = 0
                body = []
                pcs = pc + 6
                for op, rl in dead:
                    ext = rnd_ext(rng, rl)
                    w = [op] + render(ext, pcs + size + 2)
                    body += w
                    size += 2 * len(w)
                t = pc + 6 + size
                pc = put(mem, pc, [0x4EF9, t >> 16, t & 0xFFFF])
                pc = put(mem, pc, body)
            elif kind == 2 and rng.random() < 0.5:  # a diamond: Bcc L1 ; A ; BRA L2 ; L1: B ; L2:
                cc = rng.randrange(2, 16)

                def arm(start):
                    w_ = []
                    sz = 0
                    for _ in range(rng.randrange(1, 3)):
                        op, name, rl = rng.choice(ops)
                        ext = rnd_ext(rng, rl)
                        ww = [op] + render(ext, start + sz + 2)
                        w_ += ww
                        sz += 2 * len(ww)
                    return w_, sz

                a_w, a_sz = arm(pc + 4)
                b_w, b_sz = arm(pc + 4 + a_sz + 2)
                # the layout: [Bcc.W to L1][A][BRA.B to L2][B]; L1 is B, L2 is after B
                if b_sz < 127:
                    pc = put(mem, pc, [0x6000 | cc << 8, a_sz + 2 + 2])
                    pc = put(mem, pc, a_w)
                    pc = put(mem, pc, [0x6000 | b_sz])
                    pc = put(mem, pc, b_w)
            else:
                cc = rng.randrange(2, 16)
                pcs = pc + 4
                size = 0
                body = []
                for op, rl in dead:
                    ext = rnd_ext(rng, rl)
                    w = [op] + render(ext, pcs + size + 2)
                    body += w
                    size += 2 * len(w)
                size += 2  # the RTS
                pc = put(mem, pc, [0x6000 | cc << 8, size + 2])
                pc = put(mem, pc, body)
                pc = put(mem, pc, [0x4E75])
        elif r < 0.34 and loop_start is None:
            pc = put(mem, pc, [0x7A00 | rng.randrange(4)])
            loop_start = pc  # MOVEQ #n,D5: the counter; the start of a DBcc loop
        elif r < 0.44 and loop_start is not None and pc > loop_start:  # its end (the body is not empty)
            cc = rng.choice([1, 1, 1, 6, 7])  # DBF mostly; DBNE / DBEQ now and then
            pc = put(mem, pc, [0x50C8 | cc << 8 | 5, (loop_start - (pc + 2)) & 0xFFFF])
            loop_start = None
        elif (
            r < 0.60 and loop_start is None and rng.random() < 0.3
        ):  # a call with inline data (TSTC style), or one of its relatives
            pc = idiom_site(rng, ops, mem, pc, routines)
        elif (
            r < 0.60 and loop_start is None and rng.random() < 0.4
        ):  # MOVEM to / from memory, and a push that may leave the stack halfword aligned
            f = rng.randrange(5)
            long_ = rng.random() < 0.7
            sz = 0x40 if long_ else 0
            if f == 0:
                # MOVEM regs,(A1)
                pc = put(mem, pc, [0x4890 | sz | 1, movem_mask(movem_regs(rng, True), False)])
            elif f == 1:
                # MOVEM (A1),regs
                pc = put(mem, pc, [0x4C90 | sz | 1, movem_mask(movem_regs(rng, False), False)])
            elif f == 2:
                pc = put(mem, pc, [0x48A7 | sz, movem_mask(movem_regs(rng, True), True)])  # MOVEM regs,-(A7)
            elif f == 3:
                # MOVEM (A7)+,regs
                pc = put(mem, pc, [0x4C9F | sz, movem_mask(movem_regs(rng, False), False)])
            else:
                # MOVEM regs,d16(A1)
                pc = put(
                    mem,
                    pc,
                    [
                        0x4880 | sz | 0x28 | 1,
                        movem_mask(movem_regs(rng, True), False),
                        (rng.randrange(0, 8) * 2) & 0xFFFF,
                    ],
                )
        elif (
            r < 0.54 and loop_start is None
        ):  # calls and jumps through an address register that holds a constant (or used to)
            t = rng.choice(routines)
            form = rng.randrange(6)
            areg = rng.choice([4, 5])
            touched.add(areg)
            # LEA abs.l,An / MOVEA.L #imm,An
            ld = (
                [0x41F9 | areg << 9, t >> 16, t & 0xFFFF]
                if rng.random() < 0.5
                else [0x207C | areg << 9, t >> 16, t & 0xFFFF]
            )
            if form == 0:  # load, then call through it
                pc = put(mem, pc, ld)
                pc = put(mem, pc, [0x4E90 | areg])
            elif form == 1:  # load, some plain instructions, call
                pc = put(mem, pc, ld)
                for _ in range(rng.randrange(1, 3)):
                    op, name, rl = rng.choice(ops)
                    ext = rnd_ext(rng, rl)
                    pc = put(mem, pc, [op] + render(ext, pc + 2))
                pc = put(mem, pc, [0x4E90 | areg])
            elif form == 2:  # load, ADDA.W #$100,An (the next routine), call: the constant is stale
                if t != routines[-1]:
                    pc = put(mem, pc, ld)
                    pc = put(mem, pc, [0xD0FC | areg << 9, 0x100])
                    pc = put(mem, pc, [0x4E90 | areg])
            elif form == 3:  # a loop calling through An, which moves to the next routine each time round
                if t == routines[0] and loop_start is None:
                    pc = put(mem, pc, ld)
                    pc = put(mem, pc, [0x7A02])
                    head = pc
                    pc = put(mem, pc, [0x4E90 | areg])
                    pc = put(mem, pc, [0xD0FC | areg << 9, 0x100])
                    pc = put(mem, pc, [0x51CD, (head - (pc + 2)) & 0xFFFF])
            elif (
                form == 5
            ):  # Bcc over the LEA, then a call through the register: a join where only one way in knows it
                cc = rng.randrange(2, 16)
                pc = put(mem, pc, [0x6000 | cc << 8 | 6])
                pc = put(mem, pc, [0x41F9 | areg << 9, t >> 16, t & 0xFFFF])
                pc = put(mem, pc, [0x4E90 | areg])
            else:  # LEA L,An ; JMP (An) over a dead instruction
                op, name, rl = rng.choice(ops)
                ext = rnd_ext(rng, rl)
                dead = [op] + render(ext, 0)
                target = pc + 6 + 2 + 2 * len(dead)
                pc = put(mem, pc, [0x41F9 | areg << 9, target >> 16, target & 0xFFFF])
                pc = put(mem, pc, [0x4ED0 | areg])
                pc = put(mem, pc, [op] + render(ext, pc + 2))
        else:
            op, name, rl = rng.choice(ops)
            ext = rnd_ext(rng, rl)
            pc = put(mem, pc, [op] + render(ext, pc + 2))
    if loop_start is not None and pc > loop_start:
        pc = put(mem, pc, [0x51CD, (loop_start - (pc + 2)) & 0xFFFF])
    put(mem, pc, [0x4E75])
    return mem


def oracle_batch(lines):
    def run(batch):
        r = (
            subprocess.run(
                [build_path("host", "oracle1")], input="\n".join(batch) + "\n", capture_output=True, text=True
            )
            .stdout.strip()
            .splitlines()
        )
        if len(r) == len(batch):
            return r
        if len(batch) == 1:
            return [None]
        h = len(batch) // 2
        return run(batch[:h]) + run(batch[h:])

    return run(lines)


def push(B, v):
    d, a, ccr, area = B["st"]
    a = list(a)
    a[7] = (a[7] - 4) & 0xFFFFFFFF
    ar = bytearray(area)
    o = a[7] - AREA
    ar[o : o + 4] = v.to_bytes(4, "big")
    B["st"] = (d, a, ccr, bytes(ar))


def pop(B):
    d, a, ccr, area = B["st"]
    a = list(a)
    o = a[7] - AREA
    v = int.from_bytes(area[o : o + 4], "big")
    a[7] = (a[7] + 4) & 0xFFFFFFFF
    B["st"] = (d, a, ccr, area)
    return v


def jump_target(B, i):
    d, a, ccr, area = B["st"]
    if i.static is not None:
        return i.static
    mode, reg = i.op >> 3 & 7, i.op & 7
    if mode == 2:
        return a[reg]
    if mode == 5:
        return (a[reg] + blockasm.sext(i.words[1], 16)) & 0xFFFFFFFF
    raise NotImplementedError(mode)


def advance(B, mem):
    """Runs the control-flow instructions in Python until the program needs an oracle step (returns the
    instruction) or has reached where the block stops (None).
    """
    for _ in range(400):
        if B["done"]:
            return None
        pc = B["pc"]
        if pc not in B["S"]:
            B["done"] = True
            B["exit"] = pc
            return None
        i = blockasm.decode(mem, pc)
        if pc in B["T"]:  # the block ends with this instruction: do its work, the target is the exit
            if i.kind in ("bsr", "jsr"):
                push(B, pc + i.size)
                B["exit"] = i.target if i.kind == "bsr" else jump_target(B, i)
            elif i.kind == "rts":
                B["exit"] = pop(B)
            elif i.kind == "jmp":
                B["exit"] = jump_target(B, i)
            else:
                raise AssertionError(i.kind)
            B["done"] = True
            return None
        ccr = B["st"][2]
        B["steps"] = B.get("steps", 0) + 1
        B.setdefault("trace", []).append(pc)
        B["trace"] = B["trace"][-40:]
        if B["steps"] > 3000:
            B["done"] = True
            B["exit"] = None
            B["loop"] = True
            return None
        if i.kind == "plain":
            return i
        if i.kind == "bcc":
            B["pc"] = i.target if cond_true(i.cond, ccr) else pc + i.size
        elif i.kind == "dbcc":
            if cond_true(i.cond, ccr):
                B["pc"] = pc + i.size
            else:
                d, a, c, area = B["st"]
                d = list(d)
                w = ((d[i.reg] & 0xFFFF) - 1) & 0xFFFF
                d[i.reg] = d[i.reg] & 0xFFFF0000 | w
                B["st"] = (d, a, c, area)
                B["pc"] = pc + i.size if w == 0xFFFF else i.target
        elif i.kind in ("bsr", "jsr"):
            push(B, pc + i.size)
            B["pc"] = i.target if i.kind == "bsr" else jump_target(B, i)
        elif i.kind == "rts":
            B["pc"] = pop(B)
        elif i.kind == "jmp":
            B["pc"] = jump_target(B, i)
        else:
            raise AssertionError(i.kind)
    B["done"] = True
    B["exit"] = None
    B["loop"] = True
    return None


def oracle_ext(i):
    """The instruction's extension words as the oracle needs them: (d16,PC) displacements re-rendered for the
    oracle's placement of the opcode.
    """
    out = []
    kk = 1
    for cls in (i.src, i.dst):
        if cls == 4:
            t = i.pc + 2 * kk + blockasm.sext(i.words[kk], 16)
            out.append((t - (OP_BASE + 2 * (kk - 1))) & 0xFFFF)
            kk += 1
        else:
            for _ in range({0: 0, 1: 1, 2: 2, 3: 1, 5: 1, 6: 1}[cls]):
                out.append(i.words[kk])
                kk += 1
    return out


def dump(B, info, words, title):
    import capstone, struct

    print(title)
    for it in info["items"]:
        print(
            "   ",
            it.kind,
            "%04x" % it.ins.pc if it.ins is not None else "",
            ("%04X" % it.ins.op) if it.ins is not None else "",
        )
    md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_ARM)
    for k, w in enumerate(words):
        if isinstance(w, tuple):
            print("     %3d %s" % (k, w))
            continue
        i = next(md.disasm(struct.pack("<I", int(w)), 0), None)
        print("     %3d %08x %s" % (k, int(w), (i.mnemonic + " " + i.op_str) if i else "?"))
    print("   a =", [hex(x) for x in B["a"]], "d =", [hex(x) for x in B["d"]])


BUILD_OPTS = {
    k.split("=")[0]: (k.split("=")[1] if "=" in k else True)
    for k in os.environ.get("BLOCK_OPTS", "").split(",")
    if k
}


def main():
    nprog = int(sys.argv[1]) if len(sys.argv) > 1 else 300
    seed = int(sys.argv[2]) if len(sys.argv) > 2 else 1991
    offs = read_offsets()
    g = Gen(offs)
    sim = Sim(offs)
    sim.sym["pjit_goto"] = SENTINEL  # an exit: the run stops, r0 is the guest address
    rng = random.Random(seed)
    ops = candidate_ops(g)
    hot = hot_pool(ops)
    print("%d candidate opcodes" % len(ops))
    routines = [ROUT_AT + 0x100 * k for k in range(4)]
    progs = []
    for b in range(nprog):
        # every other program: only D0 / D1, so one instruction feeds the next
        mem = make_program(rng, hot if b % 2 else ops, routines)
        d, a, ccr, area = initial_state(rng, routines)
        if mem.hint and rng.random() < 0.6:
            ar_ = bytearray(area)
            ar_[0x800] = mem.hint[rng.randrange(len(mem.hint))]
            area = bytes(ar_)  # (A0): the character the program is looking for
        B = dict(
            mem=mem,
            d=d,
            a=a,
            ccr=ccr,
            area=area,
            st=(list(d), list(a), ccr, area),
            pc=MAIN_AT,
            done=False,
            exit=None,
        )
        words0, info0 = blockasm.build(g, mem, MAIN_AT, cap=200, optimise=False, **BUILD_OPTS)
        B["S"] = info0["addrs"]
        B["T"] = info0["terminators"]
        progs.append(B)
    # ---- the oracle, one instruction at a time, the control flow done here ----
    while True:
        todo = []
        for B in progs:
            i = advance(B, B["mem"])
            if i is not None:
                todo.append((B, i))
        if not todo:
            break
        lines = []
        for B, i in todo:
            d, a, ccr, area = B["st"]
            ext = oracle_ext(i)
            lines.append(
                "%x %x " % (i.op, len(ext))
                + " ".join("%x" % e for e in ext)
                + " "
                + " ".join("%x" % x for x in d)
                + " "
                + " ".join("%x" % x for x in a)
                + " %x " % ccr
                + area.hex()
                + " @ "
                + bytes(B["mem"][0x2000:0x4000]).hex()
            )
        for (B, i), ln in zip(todo, oracle_batch(lines)):
            if ln is None:
                B["done"] = True
                B["bad"] = True
                continue
            f = ln.split()
            B["st"] = (
                [int(x, 16) for x in f[1:9]],
                [int(x, 16) for x in f[9:17]],
                int(f[17], 16) & 0x1F,
                bytes.fromhex(f[19]),
            )
            B["pc"] += i.size
    # ---- the blocks ----
    stats = collections.Counter()
    shown = 0
    for B in progs:
        if B.get("bad"):
            stats["oracle faulted"] += 1
            continue
        if B.get("loop"):
            stats["reference ran away"] += 1
            continue
        for opt, fw in ((False, False), (True, False), (True, True), (True, "fold"), (True, "alloc")):
            words, info = blockasm.build(
                g,
                B["mem"],
                MAIN_AT,
                cap=200,
                optimise=opt,
                forward=bool(fw),
                cfold=fw in ("fold", "alloc"),
                alloc=fw == "alloc",
                **BUILD_OPTS,
            )
            tag = (
                "live+fwd+fold+alloc"
                if fw == "alloc"
                else "live+fwd+fold" if fw == "fold" else "live+fwd" if fw else ("live" if opt else "base")
            )
            dm = 0
            for w in words[: info["code_words"]]:
                sl = fwd.slot(w, g.D0) if not isinstance(w, tuple) else None
                dm += sl is not None
            stats["words " + tag] += info["code_words"]
            stats["D slot accesses " + tag] += dm
            stats["instructions"] += info["instructions"] // 1
            if opt and not fw:
                stats["inlined calls"] += info["inlined"]
            if opt and not fw:
                for it in info["items"]:
                    stats["items: " + it.kind] += 1
                    if getattr(it.ins, "resolved", False):
                        stats["calls / jumps resolved through a known register"] += 1
                stats["blocks ending in " + info["stop"]] += 1
            try:
                res = sim.run(words, B["d"], B["a"], B["ccr"], area=B["area"], guest=B["mem"])
            except Exception as e:
                stats["sim fault"] += 1
                if shown < 2:
                    shown += 1
                    dump(B, info, words, "sim fault %s %s" % (e, tag))
                continue
            if res is None:
                stats["trapped"] += 1
                if shown < 1 and os.environ.get("SAVE_FAIL"):
                    import pickle

                    pickle.dump(
                        {k: B[k] for k in ("mem", "d", "a", "ccr", "area")},
                        open(os.environ["SAVE_FAIL"], "wb"),
                    )
                    shown += 1
                    dump(B, info, words, "TRAPPED (%s, svc #%s)" % (tag, sim.trapped))
                continue
            dd, aa, cc, garea = res
            ex = sim.uc.reg_read(UC_ARM_REG_R0)
            wd, wa, wc, warea = B["st"]
            ok = dd == wd and aa == wa and cc & 0x1F == wc and garea == warea and ex == B["exit"]
            stats["ok " + tag if ok else "MISMATCH " + tag] += 1
            if not ok and shown < 2:
                import pickle

                pickle.dump(
                    {k: B[k] for k in ("mem", "d", "a", "ccr", "area")},
                    open(os.environ.get("SAVE_FAIL", "/dev/null"), "wb"),
                )
                shown += 1
                dump(
                    B,
                    info,
                    words,
                    "MISMATCH %s exit %x want %s"
                    % (tag, ex, "%x" % B["exit"] if B["exit"] is not None else None),
                )
                print(
                    "    D got ",
                    [hex(x) for x in dd],
                    "\n    D want",
                    [hex(x) for x in wd],
                    "\n    A got ",
                    [hex(x) for x in aa],
                    "\n    A want",
                    [hex(x) for x in wa],
                    "\n    ccr got %x want %x, area %s"
                    % (cc & 0x1F, wc, "same" if garea == warea else "DIFFERS"),
                )
    import sched_asm

    if sched_asm._inst:
        print("  asm scheduler: %d regions, %d rescheduled" % (sched_asm._inst.calls, sched_asm._inst.done))
    for k in sorted(stats):
        print("  %-22s %d" % (k, stats[k]))
    n = stats["instructions"] // 5 or 1
    if stats["words base"]:
        print(
            "  ARM words per 68K instruction: %.2f with every flag wanted, %.2f with liveness, %.2f with load forwarding too, %.2f with constants folded"
            % (
                stats["words base"] / n,
                stats["words live"] / n,
                stats["words live+fwd"] / n,
                stats["words live+fwd+fold"] / n,
            )
        )
        print(
            "  D register slot accesses per 68K instruction: %.2f, %.2f, %.2f, %.2f"
            % (
                stats["D slot accesses base"] / n,
                stats["D slot accesses live"] / n,
                stats["D slot accesses live+fwd"] / n,
                stats["D slot accesses live+fwd+fold"] / n,
            )
        )
    sys.exit(1 if any(k.startswith("MISMATCH") or k == "sim fault" for k in stats) else 0)


if __name__ == "__main__":
    main()
