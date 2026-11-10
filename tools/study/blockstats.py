#!/usr/bin/env python3
"""What the block assembler makes of real code: every distinct block entry in a dynamic trace (the target of a
taken branch / call / return) is assembled from the
program image, and its size and D-register slot traffic are totalled weighted by how often the trace enters
it. Reports words per 68K instruction and D slot
accesses per 68K instruction for: every flag wanted, flag liveness, liveness + load forwarding. Blocks that
stop early (an untranslated instruction) count as far as they go.

  .venv/bin/python tools/study/blockstats.py trace.bin image.bin base [--be] [--cap 120]"""

import sys, os, struct, collections

sys.path.insert(0, os.path.dirname(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("../gen", "..")]
import cyclemodel, blocksched

cyclemodel.MISPREDICT = (
    0  # each block is scored once, in straight-line order: its branches are not 'mispredicted'
)
from gen import Gen, read_offsets
import blockasm, fwd


def main():
    a = [x for x in sys.argv[1:] if not x.startswith("--")]
    be = "--be" in sys.argv
    cap = int(sys.argv[sys.argv.index("--cap") + 1]) if "--cap" in sys.argv else 120
    if "--cap" in sys.argv:
        a = [x for x in a if x != str(cap)]
    trace = open(a[0], "rb").read()
    image = open(a[1], "rb").read()
    base = int(a[2], 0)
    shift = 0
    mem = blockasm.ImageMem(base, image)
    entries = [struct.unpack_from(">II" if be else "<II", trace, o) for o in range(0, len(trace) - 7, 8)]
    count = collections.Counter()
    prev = None
    for pc, op in entries:
        if prev is None or pc != prev:
            count[pc] += 1
        prev = pc + 2 * 0  # (the next sequential pc is only known by decoding: approximate below)
    # entry = a pc that does not follow its predecessor by a plausible instruction length
    count = collections.Counter()
    prev_pc = None
    for pc, op in entries:
        if prev_pc is None or not (0 < pc - prev_pc <= 10):
            count[pc] += 1
        prev_pc = pc
    offs = read_offsets()
    g = Gen(offs)
    tot = collections.Counter()
    built = 0
    sims = []
    pressure = collections.Counter()
    for pc, n in count.items():
        off = pc
        if not mem.base <= off < mem.base + len(mem.image):
            continue
        try:
            res = {}
            for tag, kw in (
                ("base", dict(optimise=False)),
                ("live", dict(optimise=True)),
                ("fwd", dict(optimise=True, forward=True)),
                ("fold", dict(optimise=True, forward=True, cfold=True)),
                ("alloc", dict(optimise=True, forward=True, cfold=True, alloc=True)),
            ):
                words, info = blockasm.build(g, mem, off, cap=cap, **kw)
                dm = sum(
                    1
                    for w in words[: info["code_words"]]
                    if not isinstance(w, tuple) and fwd.slot(w, g.D0) is not None
                )
                seq = [
                    (0x80000000 + 4 * k, int(w))
                    for k, w in enumerate(words[: info["code_words"]])
                    if not isinstance(w, tuple)
                ]
                sm = cyclemodel.Sim()
                cyc = sm.run(seq) if seq else 0
                if tag == "alloc":
                    for pk in info["peaks"]:
                        pressure[pk] += n
                if tag == "fold":
                    sims.append((n, sm))
                    if "--sched" in sys.argv:
                        for reg in info["regions"]:
                            if len(reg) < 3:
                                continue
                            try:
                                _, b0, a0 = blocksched.schedule(reg, g.D0)
                            except Exception as e:
                                tot["sched failed"] += 1
                                if tot["sched failed"] == 1:
                                    import traceback

                                    traceback.print_exc()
                                continue
                            tot["sched before"] += n * b0
                            tot["sched after"] += n * a0
                            tot["sched words"] += n * len(reg)
                res[tag] = (info["code_words"], dm, info["instructions"], cyc)
        except Exception as e:
            tot["failed"] += 1
            continue
        ni = res["base"][2]
        if not ni:
            continue
        built += 1
        tot["entries"] += n
        tot["instructions"] += n * ni
        for tag in res:
            tot["words " + tag] += n * res[tag][0]
            tot["D " + tag] += n * res[tag][1]
            tot["cyc " + tag] += n * res[tag][3]
    print(
        "%d distinct entries assembled (%d failed), %d weighted entries"
        % (built, tot["failed"], tot["entries"])
    )
    ni = tot["instructions"]
    print("  68K instructions per entry: %.2f" % (ni / tot["entries"]))
    for tag, what in (
        ("base", "every flag wanted"),
        ("live", "flag liveness"),
        ("fwd", "liveness + load forwarding"),
        ("fold", "... + constant folding"),
        ("alloc", "... + register allocation"),
    ):
        print(
            "  %-28s %.2f ARM words, %.2f D slot accesses, %.2f modelled cycles per 68K instruction"
            % (what, tot["words " + tag] / ni, tot["D " + tag] / ni, tot["cyc " + tag] / ni)
        )
    if "--stalls" in sys.argv:
        stall = collections.Counter()
        dep = collections.Counter()
        seen = set()
        for n, sm in sims:
            for kk, vv in sm.stall.items():
                stall[kk] += n * vv
            for kk, vv in sm.dep.items():
                dep[kk] += n * vv
        tc = tot["cyc fold"]
        print(
            "  stalls in the final blocks, share of their modelled cycles: "
            + ", ".join("%s %.1f%%" % (k, 100 * v / tc) for k, v in stall.most_common())
        )
        print(
            "  what the dependency stalls waited for (producer -> consumer): "
            + ", ".join("%s->%s %.1f%%" % (k[0], k[1], 100 * v / tc) for k, v in dep.most_common(8))
        )
    if "--sched" in sys.argv and tot["sched before"]:
        print(
            "  scheduling the straight-line regions (renaming r0-r3 freely: an upper bound): %.1f%% fewer modelled cycles in the regions (%.2f -> %.2f cycles per word), %d regions refused"
            % (
                100 * (1 - tot["sched after"] / tot["sched before"]),
                tot["sched before"] / tot["sched words"],
                tot["sched after"] / tot["sched words"],
                tot["sched failed"],
            )
        )
    if pressure:
        t = sum(pressure.values())
        print(
            "  registers needed by a straight-line region (peak live values; the pool is r0-r4 + lr = 6), weighted by entries: "
            + ", ".join(
                "%s: %.1f%%" % ("unallocatable" if k is None else k, 100 * v / t)
                for k, v in sorted(pressure.items(), key=lambda kv: (kv[0] is None, kv[0] or 0))
            )
        )


main()
