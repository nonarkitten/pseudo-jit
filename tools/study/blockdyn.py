#!/usr/bin/env python3
"""How long are the blocks when they run? The trace is replayed through the blocks the real assembler would
build: a block is entered at a pc, the trace is followed
for as long as it stays on the block's translated addresses, and the block ends at one of its terminating
instructions (call, RTS, JMP) or when the trace leaves it.
Reports instructions executed per block entry (the thing that decides how much each block's overheads and each
pass can do) and why blocks end, for the formation
policies named on the command line.

  .venv/bin/python tools/study/blockdyn.py trace.bin image.bin base [--be] [--drop lo,hi] [--cap 120] [--nest
  2] [--k 120] [--follow] [--noinline]
"""

import sys, os, struct, collections

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("../gen", "..")]
from gen import Gen, read_offsets
import blockasm, a7


def load(args):
    a = [x for x in args if not x.startswith("--")]
    opt = {}
    for key in ("cap", "nest", "k"):
        if "--" + key in args:
            opt[key] = int(args[args.index("--" + key) + 1])
            a = [x for x in a if x != args[args.index("--" + key) + 1]]
    drop = None
    if "--drop" in args:
        drop = [int(x, 0) for x in args[args.index("--drop") + 1].split(",")]
        a = [x for x in a if x != args[args.index("--drop") + 1]]
    return a, opt, drop


WHYS = {}
STOPS = collections.Counter()


def replay(
    g,
    mem,
    shift,
    entries,
    cap=120,
    nest=2,
    k=120,
    follow=False,
    inline=True,
    tree=False,
    track=False,
    callthru=False,
):
    callwhy = collections.Counter()
    cache = {}
    blocks = collections.Counter()
    ends = collections.Counter()
    total = 0
    nblocks = 0
    sizes = collections.Counter()
    i = 0
    n = len(entries)
    while i < n:
        pc = entries[i][0] - shift
        if not 0 <= pc - 0 < (1 << 32) - 64 or not (mem.base - 64 <= pc < mem.base + len(mem.image)):
            i += 1
            continue
        if pc not in cache:
            items, top = blockasm.form(g, mem, pc, cap, inline, k, nest, follow, tree, track, callthru)
            S = set()
            T = {}
            for it in items:
                if it.ins is not None and it.kind != "goto_pc":
                    S.add(it.ins.pc)
                if it.kind in ("call", "rts", "jmp"):
                    T[it.ins.pc] = it.kind
                if it.kind == "call":
                    WHYS[it.ins.pc] = getattr(it, "why", None)
            cache[pc] = (S, T, len(S))
        S, T, size = cache[pc]
        if pc not in S:
            i += 1
            continue  # an instruction the assembler does not translate: the interpreter's
        cnt = 0
        why = "left the block"
        last = None
        while i < n:
            p = entries[i][0] - shift
            if p not in S:
                break
            cnt += 1
            i += 1
            last = p
            if p in T:
                why = T[p]
                if why == "call":
                    callwhy[WHYS.get(p)] += 1
                break
        if why == "left the block" and last is not None:
            li = blockasm.decode(mem, last)
            nxt = entries[i][0] - shift if i < n else None
            if li.kind == "plain" and nxt is not None:
                nd = blockasm.decode(mem, nxt)
                t = a7.text(mem, nxt)
                STOPS[
                    "%s%s"
                    % (
                        (t[0] if t else "?"),
                        "" if nd.kind == "stop" else " (translated, but not reached by the block)",
                    )
                ] += 1
            why = "left: " + (
                (
                    "%s %s" % (li.kind, "taken" if nxt != last + li.size else "not taken")
                    + (" backward" if nxt is not None and nxt <= last else "")
                )
                if li.kind in ("bcc", "dbcc")
                else li.kind
            )
        total += cnt
        nblocks += 1
        ends[why] += 1
        sizes[min(cnt, 30)] += 1
        blocks[pc] += 1
    union = set()
    for S_, T_, sz in cache.values():
        union |= S_
    dup = sum(sz for S_, T_, sz in cache.values()) / max(1, len(union))
    replay.callwhy = callwhy
    return nblocks, total, ends, len(cache), (dup, len(union))


def main():
    args = sys.argv[1:]
    be = "--be" in args
    a, opt, drop = load(args)
    trace = open(a[0], "rb").read()
    image = open(a[1], "rb").read()
    base = int(a[2], 0)
    shift = 0
    mem = blockasm.ImageMem(base, image)
    entries = [struct.unpack_from(">II" if be else "<II", trace, o) for o in range(0, len(trace) - 7, 8)]
    if drop:
        entries = [e for e in entries if not drop[0] <= e[0] <= drop[1]]
    g = Gen(read_offsets())
    nb, total, ends, ncache, (dup, nuniq) = replay(
        g,
        mem,
        shift,
        entries,
        cap=opt.get("cap", 120),
        nest=opt.get("nest", 2),
        k=opt.get("k", 120),
        follow="--follow" in args,
        inline="--noinline" not in args,
        tree="--tree" in args,
        track="--track" in args,
        callthru="all" if "--callthru-all" in args else "--callthru" in args,
    )
    label = " ".join(
        x
        for i_, x in enumerate(args)
        if (x.startswith("--") and x not in ("--be",)) or (i_ and args[i_ - 1] in ("--cap", "--nest", "--k"))
    )
    print(
        "%-26s %7d entries, %5.2f instr/entry, %d blocks, code x%.2f (%d distinct instrs); ends: %s"
        % (
            label or "baseline",
            nb,
            total / nb,
            ncache,
            dup,
            nuniq,
            ", ".join("%s %.0f%%" % (k, 100 * v / nb) for k, v in ends.most_common()),
        )
    )
    if "--why" in args:
        tc = sum(replay.callwhy.values())
        print("  calls that ended a block (%d), by why they were not inlined:" % tc)
        for k, v in replay.callwhy.most_common():
            print("     %5.1f%%  %s" % (100 * v / tc, k))
    if "--stops" in args:
        t = sum(STOPS.values())
        print("  the instruction a block stopped in front of (after a plain instruction), %d times:" % t)
        for k, v in STOPS.most_common(10):
            print("     %5.1f%%  %s" % (100 * v / t, k))


main()
