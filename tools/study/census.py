#!/usr/bin/env python3
"""How many words of each opcode's sequence are
unique to the opcode and how many are common to the
opcodes of its cluster (same emitter, same size / direction / EA modes, any register numbers). The common
suffix of a cluster is what a shared routine could be;
what precedes it is what the three leader slots would have to hold.   tools/study/census.py [--family NAME] [--over N]
"""

import sys, os, re, collections
import capstone

md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_ARM)
sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("../gen", "..")]
from gen import Gen, read_offsets

BXLR = 0xE12FFF1E
args = sys.argv[1:]
fam = args[args.index("--family") + 1] if "--family" in args else None
over = int(args[args.index("--over") + 1]) if "--over" in args else 3
g = Gen(read_offsets())
KEEP_COND = ("emit_Bcc", "emit_DBcc", "emit_Scc")
clusters = collections.defaultdict(list)
for op in range(65536):
    name = g.winner[op]
    if not name or name == "emit_ILLEGAL":
        continue
    seq = g.generate(op)
    if fam and name != fam:
        continue
    body = [w if isinstance(w, tuple) else int(w) for w in seq]
    if body and body[-1] == BXLR:
        body = body[:-1]
    key = (name, op >> 3 & 7, op >> 6 & 7, (op >> 8 & 15) if name in KEEP_COND else 0)
    clusters[key].append((op, tuple(map(repr, body))))


def text(w):
    if isinstance(w, tuple):
        return f"{w[0]} {w[2]}"
    i = next(md.disasm(int(w).to_bytes(4, "little"), 0), None)
    return f"{i.mnemonic} {i.op_str}" if i else "?"


def tokens(t):
    out = set(re.findall(r"\[r5, #-?0x[0-9a-f]+\]", t)) | set(
        re.findall(r"\b(?:r(?:6|7|8|9|10|11|12)|sb|sl|fp|ip|sp)\b", t)
    )
    if not out and re.search(r"#-?(?:0x)?[0-9a-f]+", t):
        out.add("imm")
    return out


rows = []
for key, ops in clusters.items():
    ln = collections.Counter(len(b) for _, b in ops).most_common(1)[0][0]
    same = [(op, b) for op, b in ops if len(b) == ln]
    diff = [k for k in range(ln) if len({b[k] for _, b in same}) > 1]
    for op, b in same:
        tk = set()
        for k in diff:
            tk |= tokens(
                text(eval(b[k])) if False else text(eval(b[k]) if b[k].startswith("(") else int(b[k]))
            )
        rows.append((key[0], op, ln, max(len(tk), len(diff) and 1), len(diff)))
    for op, b in ops:
        if len(b) != ln:
            rows.append((key[0], op, len(b), len(b), 0))
hist = collections.Counter(u for _, _, _, u, _ in rows)
tot = len(rows)
print(f"{tot} opcodes in {len(clusters)} clusters")
print(
    "operand tokens in the words that differ within the cluster (an estimate of the unique words: one address word per operand): "
    + "  ".join(f"{u}:{hist[u]}" for u in sorted(hist))
)
fit = sum(c for u, c in hist.items() if u <= over)
dh = collections.Counter(d for *_, d in rows)
print("words that differ within the cluster (positional): " + "  ".join(f"{u}:{dh[u]}" for u in sorted(dh)))
print(f"fit in {over} words: {fit} ({100 * fit / tot:.1f}%); need more: {tot - fit}")
by = collections.defaultdict(lambda: [0, 0, 0])
for name, op, L, u, suf in rows:
    b = by[name]
    b[0] += 1
    b[1] = max(b[1], u)
    b[2] += u > over
print(f"\n{'family':22} {'ops':>6} {'max uniq':>8} {'ops > ' + str(over):>8}")
for name, (n, mx, nb) in sorted(by.items(), key=lambda kv: -kv[1][2]):
    if nb:
        print(f"{name:22} {n:6} {mx:8} {nb:8}")

# ---- does the split form fit the span? worst-case prefix words (fill_emit_ea) + unique words + the `bl` against 4 words per guest halfword ----
import maketables as mt

# i16 movw+sxth, i32 movw+movt, d8,An,Xn mov + index (max 3), d16(PC) movw+movt, d8(PC,Xn) movw+movt + index,
# bdisp
PFX = {
    0: 0,
    1: 2,
    2: 2,
    3: 4,
    4: 2,
    5: 5,
    6: 2,
}
bad = collections.Counter()
badops = collections.defaultdict(list)
for name, op, L, u, _ in rows:
    e = mt.ext[op]
    pre = PFX[e >> 4 & 15] + PFX[e & 15]
    n = mt.ext_word_count(op)
    if pre + u + 1 > 4 * n:
        bad[name] += 1
        badops[name].append((op, pre, u, n))
print(f"\nopcodes whose worst-case prefix + unique words + bl exceed the span: {sum(bad.values())}")
for name, c in bad.most_common(12):
    print(
        f"  {name:20} {c:5}  e.g. {badops[name][0][0]:04X} prefix {badops[name][0][1]} unique {badops[name][0][2]} span {4 * badops[name][0][3]}"
    )
