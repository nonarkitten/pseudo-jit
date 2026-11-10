#!/usr/bin/env python3
"""A trace-driven model of the block runtime (pjit/blk.s) on the Cortex-A8: given a 68K instruction trace and the
program image it forms the blocks exactly as the
translator does (decode pass with its caps, backward flag liveness, emit with the liveness variants, the
extension prefix, load forwarding, the call / RTS / DBcc / Bcc
sequences), lays them out at host addresses, replays the trace through them and feeds every ARM word that
would execute to tools/study/cyclemodel.py's pipeline and branch
predictor model (2-bit counters, a return stack, last-target indirect prediction, real code addresses).
Reports ARM instructions, mispredicts and modelled cycles
per 68K instruction, to be compared with the board (the board's Dhrystone and Tiny BASIC numbers) and
then used on traces we cannot run (LhA).

  .venv/bin/python tools/study/blockrun.py trace.bin image.bin base [--be] [--drop lo,hi] [--limit N]
  [--noinline-rts] [--nofwd] [--nolive]

Approximations (each is a known gap, see the comparison with the board): the words inside one instruction's
sequence all execute (its internal conditional branches
are treated as not taken); the shadow return stack is keyed by a tracked A7 (tools/gen/known.py); first-time
costs (translation, router entries, linking) are counted
and costed separately by constants; no cache or TLB misses."""

import sys, os, struct, collections, bisect

sys.path.insert(0, os.path.dirname(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("../gen", "..")]
import cyclemodel
from gen import Gen, read_offsets
import blockasm as ba, btables as bt, armdce, fwd, known, a7, constfold
from arm32 import *

SRC_CAP, HOST_CAP, MAXSTUB, MAXSITE, MAXITEM = 256, 8 * 1024, 24, 48, 192
TAIL_WORDS = MAXSTUB * 4 + 24
RING_BASE = 0x82500000
# pjit_goto's address on the board (any fixed address does: the shared router code is what matters)
GOTO = 0x82002D10
CC68_TO_ARM = [14, 14, 0, 0, 3, 2, 1, 0, 7, 6, 5, 4, 10, 11, 12, 13]
PLAIN, STOPK, BCC, DBCC, BSR, JSR, JMP, RTS = (
    bt.BT_PLAIN,
    bt.BT_STOP,
    bt.BT_BCC,
    bt.BT_DBCC,
    bt.BT_BSR,
    bt.BT_JSR,
    bt.BT_JMP,
    bt.BT_RTS,
)

# the words of the runtime routines (from `objdump -d` of pjit_goto / pjit_blk_rts on the board image)
GOTO_WORDS = [
    0xE30F1000,
    0xE3481254,
    0xE7E920D0,
    0xE0811182,
    0xE5912000,
    0xE5913004,
    0xE0222000,
    0xE16F2F12,
    0xE1A022A2,
    0xE08FF102,
    0xE320F000,
    0xEA000000,
    0xE12FFF13,
]
# ldr r0,[sp],#4 ... nop
RTS_WORDS = [
    0xE49D0004,
    0xE20D2FFF,
    0xE0853082,
    0xE5131908,
    0xE513E904,
    0xE0211000,
    0xE16F1F11,
    0xE1A012A1,
    0xE08FF101,
    0xE320F000,
]
CALL_A = [0xE20D2FFF]  # and r2,sp,#0x3FC
CALL_B = [0xE52D0004, 0xE0853082, 0xE5030908, 0xE28F1004, 0xE5031904]  # push R, the shadow entry
DYN_B = [0xE52D3004, 0xE0852082, 0xE5023908, 0xE28F1004, 0xE5021904]


def offset_fold(words):
    """constfold restricted to turning a known register offset of a load / store into an immediate offset (the
    old `fold bits`), then the constant loads nothing reads are dropped.
    """
    consts = {}
    out = []
    for w in words:
        if isinstance(w, tuple):
            out.append(w)
            consts.clear()
            continue
        w = int(w)
        info = armdce.analyse(w)
        if w & 0x0FF00000 == 0x03000000 and w >> 28 == 14:
            consts[w >> 12 & 15] = (w >> 4 & 0xF000) | (w & 0xFFF)
        elif (w & 0x0FFF0FF0) in constfold.EXT and (w & 15) in consts and w >> 28 == 14:
            consts[w >> 12 & 15] = constfold.EXT[w & 0x0FFF0FF0](consts[w & 15])
        elif w & 0x0FF00000 == 0x03400000:
            consts.pop(w >> 12 & 15, None)
        elif (
            w & 0x0E000010 == 0x06000000
            and w >> 24 & 1
            and not w >> 21 & 1
            and not w & 0xFE0
            and (w & 15) in consts
        ):
            v = consts[w & 15]
            sv = v - (1 << 32) if v >> 31 else v
            if not w >> 23 & 1:
                sv = -sv
            if abs(sv) < 4096:
                w = (
                    (w & 0xF0000000)
                    | 0x05000000
                    | (1 << 23 if sv >= 0 else 0)
                    | (w & 0x00500000)
                    | (w & 0x000FF000)
                    | (w >> 16 & 15) << 16
                    | abs(sv)
                )
                w = (w & ~0x000F0000) | ((int(words[len(out)]) >> 16 & 15) << 16)
        elif (
            w & 0x0E400F90 == 0x00000090
            and w & 0x60
            and w >> 24 & 1
            and not w >> 21 & 1
            and (w & 15) in consts
        ):
            v = consts[w & 15]
            sv = v - (1 << 32) if v >> 31 else v
            if not w >> 23 & 1:
                sv = -sv
            if abs(sv) < 256:
                w = (
                    (w & 0xF0000000)
                    | 0x01400000
                    | (1 << 23 if sv >= 0 else 0)
                    | (w & 0x00100000)
                    | (w & 0x000FF000)
                    | (w & 0xF0)
                    | (abs(sv) >> 4) << 8
                    | (abs(sv) & 15)
                )
        else:
            for r in range(4):
                if info.writes >> r & 1:
                    consts.pop(r, None)
        out.append(w)
    return out


class Item:
    __slots__ = (
        "kind",
        "ins",
        "op",
        "pc",
        "size",
        "vs",
        "L",
        "words",
        "off",
        "trap",
        "bodyw",
        "bad",
        "pw",
        "xwords",
    )


class Block:
    __slots__ = ("entry", "items", "end", "base", "nwords", "pos_of")


class Runtime:
    def __init__(self, mem, g, opts):
        self.mem, self.g, self.opts = mem, g, opts
        self.starts = []
        self.entry_of = {}
        self.d0 = g.D0
        self.kinds = {}
        self.blocks = {}
        self.top = RING_BASE
        self.nblocks = 0
        self.ntrans = 0

    def kind_of(self, op):
        k = self.kinds.get(op)
        if k is None:
            kd = bt.KIND[ba.decode(bt.Mem(op), 0).kind]
            vs = None
            body = None
            if kd == PLAIN:
                seq = self.g.generate(op)
                vs = bt.variants(seq) if seq is not None else None
                if vs is None:
                    kd = STOPK
            elif kd in (JSR, JMP):
                seq = self.g.generate(0x4EC0 | op & 0x3F)
                b = ba.inline_body(seq, tail_ok=True) if seq is not None else None
                if b is None:
                    kd = STOPK
                else:
                    body = b[:-1]
            k = self.kinds[op] = (kd, vs, body)
        return k

    # ---- the translator: pass 1 (decode), pass 2 (liveness), pass 3 (emit) ----
    def form(self, pc):
        items = []
        est = 0
        nstub = nsite = 0
        p = pc
        end = "fall"
        while True:
            if nstub >= MAXSTUB - 3 or nsite >= MAXSITE - 4 or len(items) >= MAXITEM - 2 or p - pc >= SRC_CAP:
                end = "fall"
                break
            op = ba.read_w(self.mem, p)
            kd, vs, body = self.kind_of(op)
            if kd == STOPK:
                end = "stop"
                break
            blen = len(vs[0][3]) - 1 if kd == PLAIN else len(body) if body is not None else 0
            if est + blen + 40 > HOST_CAP // 4 - TAIL_WORDS:
                end = "fall"
                break
            est += blen + 40
            ins = ba.decode(self.mem, p)
            it = Item()
            it.kind, it.ins, it.op, it.pc, it.size, it.vs, it.bodyw, it.L = (
                kd,
                ins,
                op,
                p,
                ins.size,
                vs,
                body,
                3,
            )
            it.trap = False
            if kd != PLAIN:
                nstub += 1
                nsite += 2
            items.append(it)
            p += ins.size
            if kd in (JMP, RTS) or (kd == BCC and ins.cond == 0):
                end = "term"
                break
        # liveness, backward: a control item wants everything before it
        L = 3
        for it in reversed(items):
            if it.kind == PLAIN:
                it.L = L
                L = it.vs[1][it.L] if not self.opts.get("nolive") else 3
                if self.opts.get("nolive"):
                    it.L = 3
            else:
                L = 3
        return items, end, p

    def plain_words(self, it):
        d0 = self.d0
        ins = it.ins
        v = it.vs[0][it.L]
        trap = any(bt.has_trap(w) for w in it.vs[0])
        pre = ba.prefix_all(ins, d0)
        w = (ba.load_const(4, (it.pc + it.size) & 0xFFFFFFFF) if trap else []) + pre + list(v)
        if self.opts.get("cfold"):
            w = constfold.fold(w)
            try:
                w, _ = armdce.dce(w, it.L)
            except ValueError:
                pass
        # only the displacement folding: a register offset the prefix loaded with a constant becomes an
        # immediate offset
        elif self.opts.get("ofold"):
            w = offset_fold(w)
        bad = trap or any(
            isinstance(x, tuple) or (int(x) >> 25 & 7 == 5) or (int(x) >> 24 & 15) == 15 for x in w
        )
        return w, bad

    def home_rewrite(self, w, homes):
        """a load / store of a homed D register's slot becomes a register operation on its home"""
        if isinstance(w, tuple):
            return w
        sl = fwd.slot(w, self.d0)
        if sl is None or sl[2] not in homes:
            return w
        load, rt, n, part, cond = sl
        h = homes[n]
        cc = int(w) & 0xF0000000
        if load:
            return (
                cc
                | {"w": 0x01A00000, "h": 0x06FF0070, "sh": 0x06BF0070, "b": 0x06EF0070, "sb": 0x06AF0070}[
                    part
                ]
                | rt << 12
                | h
            )
        return (
            cc
            | {
                "w": 0x01A00000 | h << 12 | rt,
                "l16": 0x07CF0010 | h << 12 | rt | (15 << 16) * 0 if False else 0x07CF0010 | h << 12 | rt,
                "l8": 0x07C70010 | h << 12 | rt,
            }[part]
        )

    def plan_homes(self, items):
        """The two-home lazy allocator (--homes): regions are maximal runs of ordinary plain instructions and Bcc
        (a call, a return, a jump, DBcc, an instruction with a branch,
        a call or an svc in its sequence ends one: the homes are written back in front of it and forgotten).
        Per region the two D registers with the most slot accesses become
        homes (r4 and lr), loaded at the region's start when the first access needs the old value, written
        back at its end when dirty, and on the taken edge of every Bcc.
        -> {item: (home map {D: reg}, loads, stores at region end, stores on a taken Bcc)}"""
        d0 = self.d0
        plan = {}
        region = []
        if self.opts.get(
            "pin"
        ):  # D registers pinned for good (no region entry / exit traffic): --pin=0 or --pin=0,1 (r4, lr)
            pm = {n: (4, 14)[k] for k, n in enumerate(self.opts["pin"])}
            # write-through (--pin-wt): memory stays current, the register is reloaded at the block start and
            # after every call
            fresh = True
            for it in items:
                if it.kind == PLAIN:
                    it.pw, it.bad = self.plain_words(it)
                    if not it.bad:
                        plan[it] = {"homes": pm}
                        if self.opts.get("pin_wt") and fresh:
                            plan[it]["loads"] = [ldr(h, 5, d0 + 4 * n) for n, h in pm.items()]
                            fresh = False
                    else:
                        fresh = True
                elif it.kind != BCC:
                    fresh = True
            return plan

        def close():
            nonlocal region
            if not region:
                return
            acc = {}
            for it in region:
                if it.kind != PLAIN:
                    continue
                for w in it.pw:
                    sl = fwd.slot(w, d0)
                    if sl is None:
                        continue
                    load, rt, n, part, cond = sl
                    e = acc.setdefault(n, [0, None, False])
                    e[0] += 1
                    if e[1] is None:
                        e[1] = load or part != "w" or cond  # the first access needs the old value
                    if not load:
                        e[2] = True
            cand = sorted(
                ((cnt - (1 if first else 0) - (1 if wr else 0), n) for n, (cnt, first, wr) in acc.items()),
                reverse=True,
            )
            homes = {}
            for gain, n in cand:
                if gain >= self.opts.get("homes_min", 2) and len(homes) < 2:
                    homes[n] = (4, 14)[len(homes)]
            if homes:
                dirty = set()
                first = region[0]
                loads = [ldr(h, 5, d0 + 4 * n) for n, h in homes.items() if acc[n][1]]
                seen = set()
                for it in region:
                    if it.kind == PLAIN:
                        for w in it.pw:
                            sl = fwd.slot(w, d0)
                            if sl and sl[2] in homes and not sl[0]:
                                dirty.add(sl[2])
                    if it.kind == BCC:
                        plan.setdefault(it, {})["taken"] = [
                            str_(homes[n], 5, d0 + 4 * n) for n in sorted(dirty)
                        ]
                last = region[-1]
                plan.setdefault(first, {})["loads"] = loads
                plan.setdefault(last, {})["end"] = [str_(homes[n], 5, d0 + 4 * n) for n in sorted(dirty)]
                for it in region:
                    plan.setdefault(it, {})["homes"] = homes
            region = []

        for it in items:
            if it.kind == PLAIN:
                it.pw, it.bad = self.plain_words(it)
                if it.bad:
                    close()
                    continue
                region.append(it)
            elif it.kind == BCC:
                region.append(it)
            else:
                close()
        close()
        return plan

    def emit(self, items):
        d0 = self.d0
        mirror = {}
        words = []
        plan = self.plan_homes(items) if self.opts.get("homes") else {}
        for it in items:
            it.off = len(words)
            ins = it.ins
            if it.kind != PLAIN:
                mirror = {}
            pl = plan.get(it, {})
            if it.kind == PLAIN:
                w, bad = (it.pw, it.bad) if self.opts.get("homes") else self.plain_words(it)
                it.bad = bad
                if "homes" in pl:
                    if self.opts.get("pin_wt"):
                        w2 = []
                        for x in w:
                            y = self.home_rewrite(x, pl["homes"])
                            sl = None if isinstance(x, tuple) else fwd.slot(x, d0)
                            if sl and not sl[0] and sl[2] in pl["homes"]:
                                w2 += [x, y]  # a store goes to memory as well
                            else:
                                w2.append(y)
                        w = w2
                    else:
                        w = [self.home_rewrite(x, pl["homes"]) for x in w]
                    if self.opts.get("homes_bound"):
                        # an upper bound: perfect coalescing, every mov to / from a home vanishes
                        w = [
                            x
                            for x in w
                            if not (
                                not isinstance(x, tuple)
                                and (int(x) & 0x0FFF0FF0) == 0x01A00000
                                and ((int(x) >> 12 & 15) in (4, 14) or (int(x) & 15) in (4, 14))
                            )
                        ]
                if self.opts.get("sched") and bad:
                    # the asm translator forgets r0-r3 in front of an instruction it does not schedule
                    mirror = {}
                if self.opts.get("nofwd"):
                    m2 = mirror
                else:
                    has_branch = any(
                        isinstance(x, tuple) or (int(x) >> 25 & 7 == 5) or (int(x) >> 24 & 15) == 15
                        for x in w
                    )
                    w, mirror = fwd.forward(w, mirror, d0, may_delete=not has_branch)
                it.words = [
                    (
                        0xEB000000
                        if isinstance(x, tuple) and x[0] == "bl"
                        else 0xEA000000 if isinstance(x, tuple) else int(x)
                    )
                    for x in w
                ]
                if pl.get("loads"):
                    it.words = [int(x) for x in pl["loads"]] + it.words
                if pl.get("end"):
                    it.words = it.words + [int(x) for x in pl["end"]]
            elif it.kind == BCC:
                c = ins.cond
                it.words = (
                    [0xEA000000]
                    if c == 0
                    else (
                        [0x2A000000, 0x1A000000]
                        if c == 2
                        else [0x2A000000, 0x0A000000] if c == 3 else [CC68_TO_ARM[c] << 28 | 0x0A000000]
                    )
                )
                it.xwords = [int(x) for x in pl.get("taken", [])]
                if pl.get("loads"):
                    it.words = [int(x) for x in pl["loads"]] + it.words
                if pl.get("end"):
                    it.words = it.words + [int(x) for x in pl["end"]]
            elif it.kind == DBCC:
                c = ins.cond
                r = ins.reg
                cw = []
                if c != 1:
                    cw = (
                        [0xEA000008]
                        if c == 0
                        else (
                            [0x2A000000, 0x1A000008]
                            if c == 2
                            else [0x2A000009, 0x0A000008] if c == 3 else [CC68_TO_ARM[c] << 28 | 0x0A000008]
                        )
                    )
                tpl = [
                    int(x)
                    for x in (
                        ldrsh(0, 5, d0 + 4 * r + 2),
                        sub(0, 0, ("imm", 1)),
                        strh(0, 5, d0 + 4 * r + 2),
                        add(3, 0, ("imm", 1)),
                        clz(3, 3),
                        lsr(3, 3, 5),
                        add(15, 15, ("reg", 3, "lsl", 2)),
                        nop(),
                    )
                ]
                it.words = cw + tpl + [0xEA000000]
            elif it.kind in (BSR, JSR) and (it.kind == BSR or ins.static is not None):
                R = (it.pc + it.size) & 0xFFFFFFFF
                it.words = CALL_A + [int(x) for x in ba.load_const(0, R)] + CALL_B + [0xEB000000]
            elif it.kind == JMP and ins.static is not None:
                it.words = [0xEA000000]
            elif it.kind in (JSR, JMP):
                pre = ba.prefix_all(ins, d0)
                w = pre + [int(x) if not isinstance(x, tuple) else 0xEA000000 for x in it.bodyw]
                if it.kind == JSR:
                    w = (
                        w
                        + CALL_A
                        + [int(x) for x in ba.load_const(3, (it.pc + it.size) & 0xFFFFFFFF)]
                        + DYN_B
                    )
                it.words = w  # (the router words follow at run time)
            elif it.kind == RTS:
                it.words = RTS_WORDS
            words += it.words
        if self.opts.get(
            "sched"
        ):  # the asm scheduler over each run of plain instructions: the run's words go to its first item
            import sched_asm

            run = []

            def flush_run():
                nonlocal words
                if len(run) >= 1:
                    ws = [x for it in run for x in it.words]
                    res = sched_asm.get(d0)(ws) if len(ws) >= 4 else None
                    self.sched_runs = getattr(self, "sched_runs", []) + [(ws, res, run[0].pc)]
                    if res is not None:
                        run[0].words = res
                        for it in run[1:]:
                            it.words = []
                run.clear()

            for it in items:
                if it.kind == PLAIN and not it.bad:
                    run.append(it)
                else:
                    flush_run()
            flush_run()
            off = 0
            for it in items:
                it.off = off
                off += len(it.words)
            words = [x for it in items for x in it.words]
        return words

    def block(self, pc):
        b = self.blocks.get(pc)
        if b is not None:
            return b
        items, end, p = self.form(pc)
        words = self.emit(items)
        b = Block()
        b.entry, b.items, b.end = pc, items, end
        b.base = (self.top + 15) & ~15
        b.nwords = len(words) + 1 + 4 * sum(1 for i in items if i.kind != PLAIN) + 2  # + the stubs
        self.top = b.base + 4 * b.nwords
        for it in items:
            it.off += 0
        self.blocks[pc] = b
        self.nblocks += 1
        self.ntrans += len(items)
        bisect.insort(self.starts, b.base)
        self.entry_of[b.base] = pc
        return b


SZ = {"b": 2, "w": 2, "l": 4, "": 4, "s": 2}


def _imm(tok):
    t = tok.lstrip("#")
    neg = t.startswith("-")
    t = t.lstrip("-")
    v = int(t[1:], 16) if t.startswith("$") else int(t, 0)
    return -v if neg else v


def track_sp(kn, t, stats):
    """A7 as a number relative to the start (kn.sp), from the instruction's text: pushes, pops, LINK / UNLK
    (the frame register's value is remembered), MOVEM, LEA / ADDA / SUBA / ADDQ / SUBQ on A7.
    """
    if t is None:
        return
    m, sfx, ops, size = t
    if not hasattr(kn, "fp"):
        kn.fp = {}
    for o in ops:
        if o == "-(a7)":
            kn.sp -= SZ.get(sfx, 4)
        elif o == "(a7)+":
            kn.sp += SZ.get(sfx, 4)
    if m == "link":
        kn.sp -= 4
        kn.fp[ops[0]] = kn.sp
        kn.sp += _imm(ops[1])
    elif m == "unlk":
        kn.sp = kn.fp.get(ops[0], kn.sp) + 4
    elif m == "movem":
        regs = ops[0] if "-(a7)" in ops[1:] else ops[1]
        n = 0
        for part in regs.split("/"):
            if "-" in part:
                a, b = part.split("-")
                n += abs(int(b[1:]) - int(a[1:])) + 1
            else:
                n += 1
        if "-(a7)" in ops:
            kn.sp -= 4 * n if sfx != "w" else 2 * n
        elif "(a7)+" in ops:
            kn.sp += 4 * n if sfx != "w" else 2 * n
    elif m in ("addq", "adda", "subq", "suba", "lea") and ops[-1] == "a7":
        try:
            v = _imm(ops[0]) if ops[0].startswith("#") else None
            if m == "lea" and ops[0].endswith("(a7)"):
                v = _imm("#" + ops[0][:-4]) if ops[0][:-4] else 0
            if v is not None:
                kn.sp += v if m in ("addq", "adda", "lea") else -v
            else:
                stats["sp not tracked"] += 1
        except Exception:
            stats["sp not tracked"] += 1


def replay(rt, entries, limit, opts):
    import bisect

    sim = cyclemodel.Sim()
    buf = []
    carry = None

    def bucket_of(pc):
        if pc >= RING_BASE:
            starts = rt.starts
            j = bisect.bisect_right(starts, pc) - 1
            return rt.entry_of[starts[j]] if j >= 0 else -1
        return "router"

    ninstr = [0]
    stats = collections.Counter()

    def push(pc, w):
        buf.append((pc, w))

    def flush(final=False):
        nonlocal buf
        if not buf:
            return
        seq = buf if final else buf[:-1]
        if not final:
            seq = list(seq)
            last = buf[-1]
            seq[-1:] = [(seq[-1][0], seq[-1][1], last[0])] if seq else []
        if seq:
            sim.run(seq, bucket_of=bucket_of)
        buf = [] if final else [buf[-1]]

    def emit_words(blk, it, nwords=None):
        base = blk.base + 4 * it.off
        ws = it.words if nwords is None else it.words[:nwords]
        for k, w in enumerate(ws):
            push(base + 4 * k, w)
            ninstr[0] += 1
        if len(buf) > 60000:
            flush()

    def emit_routine(addr, words):
        for k, w in enumerate(words):
            push(addr + 4 * k, w)
            ninstr[0] += 1

    mem = rt.mem
    kn = known.Known()
    kn.sp = 0
    shadow = {}
    n = min(len(entries), limit) if limit else len(entries)
    i = 0
    cur = None
    idx = 0
    stack = []
    router = collections.Counter()
    while i < n:
        pc, op = entries[i]
        if cur is None or idx >= len(cur.items) or cur.items[idx].pc != pc:
            # (re)enter at pc: a new block (an entry that is not the continuation of the current one)
            kd = rt.kind_of(op)[0]
            if kd == STOPK:
                stats["interpret-one"] += 1
                i += 1
                cur = None
                continue
            cur = rt.block(pc)
            idx = 0
        blk = cur
        it = blk.items[idx]
        np = entries[i + 1][0] if i + 1 < n else None
        kd = it.kind
        ins = it.ins
        taken_target = None
        goto_path = False
        if kd == PLAIN:
            emit_words(blk, it)
            track_sp(kn, a7.text(mem, ins.pc), stats)
            idx += 1
            i += 1
            if idx >= len(blk.items):
                push(blk.base + 4 * (blk.nwords - 2), 0xEA000000)
                ninstr[0] += 1
                # the fall / stop stub's branch
                cur = None if blk.end != "fall" and blk.end != "stop" else None
            continue
        if kd == BCC:
            taken = np is not None and np != it.pc + it.size
            ws = it.words
            if ins.cond == 0:
                emit_words(blk, it)
                i += 1
                cur = None
                continue
            if taken:
                if it.xwords:
                    # (the dirty homes written back on the way out)
                    emit_routine(blk.base + 4 * it.off, it.xwords)
                emit_words(blk, it, len(ws) if len(ws) == 1 else len(ws) - 1 if ins.cond == 2 else len(ws))
            else:
                emit_words(blk, it)
            i += 1
            if taken:
                cur = None
            else:
                idx += 1
                if idx >= len(blk.items):
                    push(blk.base + 4 * (blk.nwords - 2), 0xEA000000)
                    ninstr[0] += 1
                    cur = None
            continue
        if kd == DBCC:
            taken = np is not None and np != it.pc + it.size
            emit_words(blk, it, len(it.words) if taken else len(it.words) - 1)
            i += 1
            if taken:
                cur = None
            else:
                idx += 1
                if idx >= len(blk.items):
                    push(blk.base + 4 * (blk.nwords - 2), 0xEA000000)
                    ninstr[0] += 1
                    cur = None
            continue
        if kd in (BSR, JSR) and (kd == BSR or ins.static is not None):
            emit_words(blk, it)
            i += 1
            if kn.sp is not None:
                key = (kn.sp & 0x3FC) if True else 0
                shadow[key] = (it.pc + it.size, blk, idx + 1)
                kn.sp -= 4
            stack.append((blk, idx + 1))
            cur = None
            continue
        if kd == JMP and ins.static is not None:
            emit_words(blk, it)
            i += 1
            cur = None
            continue
        if kd in (JSR, JMP):
            emit_words(blk, it)
            if kd == JSR and kn.sp is not None:
                shadow[kn.sp & 0x3FC] = (it.pc + it.size, blk, idx + 1)
                kn.sp -= 4
            if kd == JSR:
                stack.append((blk, idx + 1))
            emit_routine(GOTO, GOTO_WORDS[:11])
            push(GOTO + 44, 0xEA000000 if False else 0xE12FFF13)
            ninstr[0] += 1  # (the hit path skips the slow-path branch word)
            i += 1
            cur = None
            continue
        if kd == RTS:
            i += 1
            matched = False
            stats["rts sp unknown" if kn.sp is None else "rts sp known"] += 1
            if kn.sp is not None:
                kn.sp += 4
                ent = shadow.get(kn.sp & 0x3FC)
                if ent and ent[0] == np and ent[1] is not None:
                    matched = True
            if (
                opts.get("perfect_rts") and stack and stack[-1][0].items[stack[-1][1]].pc == np
                if (opts.get("perfect_rts") and stack and stack[-1][1] < len(stack[-1][0].items))
                else False
            ):
                matched = True
                ent = (np, stack[-1][0], stack[-1][1])
            if opts.get("perfect_rts") and stack:
                stack.pop()
            if opts.get("inline_rts", True):
                emit_words(blk, it)
            else:
                emit_routine(0x82002F04, RTS_WORDS)
            if matched:
                push(blk.base + 4 * (it.off + len(it.words)) + 4 if False else 0x82002F2C, 0xE12FFF1E)
                ninstr[0] += 1  # bx lr
                _, cb, ci = ent
                cur, idx = (
                    (cb, ci) if np is not None and ci < len(cb.items) and cb.items[ci].pc == np else (None, 0)
                )
                if (
                    cur is None and np is not None
                ):  # the call was the last item: the return lands on its fall stub
                    pass
            else:
                push(0x82002F28, 0xEA000000)
                ninstr[0] += 1
                emit_routine(GOTO, GOTO_WORDS[:11])
                push(GOTO + 44, 0xE12FFF13)
                ninstr[0] += 1
                cur = None
                stats["rts mismatch"] += 1
            continue
        i += 1
        cur = None
    flush(final=True)
    return sim, ninstr[0], stats


def main():
    args = sys.argv[1:]
    be = "--be" in args
    a = [x for x in args if not x.startswith("--")]
    drop = None
    limit = 0
    if "--drop" in args:
        drop = [int(x, 0) for x in args[args.index("--drop") + 1].split(",")]
        a = [x for x in a if x != args[args.index("--drop") + 1]]
    if "--limit" in args:
        limit = int(args[args.index("--limit") + 1])
        a = [x for x in a if x != args[args.index("--limit") + 1]]
    trace = open(a[0], "rb").read()
    image = open(a[1], "rb").read()
    base = int(a[2], 0)
    mem = ba.ImageMem(base, image)
    entries = [struct.unpack_from(">II" if be else "<II", trace, o) for o in range(0, len(trace) - 7, 8)]
    if drop:
        entries = [e for e in entries if not drop[0] <= e[0] <= drop[1]]
    g = Gen(read_offsets())
    for kv in args:
        if kv.startswith("--set="):
            k, v = kv[6:].split("=")
            setattr(cyclemodel, k, int(v))
            cyclemodel._cache.clear()
    for kv in args:
        if kv.startswith("--fold-off="):
            for f in kv[11:].split(","):
                constfold.FEATURES[f] = False
    opts = {
        "cfold": "--cfold" in args,
        "ofold": "--ofold" in args,
        "perfect_rts": "--perfect-rts" in args,
        "nolive": "--nolive" in args,
        "sched": "--sched" in args,
        "nofwd": "--nofwd" in args,
        "inline_rts": "--noinline-rts" not in args,
        "homes": "--homes" in args or any(x.startswith("--pin=") for x in args),
        "pin": next(([int(c) for c in x[6:].split(",")] for x in args if x.startswith("--pin=")), None),
        "homes_bound": "--homes-bound" in args,
        "pin_wt": "--pin-wt" in args,
        "homes_min": next((int(x[12:]) for x in args if x.startswith("--homes-min=")), 2),
    }
    rt = Runtime(mem, g, opts)
    sim, ninstr, stats = replay(rt, entries, limit, opts)
    n68 = min(len(entries), limit) if limit else len(entries)
    mis = sim.stall["mispredict"] // cyclemodel.MISPREDICT
    print(
        "%d 68K instructions: %d ARM instructions (%.2f per 68K), %d cycles (%.2f per 68K), %d mispredicts (%.2f per 100), %d blocks, %d translated; %s"
        % (
            n68,
            ninstr,
            ninstr / n68,
            sim.cycle,
            sim.cycle / n68,
            mis,
            100 * mis / n68,
            rt.nblocks,
            rt.ntrans,
            dict(stats),
        )
    )
    if "--top" in args:
        tot = sum(sim.bucket.values())
        for k, v in sim.bucket.most_common(12):
            print("   %-10s %6.1f%%" % (hex(k) if isinstance(k, int) else k, 100 * v / tot))
    print(
        "  stalls: " + ", ".join("%s %.1f%%" % (k, 100 * v / sim.cycle) for k, v in sim.stall.most_common())
    )
    return n68, ninstr, sim.cycle, mis, sim, rt


if __name__ == "__main__":
    main()
