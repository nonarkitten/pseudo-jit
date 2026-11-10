#!/usr/bin/env python3
"""Picks the hottest straight-line segments (runs of consecutive plain instructions of a translated block, no
branch / call / svc inside) of real traces, weighted by how
often the trace executes them, and writes them as data for tests/bbb/bbb_blocks.c (the board times each as a
loop: cycles per iteration) plus a JSON of the same words, so
that tools/study/cyclemodel.py's pipeline model can be fitted to the hardware on code that is actually
generated.

  .venv/bin/python tools/study/blockbench.py out_prefix  trace image base [--be] [--drop lo,hi] [--limit N]
  [trace image base ...]
"""

import sys, os, struct, json, collections

sys.path.insert(0, os.path.dirname(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("../gen", "..")]
import blockrun as br, blockasm as ba
from gen import Gen, read_offsets

import armdce


def writes_a_unsafely(words):
    """Does a word put an arbitrary value in an address register (r6-r12)? (A post-increment, a base
    write-back and a pop of the stack, which the harness fills with valid pointers, are fine.)
    """
    for w in words:
        a = armdce.analyse(w)
        wa = a.writes & 0x1FC0
        if not wa:
            continue
        k = w >> 25 & 7
        opc = w >> 21 & 15
        if k == 1 and opc in (2, 4) and (w >> 12 & 15) == (w >> 16 & 15):
            continue  # add / sub rX, rX, #imm
        if k in (2, 3) and w >> 20 & 1 and (w >> 16 & 15) in (13, (w >> 12 & 15)) and not (w >> 24 & 1):
            continue  # a load with post-index write-back
        if k in (2, 3) and w >> 20 & 1 and (w >> 16 & 15) == 13 and (w >> 12 & 15) in range(6, 13):
            continue  # ldr rA, [sp], #4 (a pop)
        if k == 0 and (w & 0x90) == 0x90 and w >> 20 & 1 and not (w >> 24 & 1):
            continue  # ldrsb / ldrh ... post-indexed
        if k == 0 and (w & 0x90) == 0x90 and w >> 21 & 1 and w >> 24 & 1:
            continue  # pre-indexed with write-back
        return True
    return False


import capstone

_md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_ARM)
_md.detail = True


def sp_drift(words):
    """Net bytes the segment moves sp by (pushes and pops with write-back, add / sub sp)."""
    d = 0
    for w in words:
        ci = next(_md.disasm(int(w).to_bytes(4, "little"), 0), None)
        if ci is None:
            continue
        for op in ci.operands:
            if op.type == capstone.arm.ARM_OP_MEM and op.mem.base == capstone.arm.ARM_REG_SP and ci.writeback:
                d += op.mem.disp if op.mem.disp else 0
        if (
            ci.mnemonic in ("add", "sub")
            and len(ci.operands) == 3
            and ci.operands[0].type == capstone.arm.ARM_OP_REG
            and ci.operands[0].reg == capstone.arm.ARM_REG_SP
            and ci.operands[2].type == capstone.arm.ARM_OP_IMM
        ):
            d += ci.operands[2].imm if ci.mnemonic == "add" else -ci.operands[2].imm
        if (
            ci.mnemonic in ("str", "ldr")
            and ci.writeback
            and not any(o.type == capstone.arm.ARM_OP_MEM and o.mem.disp for o in ci.operands)
        ):
            pass
    return d


def high_absolute(words):
    # a movt of a high address (Dhrystone's data): not RAM on the board
    return any((w & 0x0FF00000) == 0x03400000 and ((w >> 4 & 0xF000) | (w & 0xFFF)) >= 0x4000 for w in words)


def segments(rt, freq, min_items=2, max_words=70):
    out = {}
    for b in rt.blocks.values():
        run = []
        for it in b.items + [None]:
            if it is not None and it.kind == br.PLAIN:
                run.append(it)
                continue
            if len(run) >= 1:
                # cut the run into windows of items; each window is a candidate
                for s in range(len(run)):
                    for e in range(s + min_items, len(run) + 1):
                        its = run[s:e]
                        words = [w for x in its for w in x.words]
                        if len(words) > max_words:
                            break
                        if any(isinstance(w, tuple) for w in words):
                            continue
                        if any(
                            (w >> 24 & 15) == 15 or w >> 24 in (0xEB, 0xEC) or (w >> 25 & 7) == 5
                            for w in words
                        ):
                            continue
                        if any(x.trap for x in its if hasattr(x, "trap") and x.trap):
                            continue
                        if writes_a_unsafely(words) or high_absolute(words) or sp_drift(words) != 0:
                            continue
                        key = tuple(words)
                        wgt = freq.get(its[0].pc, 0)
                        if key not in out or out[key][0] < wgt:
                            out[key] = (wgt, [hex(x.pc) for x in its])
            run = []
    return out


def main():
    a = sys.argv[1:]
    prefix = a[0]
    rest = a[1:]
    opts = {}
    like = None
    for x in list(rest):
        if x.startswith("--opts="):
            opts = {k: True for k in x[7:].split(",")}
            rest.remove(x)
        elif x.startswith("--like="):
            like = json.load(open(x[7:]))
            rest.remove(x)
    g = Gen(read_offsets())
    allseg = {}
    while rest:
        trace, image, base = rest[0], rest[1], int(rest[2], 0)
        rest = rest[3:]
        flags = []
        while rest and rest[0].startswith("--"):
            flags.append(rest[0])
            rest = rest[1:]
            if flags[-1] in ("--drop", "--limit"):
                flags.append(rest[0])
                rest = rest[1:]
        be = "--be" in flags
        data = open(trace, "rb").read()
        mem = ba.ImageMem(base, open(image, "rb").read())
        entries = [struct.unpack_from(">II" if be else "<II", data, o) for o in range(0, len(data) - 7, 8)]
        if "--drop" in flags:
            d = [int(x, 0) for x in flags[flags.index("--drop") + 1].split(",")]
            entries = [e for e in entries if not d[0] <= e[0] <= d[1]]
        if "--limit" in flags:
            entries = entries[: int(flags[flags.index("--limit") + 1])]
        freq = collections.Counter(pc for pc, op in entries)
        rt = br.Runtime(mem, g, dict(opts))
        br.replay(rt, entries, 0, dict(opts))
        for k, v in segments(rt, freq).items():
            if k not in allseg or allseg[k][0] < v[0]:
                allseg[k] = (v[0], v[1], os.path.basename(trace))
    if like is not None:  # the same windows (by their 68K addresses) as an earlier run, with these options
        bypcs = {tuple(v[1]): (k, v) for k, v in allseg.items()}
        chosen = [bypcs[tuple(sg["pcs"])] for sg in like if tuple(sg["pcs"]) in bypcs]
        assert len(chosen) == len(like), (len(chosen), len(like))
        ranked = []
    else:
        ranked = sorted(allseg.items(), key=lambda kv: -kv[1][0] * len(kv[0]))
    # keep the hottest, but not 60 windows of the same run: skip a candidate whose words are a sub-sequence of
    # a chosen one
    if like is None:
        chosen = []
    for k, v in ranked:
        if any(
            len(k) <= len(c[0]) and any(c[0][i : i + len(k)] == k for i in range(len(c[0]) - len(k) + 1))
            for c in chosen
        ):
            continue
        chosen.append((k, v))
        if len(chosen) >= 40:
            break
    with open(prefix + ".json", "w") as f:
        json.dump([{"words": list(k), "weight": v[0], "pcs": v[1], "src": v[2]} for k, v in chosen], f)
    with open(prefix + ".h", "w") as f:
        f.write(
            "// generated by tools/study/blockbench.py: hot straight-line segments of generated blocks\n#include <stdint.h>\n"
        )
        for n, (k, v) in enumerate(chosen):
            f.write("static const uint32_t seg%d[] = { %s };\n" % (n, ", ".join("0x%08X" % w for w in k)))
        f.write(
            "static const struct { const uint32_t *w; unsigned n; } segs[] = {\n"
            + "".join("    { seg%d, %d },\n" % (n, len(k)) for n, (k, v) in enumerate(chosen))
            + "};\n#define NSEGS %d\n" % len(chosen)
        )
    print(
        "%d segments; words %d..%d"
        % (len(chosen), min(len(k) for k, v in chosen), max(len(k) for k, v in chosen))
    )


main()
