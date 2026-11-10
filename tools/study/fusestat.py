#!/usr/bin/env python3
"""How often could a compare-class instruction followed by a Bcc use the ARM flags in their natural polarity (no
68K-polarity fix-up)? That is allowed when the flags are dead
on BOTH successors of the branch: the instructions at the branch target and after it do not read N Z V C
(tools/gen/armdce.py's live-in groups for an empty live-out).
Per trace: dynamic compare+Bcc pairs, and the share whose successors (as seen in the trace) both kill the
flags.
  .venv/bin/python tools/study/fusestat.py trace.bin [--be]"""

import sys, os, collections

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("../gen", "..")]
import tiersim as ts
from gen import Gen, read_offsets
import armdce, blockasm as ba

g = Gen(read_offsets())
g.legacy = True
_reads = {}


def reads(op):
    """does the instruction read N Z V C (or is it a control transfer / something that is not a plain
    sequence)?
    """
    if op in _reads:
        return _reads[op]
    r = True
    try:
        k = ts.classify(op)
        if k == "seq":
            seq = g.generate(op)
            if seq is not None:
                v, li = armdce.dce(list(seq), 0)
                r = bool(li & 1)
    except Exception:
        r = True
    _reads[op] = r
    return r


def is_cmp(op):
    # CMP / CMPA / CMPM / EOR (0xB1xx!) ... see below
    return (op & 0xF000) == 0xB000 or (op & 0xFF00) == 0x0C00 or (op & 0xFFC0) in (0x4A00, 0x4A40, 0x4A80)


def kind(op):
    if (op & 0xF1C0) in (0xB000, 0xB040, 0xB080, 0xB0C0, 0xB1C0):
        return "CMP/CMPA"
    if (op & 0xF138) == 0xB108 and (op & 0xC0) != 0xC0:
        return "CMPM"
    if (op & 0xFF00) == 0x0C00:
        return "CMPI"
    if (op & 0xFF00) == 0x4A00 and (op & 0xC0) != 0xC0:
        return "TST"
    if (op & 0xF100) == 0x5100 and (op & 0xC0) != 0xC0:
        return "SUBQ"
    if (op & 0xF100) == 0x9100 or (op & 0xF000) == 0x9000:
        return "SUB"
    return None


def main():
    args = sys.argv[1:]
    be = "--be" in args
    path = [a for a in args if not a.startswith("--")][0]
    tr = ts.Trace(ts.load(path, be))
    n = tr.n
    succ_t = collections.defaultdict(set)
    succ_n = collections.defaultdict(set)
    for j in range(n - 1):
        if tr.kind[j] == "bcc":
            taken = tr.pc[j + 1] != tr.pc[j] + tr.size[j]
            (succ_t if taken else succ_n)[tr.pc[j]].add(tr.op[j + 1])
    tot = collections.Counter()
    fus = collections.Counter()
    sites = {}
    for j in range(n - 1):
        if tr.kind[j + 1] == "bcc" and tr.pc[j + 1] == tr.pc[j] + tr.size[j]:
            k = kind(tr.op[j])
            if k is None:
                continue
            tot[k] += 1
            b = tr.pc[j + 1]
            if b not in sites:
                st, sn = succ_t.get(b, set()), succ_n.get(b, set())
                sites[b] = bool(st) and bool(sn) and not any(reads(o) for o in st | sn)
            if sites[b]:
                fus[k] += 1
    print("%s: %d instructions" % (os.path.basename(path), n))
    for k in sorted(tot, key=lambda k: -tot[k]):
        print(
            "  %-9s followed by Bcc: %7d (%.2f%% of instructions), flags dead on both successors: %5.1f%%"
            % (k, tot[k], 100 * tot[k] / n, 100 * fus[k] / tot[k])
        )
    t = sum(tot.values())
    f = sum(fus.values())
    print(
        "  all: %d pairs = %.2f%% of instructions; fusable %.1f%%" % (t, 100 * t / n, 100 * f / t if t else 0)
    )


if __name__ == "__main__":
    main()
