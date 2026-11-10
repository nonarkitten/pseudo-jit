"""Instruction scheduling over a straight-line region of ARM words (the words of several consecutive 68K
instructions of one block), to hide the latencies the
in-order, dual-issue Cortex-A8 pays: load-to-use 3 cycles, a shifted operand 2, the three-step carry fix-ups
the generator uses.

Every sequence uses r0-r3 as scratch, so two 68K instructions cannot be interleaved as they stand. Here the
values in r0-r3 are renamed freely (each write is a new
value; the physical registers are not assigned), which gives the upper bound of what scheduling can do with
enough registers; what registers it actually takes is the
next question. Dependences kept: register read-after-write for the renamed registers; read-after-write,
write-after-read and write-after-write for every other
register (the A registers, r4...) and for the flags N Z C V G; and order between memory accesses of the same
kind: D register slots (per D register), guest memory
(a store orders against every other access), other cpu state.

  schedule(words) -> (the words in the new order, estimated cycles before, after)   [needs tools/study/cyclemodel.py for the timing]
"""

import os, sys, copy

sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path.insert(0, os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "gen"))
import armdce, fwd, cyclemodel

cyclemodel.MISPREDICT = 0
RENAMED = (0, 1, 2, 3)
FL = {"N": 100, "Z": 101, "C": 102, "V": 103, "G": 104}


class Node:
    __slots__ = ("idx", "word", "tmpl", "reads", "writes", "mem", "preds", "succs", "prio", "lat")


def bits(mask):
    return [r for r in range(16) if mask >> r & 1]


def flag_ids(m):
    return [FL[n] for n, b in (("N", 1), ("Z", 2), ("C", 4), ("V", 8), ("G", 16)) if m & b]


def build_nodes(words, d0):
    nodes = []
    ver = {r: 0 for r in RENAMED}
    lastw = {}
    lastr = {}
    memw = {}
    memr = {}
    for idx, w in enumerate(words):
        n = Node()
        n.idx = idx
        n.word = int(w)
        n.tmpl = cyclemodel.decode(0, n.word)
        info = armdce.analyse(n.word)
        n.preds = {}
        n.succs = []
        n.lat = n.tmpl.lat
        rd = bits(info.reads)
        wr = bits(info.writes)
        cond = n.word >> 28 != 14
        if cond:
            rd = sorted(set(rd) | set(wr))  # a conditional write keeps the old value when it does not happen
        fr = flag_ids(info.freads)
        fw = flag_ids(info.fkill | info.fmay)
        if info.fmay:
            fr = sorted(set(fr) | set(fw))  # a maybe-write keeps the old flag
        if cond:
            fr = sorted(set(fr) | set(fw))
        n.reads = []
        n.writes = []
        for r in rd:
            n.reads.append(("r%d.%d" % (r, ver[r])) if r in RENAMED else "r%d" % r)
        for r in wr:
            if r in RENAMED:
                ver[r] += 1
                n.writes.append("r%d.%d" % (r, ver[r]))
            else:
                n.writes.append("r%d" % r)
        n.reads += ["f%d" % f for f in fr]
        n.writes += ["f%d" % f for f in fw]
        # memory
        s = fwd.slot(n.word, d0)
        load = n.tmpl.kind in ("mem", "multi") and (n.tmpl.name.startswith(("ldr", "ldm", "pop")))
        store = n.tmpl.kind in ("mem", "multi") and not load
        if n.tmpl.kind in ("mem", "multi"):
            cls = (
                ("D", s[2])
                if s is not None
                else (("S", (n.word >> 16 & 15) == 5) if (n.word >> 16 & 15) == 5 else ("G",))
            )
            n.mem = (cls, load, store)
        else:
            n.mem = None
        nodes.append(n)

    # dependences
    def edge(a, b, lat):
        if a is b:
            return
        if b.idx not in a.succs and True:
            pass
        if a.idx in b.preds:
            b.preds[a.idx] = max(b.preds[a.idx], lat)
        else:
            b.preds[a.idx] = lat
        a.succs.append(b.idx)

    writer = {}
    readers = collections_dd()
    for n in nodes:
        for r in n.reads:
            if r in writer:
                edge(nodes[writer[r]], n, nodes[writer[r]].lat)  # RAW
        if n.mem:
            cls, load, store = n.mem
            for key in [cls] + ([("G",)] if False else []):
                pass
        for r in n.writes:
            renamed = "." in r
            if not renamed:
                for rd in readers[r]:
                    edge(nodes[rd], n, 0)  # WAR
                if r in writer:
                    edge(nodes[writer[r]], n, 1)  # WAW
        for r in n.reads:
            if "." not in r:
                readers[r].append(n.idx)
        for r in n.writes:
            writer[r] = n.idx
            if "." not in r:
                readers[r] = []
    # memory ordering
    lastst = {}
    lastld = collections_dd()
    for n in nodes:
        if not n.mem:
            continue
        cls, load, store = n.mem
        keys = [cls]
        for k in keys:
            if store:
                if k in lastst:
                    edge(nodes[lastst[k]], n, 1)
                for l in lastld[k]:
                    edge(nodes[l], n, 0)
                lastst[k] = n.idx
                lastld[k] = []
            else:
                if k in lastst:
                    edge(nodes[lastst[k]], n, 1)
                lastld[k].append(n.idx)
    return nodes


class collections_dd(dict):
    def __missing__(self, k):
        self[k] = []
        return self[k]


def timed_copy(n, renamed):
    i = copy.copy(n.tmpl)
    if renamed:
        i.reads = tuple(n.reads_names)
        i.writes = tuple(n.writes_names)
    return i


def evaluate(order, nodes, renamed):
    """Cycles for the nodes issued in this order, by the cycle model (registers by name: renamed values or the
    physical ones).
    """
    seq = []
    for k, idx in enumerate(order):
        n = nodes[idx]
        i = copy.copy(n.tmpl)
        if renamed:
            i.reads = tuple(x for x in n.reads if not x.startswith("f"))
            i.writes = tuple(x for x in n.writes if not x.startswith("f"))
        key = (0x90000000 + 4 * k, 0x10000000 + n.idx * 64 + k)
        cyclemodel._cache[key] = i
        seq.append(key)
    return cyclemodel.Sim().run(seq)


def schedule(words, d0):
    nodes = build_nodes(words, d0)
    n = len(nodes)
    # priority: longest latency-weighted path to the end
    prio = [0] * n
    for k in range(n - 1, -1, -1):
        p = nodes[k].lat
        for s in nodes[k].succs:
            p = max(p, nodes[k].lat + prio[s])
        prio[k] = p
    issued = {}
    order = []
    cycle = 0
    remaining = set(range(n))
    while remaining:
        group = []
        ls_used = False
        writes_in_group = set()
        flag_written = False
        while len(group) < 2:
            best = None
            for k in sorted(remaining, key=lambda x: (-prio[x], x)):
                nd = nodes[k]
                if any(p not in issued for p in nd.preds):
                    continue
                if any(issued[p] + lat > cycle for p, lat in nd.preds.items() if lat > 0):
                    continue
                if any(p in [g for g in group] and lat > 0 for p, lat in nd.preds.items()):
                    continue
                if nd.mem and nd.tmpl.kind == "mem" and ls_used:
                    continue
                best = k
                break
            if best is None:
                break
            group.append(best)
            nd = nodes[best]
            if nd.tmpl.kind == "mem":
                ls_used = True
            issued[best] = cycle
            remaining.discard(best)
            order.append(best)
        cycle += 1
        if cycle > 4 * n + 20:
            raise RuntimeError("scheduler stuck")
    before = evaluate(list(range(n)), nodes, False)
    after = evaluate(order, nodes, True)
    global last_order
    last_order = list(order)
    return [words[i] for i in order], before, after
