#!/usr/bin/env python3
"""Instruction pair statistics: which two consecutive 68K instructions are the most frequent, per workload and
averaged over the workloads (each trace normalised to 100%
so the long ones do not dominate), with register numbers and immediates folded away (`move.l D,(A)+`). A pair
is marked DEP when the second one uses the register the first
one wrote (its last operand): those are the candidates for fusion or forwarding.
  .venv/bin/python tools/study/pairstat.py [N]        (the traces are named below, all under local/traces/ or $TRACES: Dhrystone and the BASIC interpreter from tests/qemu/smoke_dhrystone_trace.c and trace_tinybasic.c, LhA from tools/study/trace_lha.sh)
"""

import sys, os, re, collections

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("../gen", "..")]
import tiersim as ts
import maketables as mt

N = int(sys.argv[1]) if len(sys.argv) > 1 else 25
T = (
    os.environ.get(
        "TRACES", os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "local", "traces")
    )
    + "/"
)
S = T + "lhatest/"
TRACES = (
    (
        "Dhrystone",
        T + "trace_dhrystone_b.bin",
        True,
        (0x1296, 0x12A8),
    ),  # strcpy of the Dhrystone image (DHRYSTONE_STRCPY in tests/data/dhrystone_image.h)
    ("BASIC interpreter", T + "trace_tinybasic.bin", False, None),
    ("LhA compress", S + "lha_a.trace", True, None),
    ("LhA decompress", S + "x/lha_x.trace", True, None),
)
_cls = {}


def klass(op):
    """-> (class text, registers read or written as a set of 'D0'.. 'A7', destination register or None)"""
    if op in _cls:
        return _cls[op]
    t = mt.m68k_text(op)
    regs = re.findall(r"\b([da][0-7])\b", t)
    mn, _, rest = t.partition(" ")
    ops = [x.strip() for x in re.split(r",(?![^()]*\))", rest)] if rest else []
    dst = None
    if ops:
        m = re.fullmatch(r"([da][0-7])", ops[-1])
        if m:
            dst = m.group(1).upper()
    c = re.sub(r"\b[da][0-7]\b", lambda m: "D" if m.group(0)[0] == "d" else "A", t)
    c = re.sub(r"#\$?[0-9a-fA-F-]+", "#imm", c)
    c = re.sub(r"(?<![A-Za-z])-?\$?[0-9a-fA-F]+(?=\()", "d", c)
    c = re.sub(r"\$[0-9a-fA-F]+", "abs", c)
    c = re.sub(r"\s+", " ", c).strip()
    _cls[op] = (c, frozenset(r.upper() for r in regs), dst)
    return _cls[op]


def dependent(a, b):
    """the second uses the register the first wrote -- by register number (an op's registers come from its own
    opcode, so the numbers are the real ones)
    """
    return a[2] is not None and a[2] in b[1]


def main():
    per = {}
    for name, path, be, drop in TRACES:
        if not os.path.exists(path):
            continue
        ents = ts.load(path, be, drop and (drop[0], drop[1]))
        ops = [o for _, o in ents]
        n = len(ops) - 1
        cnt = collections.Counter()
        dep = collections.Counter()
        for i in range(n):
            a, b = klass(ops[i]), klass(ops[i + 1])
            k = (a[0], b[0])
            cnt[k] += 1
            if dependent(a, b):
                dep[k] += 1
        per[name] = (cnt, dep, n)
        print("\n== %s: %d pairs" % (name, n))
        for k, c in cnt.most_common(N // 2 if N > 12 else N):
            print(
                "  %5.2f%%  %-34s -> %-34s %s"
                % (100 * c / n, k[0], k[1], "DEP %d%%" % (100 * dep[k] / c) if dep[k] else "")
            )
    avg = collections.Counter()
    avgd = collections.Counter()
    for name, (cnt, dep, n) in per.items():
        for k, c in cnt.items():
            avg[k] += c / n / len(per)
        for k, c in dep.items():
            avgd[k] += c / n / len(per)
    print("\n== averaged over the workloads (each trace = 100%)")
    for k, v in avg.most_common(N):
        print(
            "  %5.2f%%  %-34s -> %-34s %s"
            % (100 * v, k[0], k[1], "DEP %d%%" % (100 * avgd[k] / v) if avgd[k] else "")
        )


if __name__ == "__main__":
    main()
