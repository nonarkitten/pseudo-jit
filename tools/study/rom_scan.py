#!/usr/bin/env python3
"""Static opcode census of a 68K disassembly listing (an IRA disassembly of your own Kickstart 3.1 ROM, one
instruction per line with its
address and machine words after a `;`; Commodore's work, so it is not in the repository: keep it under local/). Writes a `XXXX count` histogram in the format
tools/study/profile_cost.py reads (static counts:
how often each opcode word occurs in the code, not how often it runs) and prints the call/return mix.

   python3 tools/study/rom_scan.py local/roms/kick31.s tools/data/profiles/kickstart31.static.hist"""

import re, sys, collections

hist = collections.Counter()
mn = collections.Counter()
jsr = collections.Counter()
total = 0
pat = re.compile(r"^\s+(\S+)\s+(.*?)\s*;([0-9a-f]+): ([0-9a-f]+)")
for l in open(sys.argv[1], errors="replace"):
    m = pat.match(l.rstrip("\n"))
    if not m:
        continue
    name, ops, addr, words = m.groups()
    if len(words) < 4 or len(words) % 4:
        continue
    hist[int(words[:4], 16)] += 1
    total += 1
    base = name.split(".")[0]
    mn[base] += 1
    if base in ("JSR", "JMP", "BSR"):
        form = re.sub(r"-?\$?[0-9a-fA-F]+\(", "d(", ops)
        form = re.sub(r"^lb_\w+|^\$?[0-9a-fA-F]+\.[WL]$", "abs", form)
        jsr[f"{name} {form}"] += 1
with open(sys.argv[2], "w") as f:
    f.write(
        f"# Static opcode counts of {sys.argv[1]} ({total} instructions, {len(hist)} distinct opcode words).\n"
    )
    for op, n in hist.most_common():
        f.write(f"{op:04X} {n}\n")
print(total, "instructions;", len(hist), "distinct opcode words")
for k, v in mn.most_common(25):
    print(f"  {k:8s} {v:7d} {100*v/total:5.1f}%")
print("call/jump forms:")
for k, v in jsr.most_common(14):
    print(f"  {k:24s} {v:6d}")
