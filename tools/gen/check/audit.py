#!/usr/bin/env python3
"""Audit of the generator's sequences: how many ARM words each 68K instruction costs and where the cruft is.
  .venv/bin/python tools/gen/check/audit.py [--alarm N] [--family emit_ADDXL] [--hist file ...]
Reports: the length distribution (cached path: leader + body, `bx lr` excluded), per emitter family the min /
max / mean length and the static (Kickstart 3.1) and dynamic
(Dhrystone, Tiny BASIC) weight, the families over the alarm length (default 12) that are not known warts, who
touches the CPSR (mrs / msr / bfi / ubfx ...), and what the
leader (the first word, the one the dispatch table runs itself) is."""

import sys, os, re, collections

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("..", "../..")]
from gen import Gen, read_offsets

args = sys.argv[1:]
alarm = int(args[args.index("--alarm") + 1]) if "--alarm" in args else 12
only = args[args.index("--family") + 1] if "--family" in args else None
WARTS = {
    "emit_ABCD",
    "emit_SBCD",
    "emit_NBCD",
    "emit_DIVU",
    "emit_DIVS",
    "emit_DIVUW",
    "emit_DIVSW",
    "emit_MULU",
    "emit_MULS",
    "emit_MOVEM",
    "emit_MOVEP",
    "emit_CHK",
}


def load_hist(path):
    h = collections.Counter()
    for l in open(path):
        if l.startswith("#") or not l.strip():
            continue
        p = l.split()
        try:
            h[int(p[0], 16)] += int(p[1])
        except ValueError:
            pass
    return h


H = {
    "kick": load_hist("tools/data/profiles/kickstart31.static.hist"),
    "dhry": load_hist("tools/data/profiles/dhrystone.hist"),
    "tb": load_hist("tools/data/profiles/tinybasic_sieve.hist"),
}
g = Gen(read_offsets())


def mn(w):
    if isinstance(w, tuple):
        return w[0] + "*"
    t = getattr(w, "text", None)
    return t.split()[0].rstrip("0123456789") if t else "?"


rows = []
for op in range(65536):
    name = g.winner[op] or "emit_ILLEGAL"
    if name in ("emit_ILLEGAL",):
        continue
    seq = g.generate(op)
    if seq is None:
        continue
    body = [w for w in seq if not (not isinstance(w, tuple) and getattr(w, "text", "") == "bx\tlr")]
    rows.append((op, name, len(body), [mn(w) for w in body], mn(seq[0])))
print("%d valid opcodes" % len(rows))
dist = collections.Counter(min(r[2], 40) for r in rows)
print("length of leader + body (words):", " ".join("%d:%d" % (k, dist[k]) for k in sorted(dist)))
fam = collections.defaultdict(list)
for r in rows:
    fam[r[1]].append(r)


def weight(rs, k):
    return sum(H[k][r[0]] for r in rs)


tot = {k: sum(H[k].values()) for k in H}
print(
    "\n%-22s %5s %4s %4s %5s | %8s %8s %8s | %s"
    % ("family", "ops", "min", "max", "mean", "kick%", "dhry%", "tb%", "notes")
)
out = []
for name, rs in fam.items():
    ls = [r[2] for r in rs]
    mnem = collections.Counter(m for r in rs for m in r[3])
    cp = sum(mnem[m] for m in ("mrs", "msr", "bfi", "bfc", "ubfx", "sbfx"))
    out.append((weight(rs, "kick") * sum(ls) / len(ls), name, rs, ls, cp))
for _, name, rs, ls, cp in sorted(out, key=lambda x: -x[0])[: 45 if not only else 999]:
    if only and name != only:
        continue
    note = (
        ("CPSR/bitfield words: %d" % cp if cp else "")
        + (" WART" if name in WARTS else "")
        + (" ALARM" if max(ls) > alarm and name not in WARTS else "")
    )
    print(
        "%-22s %5d %4d %4d %5.1f | %7.2f%% %7.2f%% %7.2f%% | %s"
        % (
            name,
            len(rs),
            min(ls),
            max(ls),
            sum(ls) / len(ls),
            100 * weight(rs, "kick") / tot["kick"],
            100 * weight(rs, "dhry") / tot["dhry"],
            100 * weight(rs, "tb") / tot["tb"],
            note,
        )
    )
print("\nfamilies over %d words that are not known warts:" % alarm)
for _, name, rs, ls, cp in sorted(out, key=lambda x: -max(x[3])):
    if max(ls) > alarm and name not in WARTS:
        print("  %-22s max %3d mean %5.1f (%d opcodes)" % (name, max(ls), sum(ls) / len(ls), len(rs)))
print("\nwords that touch the CPSR as a value (mrs / msr / bfi / ubfx ...), opcodes affected:")
c = collections.Counter()
ops = collections.defaultdict(set)
for op, name, n, ms, lead in rows:
    for m in set(ms):
        if m in ("mrs", "msr", "bfi", "bfc", "ubfx", "sbfx"):
            ops[m].add(op)
for m in sorted(ops):
    print(
        "  %-5s in %5d opcodes (%.1f%% of Kickstart, %.1f%% of Dhrystone)"
        % (
            m,
            len(ops[m]),
            100 * sum(H["kick"][o] for o in ops[m]) / tot["kick"],
            100 * sum(H["dhry"][o] for o in ops[m]) / tot["dhry"],
        )
    )
lead = collections.Counter(r[4] for r in rows)
print(
    "\nthe leader (first word) of each opcode, by mnemonic:",
    ", ".join("%s %d" % kv for kv in lead.most_common(12)),
)
print(
    "mean words per instruction, weighted: Kickstart %.2f  Dhrystone %.2f  Tiny BASIC %.2f"
    % tuple(
        sum(H[k][r[0]] * r[2] for r in rows) / sum(H[k][r[0]] for r in rows) for k in ("kick", "dhry", "tb")
    )
)

# ---- semantic routines used by one opcode only ----
# A body (everything after the leader) that exactly one opcode uses is code that should have been shared: it
# names a register or an address mode. In the leader + body layout the unique parts live in the leader slots and the body is a routine per
# (operation, size), so this list is the remaining wart families.
if "--no-bodies" not in args:
    import maketables as mt

    ot = mt.build(g)[1]
    units = [o >> 2 for o in ot]
    ref = collections.Counter(units)
    byfam = collections.defaultdict(list)
    for op in range(65536):
        byfam[g.winner[op] or "emit_ILLEGAL"].append(op)
    priv = []
    for name, ops in byfam.items():
        if name == "emit_ILLEGAL":
            continue
        n = sum(1 for o in ops if ref[units[o]] == 1)
        if n:
            priv.append((n, name, len(ops), len({units[o] for o in ops})))
    print(
        "\nWARNING: bodies used by exactly one opcode (%d opcodes of %d have a private body; %d distinct bodies in all):"
        % (sum(p[0] for p in priv), len(ot), len(ref))
    )
    for n, name, k, d in sorted(priv, reverse=True)[:40]:
        print(
            "  %-18s %5d opcodes, %4d distinct bodies, %4d private%s"
            % (name, k, d, n, "  WART" if name in WARTS else "")
        )
