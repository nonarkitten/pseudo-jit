"""Opcode frequencies for the size-vs-speed decisions of the DRSR abstraction (tools/gen/abstract.py): every
profile in tools/data/profiles/*.hist is normalised to a
probability per opcode (the dynamic ones: share of executed instructions; kickstart31.static: share of the
instructions in the ROM) and the opcode's weight is their mean,
so no single program decides.   profiles() -> {name: {op: share}};  combined() -> {op: mean share}."""

import os, glob

HERE = os.path.dirname(os.path.abspath(__file__))


def load(path):
    h = {}
    for l in open(path):
        if l.startswith("#") or not l.strip():
            continue
        p = l.split()
        try:
            h[int(p[0], 16)] = h.get(int(p[0], 16), 0) + int(p[1])
        except ValueError:
            pass
    t = sum(h.values())
    return {op: c / t for op, c in h.items()}


def profiles():
    files = sorted(glob.glob(os.path.join(HERE, "..", "data", "profiles", "*.hist")))
    if not files:
        raise SystemExit(
            "weights.py: no opcode profiles found in tools/data/profiles/ (the DRSR sharing budget needs them)"
        )
    return {os.path.basename(f).replace(".hist", ""): load(f) for f in files}


def combined(profs=None):
    profs = profs or profiles()
    out = {}
    for h in profs.values():
        for op, s in h.items():
            out[op] = out.get(op, 0) + s / len(profs)
    return out
