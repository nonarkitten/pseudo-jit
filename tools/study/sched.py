#!/usr/bin/env python3
"""How much would instruction scheduling buy? Takes an executed ARM trace, cuts it into straight-line windows (a
branch, an MRS/MSR
or an LDM/STM ends one), list-schedules each window under the real dependencies -- registers, the flags,
memory order (stores stay
in order, a load never passes a store) and predicated writes -- and replays the original and the rescheduled
trace through
tools/study/cyclemodel.py. Two bounds: everything movable (the trace as a whole), and only code that sits in
the cache slots movable
(the shared DRSR bodies stay as they are, which is what a fill-time scheduler could do without new copies)."""

import collections, struct, cyclemodel
from capstone.arm import ARM_OP_MEM


def memref(pc, word):
    """(base register, displacement, size) of an instruction's memory operand, or None. r5 + a constant is
    cpu_t (the D registers, SR, ...).
    """
    insn = next(cyclemodel.md.disasm(struct.pack("<I", word), pc), None)
    if insn is None:
        return None
    for op in insn.operands:
        if op.type == ARM_OP_MEM:
            m = insn.mnemonic
            size = (
                1
                if ("b" in m[3:5] and m.startswith(("ldr", "str")))
                else (
                    2 if m[3:5] in ("h", "sh", "sb") and m.startswith(("ldr", "str")) and "h" in m[3:5] else 4
                )
            )
            if m.startswith(("ldrsb", "ldrb", "strb")):
                size = 1
            elif m.startswith(("ldrsh", "ldrh", "strh")):
                size = 2
            return (insn.reg_name(op.mem.base), op.mem.disp if op.mem.index == 0 else None, size)
    return None


def may_alias(a, b):
    """Can two memory operands touch the same bytes? cpu_t (r5 + constant) never overlaps guest memory or the
    guest stack.
    """
    if a is None or b is None:
        return True
    (ba, da, sa), (bb, db, sb) = a, b
    if ba == "r5" and bb == "r5" and da is not None and db is not None:
        return da < db + sb and db < da + sa
    if (ba == "r5") != (bb == "r5"):
        return False
    return True


def deps(window):
    """For each instruction of the window, the indices it must follow."""
    n = len(window)
    preds = [set() for _ in range(n)]
    last_w = {}
    readers = collections.defaultdict(list)
    last_fw = None
    flag_readers = []
    mem_ops = []
    for k, (pc, word, i) in enumerate(window):
        reads = set(i.reads)
        if i.cond:
            reads |= set(i.writes)  # a predicated write keeps the old value if the condition fails
        for r in reads:
            if r in last_w:
                preds[k].add(last_w[r])  # RAW
        for r in i.writes:
            if r in last_w:
                preds[k].add(last_w[r])  # WAW
            for j in readers[r]:
                preds[k].add(j)  # WAR
        if i.rflags and last_fw is not None:
            preds[k].add(last_fw)
        if i.wflags:
            if last_fw is not None:
                preds[k].add(last_fw)
            for j in flag_readers:
                preds[k].add(j)
        if i.kind in ("mem",):
            is_load = i.name.startswith("ldr")
            me = memref(pc, word)
            for j, (jm, jload) in mem_ops:
                if (not is_load or not jload) and may_alias(me, jm):
                    preds[k].add(j)  # any pair with a store, unless the addresses cannot overlap
            mem_ops.append((k, (me, is_load)))
        for r in reads:
            readers[r].append(k)
        for r in i.writes:
            last_w[r] = k
            readers[r] = []
        if i.rflags:
            flag_readers.append(k)
        if i.wflags:
            last_fw = k
            flag_readers = []
        preds[k].discard(k)
    return preds


def schedule(window):
    """Greedy list scheduling: among the instructions whose predecessors are done, take the one that can start
    soonest, then the one with the longest path.
    """
    n = len(window)
    if n < 3:
        return list(range(n))
    preds = deps(window)
    succs = [[] for _ in range(n)]
    for k in range(n):
        for j in preds[k]:
            succs[j].append(k)
    prio = [0] * n
    for k in range(n - 1, -1, -1):
        lat = window[k][2].lat
        prio[k] = lat + max((prio[s] for s in succs[k]), default=0)
    done = set()
    order = []
    ready_at = collections.defaultdict(int)
    flag_at = 0
    cyc = 0
    remaining = set(range(n))
    while remaining:
        cands = [k for k in remaining if preds[k] <= done]

        def start(k):
            i = window[k][2]
            t = 0
            for r in i.reads:
                t = max(t, ready_at[r])
            if i.rflags:
                t = max(t, flag_at)
            return t

        k = min(cands, key=lambda k: (max(start(k), cyc), -prio[k], k))
        i = window[k][2]
        t = max(start(k), cyc)
        for r in i.writes:
            ready_at[r] = t + i.lat
        if i.wflags:
            flag_at = t + 1
        cyc = t  # (in-order issue: the next one cannot start earlier)
        order.append(k)
        done.add(k)
        remaining.discard(k)
    return order


def reschedule(seq, movable):
    """seq: [(pc, word)]. Returns [(pc, word, next_pc)] with each window reordered."""
    out = []
    window = []
    n = len(seq)

    def flush():
        nonlocal window
        if window:
            for k in schedule(window):
                out.append(window[k][:2])
        window = []

    for idx, (pc, word) in enumerate(seq):
        i = cyclemodel.decode(pc, word)
        if i.branch or i.serial or i.kind == "multi" or not movable(pc):
            flush()
            out.append((pc, word))
        else:
            window.append((pc, word, i))
    flush()
    # next-pc of every element: the original successor of the instruction that ends each window must stay what
    # it was
    nxt = {}
    res = []
    orig_next = [seq[k + 1][0] if k + 1 < n else seq[k][0] + 4 for k in range(n)]
    # the i-th element of the new order was originally followed by orig_next of the slot it now fills;
    # branches keep their own
    for k, (pc, word) in enumerate(out):
        i = cyclemodel.decode(pc, word)
        res.append((pc, word, orig_next[k] if i.branch else (out[k + 1][0] if k + 1 < n else pc + 4)))
    # a branch's successor is the first instruction of the next window as it was *originally*: take it from
    # the original trace by position
    return res


def study(segs, movable, label, runs):
    """segs: the two traces (run lengths ra, rb). -> prints per-run cycles for the original and the
    rescheduled order.
    """
    base = []
    sched = []
    wins = collections.Counter()
    for s in segs:
        a = cyclemodel.Sim()
        base.append(a.run(s))
        r = reschedule(s, movable)
        b = cyclemodel.Sim()
        sched.append(b.run(r))
    n = runs
    cb = (base[1] - base[0]) / n
    cs = (sched[1] - sched[0]) / n
    print(f"  {label}: {cb:.0f} -> {cs:.0f} modelled cycles per run ({100.0 * (cs - cb) / cb:+.1f}%)")
    return cb, cs


def flatten(seq):
    """The trace as if every body were inlined: unconditional direct branches and returns removed (their
    targets are what follows).
    """
    out = []
    for pc, word in seq:
        i = cyclemodel.decode(pc, word)
        if i.branch in ("direct", "return") and not i.cond:
            continue
        out.append((pc, word))
    return out


def study_flat(segs, label, runs):
    flat = [flatten(s) for s in segs]
    plain = [cyclemodel.Sim().run(f) for f in flat]
    sch = [cyclemodel.Sim().run(reschedule(f, lambda pc: True)) for f in flat]
    cp = (plain[1] - plain[0]) / runs
    cs = (sch[1] - sch[0]) / runs
    removed = ((len(segs[1]) - len(flat[1])) - (len(segs[0]) - len(flat[0]))) / runs
    print(
        f"  {label}: {cp:.0f} cycles flattened ({removed:.0f} branch/return instructions per run gone), {cs:.0f} flattened and scheduled ({100.0 * (cs - cp) / cp:+.1f}% from the scheduling alone)"
    )


def calls(seq, in_drsr):
    """Call episodes from the slots into shared bodies: (index of the bl, index of the returning bx,
    straight-line length of the body without its bx).
    """
    out = []
    n = len(seq)
    for k in range(n - 1):
        i = cyclemodel.decode(*seq[k])
        if i.branch == "direct" and i.link and not i.cond and in_drsr(seq[k + 1][0]):
            j = k + 1
            length = 0
            ok = True
            while j < n:
                b = cyclemodel.decode(*seq[j])
                if b.branch == "return" and not b.cond:
                    break
                if b.branch or b.serial or b.kind == "multi":
                    ok = False
                    break
                length += 1
                j += 1
            if ok and j < n:
                out.append((k, j, length))
    return out


def study_inline_length(segs, in_drsr, runs, limits=(1, 2, 3, 4, 5, 6, 8, 12)):
    """What if every shared body of at most N straight-line instructions were inlined (its bl and bx lr gone)?"""
    base = [cyclemodel.Sim().run(s) for s in segs]
    hist = collections.Counter()
    dyn = collections.Counter()
    for k, j, L in calls(segs[1], in_drsr):
        hist[L] += 1
    tot = sum(hist.values())
    print(
        f"  {tot / 1:.0f} calls into shared bodies in the long run; executed length of the bodies (instructions, excluding the bx lr):"
    )
    acc = 0
    for L in sorted(hist):
        acc += hist[L]
        if L <= 12:
            print(f"    {L:2d}: {hist[L]:6d} calls   cumulative {100.0 * acc / tot:5.1f}%")
    cb = (base[1] - base[0]) / runs
    for N in limits:
        flat = []
        for s in segs:
            drop = set()
            for k, j, L in calls(s, in_drsr):
                if L <= N:
                    drop.add(k)
                    drop.add(j)
            flat.append([e for idx, e in enumerate(s) if idx not in drop])
        c = [cyclemodel.Sim().run(f) for f in flat]
        cn = (c[1] - c[0]) / runs
        print(
            f"    bodies of up to {N:2d} instructions inlined: {cn:.0f} cycles per run ({100.0 * (cn - cb) / cb:+.1f}%)"
        )


def taken_branches(segs, bucket, runs):
    """Every taken branch / return per run, by (kind, where from -> where to), with the cycles the model
    charges for it.
    """
    cnt = [collections.Counter(), collections.Counter()]
    for t, s in enumerate(segs):
        for k in range(len(s) - 1):
            pc, word = s[k]
            i = cyclemodel.decode(pc, word)
            if i.branch and s[k + 1][0] != pc + 4:
                kind = (
                    "return"
                    if i.branch == "return"
                    else (
                        "indirect"
                        if i.branch == "indirect"
                        else "call" if i.link else ("cond " if i.cond else "") + "branch"
                    )
                )
                cnt[t][(kind, bucket(pc), bucket(s[k + 1][0]))] += 1
    out = collections.Counter()
    for key in cnt[1]:
        out[key] = (cnt[1][key] - cnt[0][key]) / runs
    print("  taken branches and returns per run (kind, from -> to):")
    for key, v in out.most_common(14):
        print(f"    {v:7.1f}  {key[0]:14s} {key[1]:18s} -> {key[2]}")
    print(f"    {sum(out.values()):7.1f}  in all")


def slot_branch_distances(segs, in_cache, runs):
    """Direct unconditional branches that stay inside the cache: how far forward they go, in words (a padding
    skip is a few words).
    """
    cnt = [collections.Counter(), collections.Counter()]
    for t, s in enumerate(segs):
        for k in range(len(s) - 1):
            pc, word = s[k]
            i = cyclemodel.decode(pc, word)
            if i.branch == "direct" and not i.cond and not i.link and in_cache(pc) and in_cache(s[k + 1][0]):
                d = (s[k + 1][0] - pc) // 4
                cnt[t][d if abs(d) <= 12 else ("far fwd" if d > 0 else "far back")] += 1
    print("  direct unconditional branches inside the cache, by distance in words, per run:")
    keys = set(cnt[1]) | set(cnt[0])
    for key in sorted(keys, key=lambda k: (isinstance(k, str), k if not isinstance(k, str) else 0)):
        v = (cnt[1][key] - cnt[0][key]) / runs
        if v >= 0.5:
            print(f"    {str(key):>9}: {v:6.1f}")
