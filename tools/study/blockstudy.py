#!/usr/bin/env python3
"""Block-shape study: what would a translator that forms length-capped blocks (instead of translating one
instruction at a time) get out of real instruction streams?
From a dynamic opcode trace (8-byte entries: PC, opcode; see tools/study/regreuse.py) and the code image,
replays the stream as a sequence of block runs and reports, per cap on
the block's source length (distinct instruction bytes):
  * instructions executed per block entry (1 for the per-instruction cache; fewer taken branches / lookups the
  larger it is),
  * why runs end (cap / unconditional transfer / taken exit / not-taken conditional branch in 'basic' mode /
  call-return),
  * in-block back-edges: taken backward branches whose target is already in the block (an internal loop: no
  second translation, no lookup) against those that leave it,
  * translations: entry-keyed blocks (a block per distinct entry PC, so a loop entered at its head AND
  from the middle is translated twice) against interior entry
    allowed (a block whose body contains the entry address is reused; only the unmapped remainder is
    translated), and the code duplication factor of the first,
  * dead flag writes inside a run (a write whose every bit is overwritten before any read, with every side
  exit and the run's end treated as reading all flags),
  * faulting instructions (DIVx, CHK, TRAP, TRAPV) that sit before the end of a run (a precise-state problem
  for the optimizer; address errors not counted).
  tools/study/blockstudy.py trace.bin image.bin base [--be] [--super] [--caps 64,120,240]
    --inline K,K..: also run with callees inlined into the run (a call whose callee, followed statically
    through every path, is at most K source bytes, ends every path in an RTS with net
      A7 delta 0, has no indirect jump or trap, and only writes the stack at or below the callee's entry SP
      minus the return address slot, is spliced in: the call and the RTS cost nothing
      and the run goes on; the pushed return address is still stored). --nested: callees may themselves call
      such callees (recursion refused); --depth N: nesting limit at run time (default 4).
    --homes: also measure how much of the D-register memory traffic forwarding, dead-store removal and
    register homes would remove.
    --super: a not-taken conditional branch does not end the run (a superblock with side exits); default is
    basic blocks (they end there).
"""

import sys, struct, collections, re
import capstone

md = capstone.Cs(capstone.CS_ARCH_M68K, capstone.CS_MODE_M68K_000)
md.detail = True
argv = sys.argv[1:]
be = "--be" in argv
sup = "--super" in argv
caps = [64, 120, 240]
ks = [0]
depth = 4
nested = "--nested" in argv
if "--caps" in argv:
    k = argv.index("--caps")
    caps = [int(x) for x in argv[k + 1].split(",")]
    del argv[k : k + 2]
if "--inline" in argv:
    k = argv.index("--inline")
    ks = [0] + [int(x) for x in argv[k + 1].split(",")]
    del argv[k : k + 2]
if "--depth" in argv:
    k = argv.index("--depth")
    depth = int(argv[k + 1])
    del argv[k : k + 2]
drop = None
if "--drop" in argv:
    k = argv.index("--drop")
    drop = [int(x, 0) for x in argv[k + 1].split(",")]
    del argv[k : k + 2]
args = [a for a in argv if not a.startswith("--")]
trace = open(args[0], "rb").read()
image = open(args[1], "rb").read()
base = int(args[2], 0)
entries = [struct.unpack_from(">II" if be else "<II", trace, o) for o in range(0, len(trace) - 7, 8)]
X, N, Z, V, C = 16, 8, 4, 2, 1
CC = {
    "eq": Z,
    "ne": Z,
    "cs": C,
    "cc": C,
    "lo": C,
    "hs": C,
    "mi": N,
    "pl": N,
    "vs": V,
    "vc": V,
    "hi": C | Z,
    "ls": C | Z,
    "ge": N | V,
    "lt": N | V,
    "gt": N | V | Z,
    "le": N | V | Z,
    "t": 0,
    "f": 0,
    "ra": 0,
}
BCC = {"b" + c for c in CC if c not in ("t", "f")}
UNCOND = {"bra", "jmp", "jsr", "bsr", "rts", "rte", "rtr", "trap", "illegal", "stop"}
FAULT = {"divu", "divs", "chk", "trap", "trapv"}
WRITES = {m: 31 for m in "add addi addq sub subi subq addx subx neg negx asl asr lsl lsr roxl roxr".split()}
WRITES.update(
    {
        m: 15
        for m in "cmp cmpa cmpi cmpm tst move moveq and andi or ori eor eori not clr ext swap mulu muls divu divs rol ror tas".split()
    }
)
WRITES.update({m: Z for m in "btst bchg bclr bset".split()})
WRITES.update({m: X | Z | C for m in "abcd sbcd nbcd".split()})
WRITES["ori"] = WRITES["andi"] = WRITES["eori"] = 15
READS = {m: X | Z for m in "addx subx negx abcd sbcd nbcd".split()}
READS.update({"roxl": X, "roxr": X})
NOFLAGS = {
    "adda",
    "suba",
    "movea",
    "lea",
    "pea",
    "movem",
    "link",
    "unlk",
    "nop",
    "exg",
    "jmp",
    "jsr",
    "bsr",
    "bra",
    "rts",
    "dbra",
    "dbf",
}
cache = {}


def decode(pc):
    if pc in cache:
        return cache[pc]
    off = pc - base
    dec = None
    if 0 <= off < len(image):
        i = next(md.disasm(image[off : off + 12], pc), None)
        if i is not None:
            m = i.mnemonic.split(".")[0]
            ops = i.op_str
            kind = "seq"
            if m in UNCOND:
                kind = "uncond"
            elif m in BCC or m in ("dbra", "dbf") or (m.startswith("db") and m[2:] in CC):
                kind = "cond"
            rd = wr = 0
            if m in BCC:
                rd = CC[m[1:]]
            elif m.startswith("db") and m[2:] in CC:
                rd = CC[m[2:]]
            elif m in ("dbra", "dbf"):
                rd = 0
            elif m.startswith("s") and m[1:] in CC:
                rd = CC[m[1:]]
                wr = 0
            elif m in NOFLAGS:
                pass
            elif "sr" in ops.lower().split(",")[-1:] or "ccr" in ops.lower().split(",")[-1:]:
                wr = 31 if m == "move" else 0
                rd = 31 if m != "move" else 0
            elif ops.lower().startswith(("sr", "ccr")):
                rd = 31
            elif m in WRITES:
                wr = WRITES[m]
                rd = READS.get(m, 0)
                dst = ops.split(",")[-1].strip()
                if (
                    m in ("addq", "subq", "add", "sub")
                    and len(dst) == 2
                    and dst[0] == "a"
                    and dst[1].isdigit()
                    and m in ("addq", "subq")
                ):
                    wr = 0
            elif m in ("trap", "trapv", "rte", "rtr", "illegal", "stop", "bkpt", "reset", "tst"):
                rd = 31
            else:
                rd = 31  # unknown: conservative (reads everything, writes nothing)
            dec = (i.size, m, kind, rd, wr)
    cache[pc] = dec
    return dec


def split_ops(s):
    out = []
    d = 0
    cur = ""
    for ch in s:
        if ch == "(":
            d += 1
        elif ch == ")":
            d -= 1
        if ch == "," and d == 0:
            out.append(cur.strip())
            cur = ""
        else:
            cur += ch
    if cur.strip():
        out.append(cur.strip())
    return out


rawc = {}


def raw(pc):
    if pc in rawc:
        return rawc[pc]
    off = pc - base
    r = None
    if 0 <= off < len(image):
        i = next(md.disasm(image[off : off + 12], pc), None)
        if i is not None:
            mn = i.mnemonic.split(".")
            r = (i.size, mn[0], mn[1] if len(mn) > 1 else "l", [o.lower() for o in split_ops(i.op_str)])
    rawc[pc] = r
    return r


def imm(tok):
    m = re.fullmatch(r"#(-?)(\$?)([0-9a-f]+)", tok)
    if not m:
        return None
    v = int(m.group(3), 16 if m.group(2) else 10)
    return -v if m.group(1) else v


def reglist(tok):
    n = 0
    for part in tok.split("/"):
        if "-" in part:
            a, b = part.split("-")
            n += int(b[1:]) - int(a[1:]) + 1
        else:
            n += 1
    return n


NOWRITE_M = {"cmp", "cmpi", "cmpa", "cmpm", "tst", "btst", "pea", "jmp", "jsr", "lea"}


def a7effect(m, sfx, ops, d, frames):
    """(new delta, new frames) or a string naming why the callee is refused."""
    sz = 2 if sfx in "bw" else 4
    d0 = d
    frames = dict(frames)
    if m == "movem":
        if ops[-1] == "-(a7)":
            d -= reglist(ops[0]) * sz
        elif ops[0] == "(a7)+":
            d += reglist(ops[-1]) * sz
    elif m == "pea":
        d -= 4
    elif m == "link":
        v = imm(ops[1])
        if v is None:
            return "link"
        d -= 4
        frames[ops[0]] = d
        d += v
    elif m == "unlk":
        if ops[0] not in frames:
            return "unlk without link"
        d = frames.pop(ops[0]) + 4
    else:
        for t in ops:
            if t == "-(a7)":
                d -= sz
            elif t == "(a7)+":
                d += sz
        if ops and ops[-1] == "a7":
            v = imm(ops[0]) if len(ops) > 1 else None
            if m in ("addq", "addi", "add", "adda") and v is not None:
                d += v
            elif m in ("subq", "subi", "sub", "suba") and v is not None:
                d -= v
            elif m == "lea" and re.fullmatch(r"-?\$[0-9a-f]+\(a7\)", ops[0]):
                d += int(ops[0].split("(")[0].replace("$", ""), 16)
            else:
                return "A7 written"
        if ops and m not in NOWRITE_M:
            mm = re.fullmatch(r"(-?)(\$[0-9a-f]+)?\(a7\)", ops[-1])
            if mm:
                off = int(mm.group(2)[1:], 16) * (-1 if mm.group(1) else 1) if mm.group(2) else 0
                if d + off < 4 and d + off + {"b": 1, "w": 2}.get(sfx, 4) > 0:
                    return "writes the return address"
    if d > 0:
        return "pops past the return address"
    return d, frames


acache = {}


def analyse(entry, stack=()):
    """(ok, {pc: size} of every instruction reachable in the callee, why not)."""
    if entry in acache:
        return acache[entry]
    if entry in stack or len(stack) > 3:
        return (False, {}, "recursive")
    res = analyse1(entry, stack + (entry,))
    acache[entry] = res
    return res


def analyse1(entry, stack):
    sset = {}
    states = {}
    work = [(entry, 0, ())]

    def no(why):
        return (False, {}, why)

    while work:
        pc, d, fr = work.pop()
        if pc in states:
            if states[pc] != d:
                return no("A7 differs between paths")
            continue
        states[pc] = d
        r = raw(pc)
        if r is None:
            return no("undecodable")
        size, m, sfx, ops = r
        sset[pc] = size
        if sum(sset.values()) > 1024:
            return no("size")
        eff = a7effect(m, sfx, ops, d, dict(fr))
        if isinstance(eff, str):
            return no(eff)
        d2, fr2 = eff
        fr2 = tuple(sorted(fr2.items()))
        tm = re.fullmatch(r"\$([0-9a-f]+)(\.[lw])?", ops[-1]) if ops else None
        tgt = int(tm.group(1), 16) if tm else None
        if m == "rts":
            if d != 0:
                return no("RTS with A7 not back at the return address")
        elif m in ("rte", "rtr", "jmp", "trap", "trapv", "illegal", "stop", "reset", "chk"):
            return no("jmp/trap/rte")
        elif m == "bra":
            if tgt is None:
                return no("branch target")
            work.append((tgt, d2, fr2))
        elif m in BCC or m.startswith("db"):
            if tgt is None:
                return no("branch target")
            work.append((tgt, d2, fr2))
            work.append((pc + size, d2, fr2))
        elif m in ("jsr", "bsr"):
            if not nested:
                return no("calls another routine")
            if tgt is None:
                return no("indirect call")
            ok, s2, why = analyse(tgt, stack)
            if not ok:
                return no("callee: " + why)
            sset.update(s2)
            work.append((pc + size, d2, fr2))
        else:
            work.append((pc + size, d2, fr2))
    return (True, sset, None)


HOMES = "--homes" in argv
from capstone.m68k_const import *


def _dn(r):
    return r - M68K_REG_D0 if M68K_REG_D0 <= r <= M68K_REG_D7 else None


def _an(r):
    return r - M68K_REG_A0 if M68K_REG_A0 <= r <= M68K_REG_A7 else None


NOWRITE2 = {"cmp", "cmpi", "cmpa", "cmpm", "tst", "btst", "pea", "jmp", "jsr", "chk", "movem"}
WRITEONLY2 = {
    "move",
    "moveq",
    "movea",
    "lea",
    "clr",
    "scc",
    "st",
    "sf",
    "shi",
    "sls",
    "scs",
    "sne",
    "seq",
    "svc",
    "svs",
    "spl",
    "smi",
    "sge",
    "slt",
    "sgt",
    "sle",
    "ext",
    "swap",
}
dcache = {}


def dinfo(pc):
    """-> (D registers read, D registers written, A registers touched) by the instruction at pc:
    memory-resident state the translated code loads / stores.
    """
    if pc in dcache:
        return dcache[pc]
    off = pc - base
    res = (set(), set(), set())
    if 0 <= off < len(image):
        i = next(md.disasm(image[off : off + 12], pc), None)
        if i is not None:
            m = i.mnemonic.split(".")[0]
            rd, wr, ar = set(), set(), set()
            try:
                ops = i.operands
            except Exception:
                ops = []  # (data in the code stream: SKIPDATA)
            for k, o in enumerate(ops):
                dst = (k == len(ops) - 1) and len(ops) >= 1
                if o.type == 1:
                    d = _dn(o.reg)
                    a = _an(o.reg)
                    if a is not None:
                        ar.add(a)
                    if d is not None:
                        if dst and (
                            len(ops) > 1
                            or m in ("clr", "neg", "not", "ext", "swap", "scc", "st", "sf")
                            or m.startswith("s")
                            and m[1:] in CC
                        ):
                            if m in NOWRITE2:
                                rd.add(d)
                            elif m in WRITEONLY2 and m not in ("ext", "swap"):
                                wr.add(d)
                            else:
                                rd.add(d)
                                wr.add(d)
                        else:
                            rd.add(d)
                elif o.type == 3:
                    for rr in (o.mem.base_reg, o.mem.index_reg):
                        try:
                            if _an(rr) is not None:
                                ar.add(_an(rr))
                            if _dn(rr) is not None:
                                rd.add(_dn(rr))
                        except Exception:
                            pass
            if m.startswith("db") and ops and ops[0].type == 1 and _dn(ops[0].reg) is not None:
                rd.add(_dn(ops[0].reg))
                wr.add(_dn(ops[0].reg))
            if m in ("jsr", "bsr", "rts", "rtr", "rte", "link", "unlk", "pea"):
                ar.add(7)
            res = (rd, wr, ar)
    dcache[pc] = res
    return res


def homes_study(runs):
    """What the register traffic of the D registers (each read an LDR, each write an STR in the translated code)
    becomes with
    forwarding alone, with dead stores dropped, and with k registers kept as homes for the run (free
    registers: r4, lr, and the A registers the run never touches,
    which cost a spill at entry and a reload at exit)."""
    KS = (0, 2, 4, 6, 8)
    tot = 0
    fwd = 0
    dead = 0
    after = {k: 0 for k in KS}
    free_hist = collections.Counter()
    instr = 0
    for pc0, path, _ in runs:
        seq = [dinfo(p[0]) for p in path]
        instr += len(seq)
        reads = sum(len(r) for r, w, a in seq)
        writes = sum(len(w) for r, w, a in seq)
        tot += reads + writes
        # forwarding: a read of the register the previous instruction wrote
        f = 0
        for t in range(1, len(seq)):
            f += len(seq[t][0] & seq[t - 1][1])
        # dead stores: a write overwritten later in the run by one that does not read it, nothing reading it
        # in between
        dd = 0
        for t, (r, w, a) in enumerate(seq):
            for reg in w:
                for u in range(t + 1, len(seq)):
                    if reg in seq[u][0]:
                        break
                    if reg in seq[u][1]:
                        dd += 1
                        break
        fwd += f
        dead += dd
        used_a = set().union(*[a for r, w, a in seq]) if seq else set()
        free = 2 + (8 - len(used_a))
        free_hist[min(free, 10)] += 1
        # per D register: accesses, whether the first access is a read, whether it is written
        acc = {}
        for t, (r, w, a) in enumerate(seq):
            for reg in r | w:
                e = acc.setdefault(reg, [0, None, False])
                e[0] += (reg in r) + (reg in w)
                if e[1] is None:
                    e[1] = reg in r
                if reg in w:
                    e[2] = True
        for k in KS:
            n_home = min(k, free)
            save = 0
            chosen = []
            cand = sorted(acc.items(), key=lambda kv: -kv[1][0])
            used_free = 0
            for reg, (cnt, first_read, written) in cand:
                if len(chosen) >= n_home:
                    break
                # the first two free registers (r4, lr) cost no spill
                cost = (1 if first_read else 0) + (1 if written else 0) + (2 if used_free >= 2 else 0)
                if cnt - cost <= 0:
                    continue
                chosen.append(reg)
                used_free += 1
                save += cnt - cost
            rest = reads + writes - sum(acc[c][0] for c in chosen)
            f2 = sum(len((seq[t][0] & seq[t - 1][1]) - set(chosen)) for t in range(1, len(seq)))
            after[k] += (reads + writes) - save - (f2 if k else f) - (dd if k == 0 else 0) * 0
    print("      D-register memory operations: %d (%.2f per instruction)" % (tot, tot / max(1, instr)))
    print(
        "        forwarding the value just stored: -%.1f%%;  + dead stores dropped: -%.1f%%"
        % (100 * fwd / tot, 100 * (fwd + dead) / tot)
    )
    print(
        "        homes (k registers, forwarding on the rest): "
        + ", ".join("k=%d: -%.1f%%" % (k, 100 * (tot - after[k]) / tot) for k in KS if k)
    )
    print(
        "        registers available per run (r4, lr, untouched A registers): "
        + " ".join("%d:%.0f%%" % (k, 100 * v / len(runs)) for k, v in sorted(free_hist.items()))
    )


def study(cap, K):
    n = len(entries)
    runs = []  # each: (entry_pc, [(pc, dec)], reason, backedges_in, backedges_out)
    i = 0
    reasons = collections.Counter()
    be_in = be_out = fwd_in = 0
    skipped = 0
    ncalls = inl_calls = inl_rets = 0
    rej = collections.Counter()
    inl_bytes = 0
    while i < n:
        pc0 = entries[i][0]
        if decode(pc0) is None:
            i += 1
            skipped += 1
            continue
        covered = {}
        nbytes = 0
        path = []
        j = i
        reason = None
        ras = []
        while True:
            pc = entries[j][0]
            dec = decode(pc)
            if dec is None:
                reason = "unknown"
                break
            size, m, kind, rd, wr = dec
            if pc not in covered:
                covered[pc] = size
                nbytes += size
            path.append((pc, dec))
            if j + 1 >= n:
                reason = "end of trace"
                j += 1
                break
            nxt = entries[j + 1][0]
            taken = nxt != pc + size and not (pc + size in skip_at and nxt == pc + size + skip_at[pc + size])
            if kind == "uncond" and not (
                m in ("jsr", "bsr") and not taken
            ):  # (a call whose callee was dropped from the stream falls through)
                if m == "bra" and taken and nxt in covered:
                    if nxt <= pc:
                        be_in += 1
                    else:
                        fwd_in += 1
                    path[-1] = (pc, dec, "edge")
                    j += 1
                    continue
                if m in ("jsr", "bsr"):
                    ncalls += 1
                    if K:
                        ok, sset, why = analyse(nxt)
                        if ok and sum(sset.values()) > K:
                            ok = False
                            why = "larger than K"
                        if ok and len(ras) >= depth:
                            ok = False
                            why = "nesting depth"
                        if ok:
                            add = sum(sz for q, sz in sset.items() if q not in covered)
                            if nbytes + add > cap:
                                ok = False
                                why = "would exceed the cap"
                        if ok:
                            for q, sz in sset.items():
                                covered.setdefault(q, sz)
                            nbytes += add
                            inl_bytes += add
                            ras.append(pc + size)
                            inl_calls += 1
                            path[-1] = (pc, dec, "call")
                            j += 1
                            continue
                        rej[why] += 1
                if m == "rts" and ras and nxt == ras[-1]:
                    ras.pop()
                    inl_rets += 1
                    path[-1] = (pc, dec, "ret")
                    j += 1
                    continue
                reason = (
                    "call/return"
                    if m in ("jsr", "bsr", "rts", "rtr", "rte")
                    else ("trap" if m in ("trap", "illegal", "stop") else "jump")
                )
                j += 1
                break
            if kind == "cond":
                if taken:
                    if nxt in covered:
                        if nxt <= pc:
                            be_in += 1
                        else:
                            fwd_in += 1
                        path[-1] = (pc, dec, "edge")
                        j += 1
                        continue
                    if nxt <= pc:
                        be_out += 1
                    reason = "taken exit"
                    j += 1
                    break
                if not sup:
                    reason = "not-taken (basic)"
                    j += 1
                    break
            elif taken:
                reason = "discontinuity"
                j += 1
                break
            nd = decode(nxt)
            if nd is not None and nxt not in covered and nbytes + nd[0] > cap:
                reason = "cap"
                j += 1
                break
            j += 1
        reasons[reason] += 1
        runs.append((pc0, path, reason))
        i = max(j, i + 1)
    total = sum(len(r[1]) for r in runs)
    if HOMES:
        pending_homes = runs
    # translations
    byentry = {}  # entry -> set of pcs (union of runs from that entry)
    for pc0, path, _ in runs:
        byentry.setdefault(pc0, {}).update({p[0]: p[1][0] for p in path})
    p1_bytes = sum(sum(s.values()) for s in byentry.values())
    uniq = {}
    for s in byentry.values():
        uniq.update(s)
    mapped = set()
    p2_trans = 0
    for pc0, path, _ in runs:  # interior entry allowed: a translation per maximal unmapped stretch
        inrun = False
        for p in path:
            if p[0] in mapped:
                inrun = False
            else:
                if not inrun:
                    p2_trans += 1
                inrun = True
                mapped.add(p[0])
    # dead flags, faults
    dead_all = dead_bits = wbits = wcount = 0
    faults_mid = 0
    bl = 0
    for _, path, _ in runs:
        live = 31
        seg = []
        for k in range(len(path) - 1, -1, -1):
            p = path[k]
            size, m, kind, rd, wr = p[1]
            if (len(p) > 2 and p[2] == "edge") or (kind == "cond" and k != len(path) - 1):
                live = 31  # back-edge or side exit: everything live
            if wr:
                wcount += 1
                wbits += bin(wr).count("1")
                d = wr & ~live
                dead_bits += bin(d).count("1")
                if d == wr:
                    dead_all += 1
            live = (live & ~wr) | rd
            if m in FAULT and k != len(path) - 1:
                faults_mid += 1
    print(
        "  cap %3d bytes%s: %7d runs, %5.2f instructions per run (1.00 for slots)"
        % (
            cap,
            (
                ", callees <= %d bytes inlined%s" % (K, " (nested)" if nested else " (leaf only)")
                if K
                else ", no inlining"
            ),
            len(runs),
            total / len(runs),
        )
    )
    if K:
        print(
            "      calls: %d dynamic, %d inlined (%.0f%%; %d inlined returns, %.0f bytes of callee per inlined call); refused: %s"
            % (
                ncalls,
                inl_calls,
                100 * inl_calls / max(1, ncalls),
                inl_rets,
                inl_bytes / max(1, inl_calls),
                ", ".join("%s %d" % kv for kv in rej.most_common()) or "none",
            )
        )
    else:
        print("      calls: %d dynamic (%.1f%% of instructions)" % (ncalls, 100 * ncalls / total))
    print(
        "      run ends: "
        + ", ".join("%s %.0f%%" % (k, 100 * v / len(runs)) for k, v in reasons.most_common())
    )
    tb = be_in + be_out
    print(
        "      taken backward branches: %d inside their block (%.0f%%), %d leave it; %d taken forward branches stay inside"
        % (be_in, 100 * be_in / max(1, tb), be_out, fwd_in)
    )
    print(
        "      translations: %d entry-keyed (%d source bytes translated for %d distinct: x%.2f duplication), %d with interior entry allowed"
        % (len(byentry), p1_bytes, sum(uniq.values()), p1_bytes / sum(uniq.values()), p2_trans)
    )
    print(
        "      flag writes: %.1f%% of instructions write flags; %.1f%% of those are dead in full (%.1f%% of the flag bits)"
        % (100 * wcount / total, 100 * dead_all / max(1, wcount), 100 * dead_bits / max(1, wbits))
    )
    print(
        "      faulting instructions (DIV/CHK/TRAP) before the end of a run: %d (%.2f%% of instructions)"
        % (faults_mid, 100 * faults_mid / total)
    )
    if HOMES:
        homes_study(runs)


skip_at = {}  # pc of a dropped call -> its size (the stream goes straight past it)
# --drop lo,hi: remove a routine and the calls to it from the stream (e.g. a console putchar that polls a UART
# inside the traced window)
if drop:
    lo, hi = drop
    keep = []
    n0 = len(entries)
    for k2, (pc, op) in enumerate(entries):
        if lo <= pc <= hi:
            continue
        if k2 + 1 < n0 and lo <= entries[k2 + 1][0] <= hi:
            d = decode(pc)
            if d and d[1] in ("jsr", "bsr"):
                skip_at[pc] = d[0]
                continue
        keep.append((pc, op))
    entries = keep
    print("--drop: %d of %d entries removed" % (n0 - len(entries), n0))
print(
    "%d instructions traced, %s mode"
    % (
        len(entries),
        (
            "superblocks (not-taken conditional branches do not end a run)"
            if sup
            else "basic blocks (a conditional branch ends a run)"
        ),
    )
)
for c in caps:
    for k in ks:
        study(c, k)
