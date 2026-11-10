#!/usr/bin/env python3
"""Lists the hottest regions the asm scheduler reorders in a trace's blocks, before / after (disassembled).
usage: schedregions.py trace image base [--be] [--drop lo,hi] [--top N]
"""

import sys, os, struct, collections

sys.path.insert(0, os.path.dirname(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("../gen", "..")]
import blockrun as br, blockasm as ba, cyclemodel
from gen import Gen, read_offsets

a = sys.argv[1:]
be = "--be" in a
drop = [int(x, 0) for x in a[a.index("--drop") + 1].split(",")] if "--drop" in a else None
top = int(a[a.index("--top") + 1]) if "--top" in a else 12
pos = [
    x for i, x in enumerate(a) if not x.startswith("--") and (i == 0 or a[i - 1] not in ("--drop", "--top"))
]
data = open(pos[0], "rb").read()
mem = ba.ImageMem(int(pos[2], 0), open(pos[1], "rb").read())
entries = [struct.unpack_from(">II" if be else "<II", data, o) for o in range(0, len(data) - 7, 8)]
if drop:
    entries = [e for e in entries if not drop[0] <= e[0] <= drop[1]]
freq = collections.Counter(pc for pc, op in entries)
g = Gen(read_offsets())
opts = {"ofold": True, "sched": True, "inline_rts": True, "nolive": True}
rt = br.Runtime(mem, g, dict(opts))
br.replay(rt, entries, 0, opts)
runs = {}
for ws, res, pc in rt.sched_runs:
    runs[(tuple(ws), pc)] = (ws, res, pc)
rows = sorted(runs.values(), key=lambda r: -freq[r[2]] * len(r[0]))
n = sum(1 for r in rows if r[1] is not None)
print("%d regions, %d reordered" % (len(rows), n))
import capstone

md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_ARM)


def dis(w):
    i = next(md.disasm(int(w).to_bytes(4, "little"), 0), None)
    return ("%s %s" % (i.mnemonic, i.op_str)) if i else "%08x" % w


for ws, res, pc in rows[:top]:
    print(
        "--- pc %x weight %d, %d words %s"
        % (pc, freq[pc], len(ws), "" if res is not None else "(not scheduled)")
    )
    for k in range(len(ws)):
        print("  %-28s %s" % (dis(ws[k]), dis(res[k]) if res else ""))
