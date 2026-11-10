#!/usr/bin/env python3
"""Trace-driven simulator of the tiering policy (docs/design/TIERING.md): which
code gets promoted from the slot cache to a block, and what does that
buy?  Everything is a deterministic function of the guest's control flow: no timers, no sampling.

The policy under test:
  * a small tagged table of taken-transfer TARGETS, direct-mapped by a hash of the address, each with a
  saturating byte count; a collision replaces the entry (count restarts);
  * when a target's count reaches the threshold T, a block is formed there and put in the block ARENA
  (separate from the slot cache, so a slot line being evicted does not lose it);
  * the arena is bounded (host bytes; a block costs 7 host bytes per source byte); when it is full, a
  replacement policy picks the victim (fifo / lru / lfu);
  * a promoted block runs from its entry until the trace leaves its code.
Which transfers the counter can SEE is a parameter: 'all' (every taken transfer: ideal), 'back' (backward
branches only: today's reverse-loop idea), 'back+dyn' (backward branches
plus the transfers the router handles anyway: RTS / RTR / RTE, JMP / JSR through a register-dependent EA),
'back+dyn+call' (also static calls).

A block is approximated from the trace itself (no code image needed): from the entry along the executed path
until a terminator (call, return, dynamic JMP), through taken
branches forward (and backward ones that stay inside the block), up to CAP source bytes.  Cost model (cycles;
see docs/design/CORTEX_A8.md): a slot-tier instruction costs SLOT cycles; a block
saves G cycles per executed instruction and costs C cycles per translated instruction (the slot fill is
already paid, since promotion happens after T executions).

  .venv/bin/python tools/study/tiersim.py trace.bin [--be] [--drop lo,hi] [--g 0.9] [--c 435]"""

import sys, os, struct, collections

sys.path.insert(0, os.path.dirname(__file__))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("../gen", "..")]
import maketables as mt

CAP = 512
EXPANSION = 7
SLOT = 6.0


def load(path, be, drop=None):
    t = open(path, "rb").read()
    fmt = ">II" if be else "<II"
    e = [struct.unpack_from(fmt, t, o) for o in range(0, len(t) - 7, 8)]
    if drop:
        e = [x for x in e if not (drop[0] <= x[0] <= drop[1])]
    return e


def classify(op):
    """-> (kind, terminates_block) for the instruction; kind in call_s call_d ret jmp_s jmp_d bra bcc dbcc seq"""
    if op >> 8 == 0x61:
        return "call_s"
    if op & 0xFFC0 == 0x4E80 or op & 0xFFC0 == 0x4EC0:
        m, r = op >> 3 & 7, op & 7
        static = m == 7 and r in (0, 1, 2)
        return ("call_" if op & 0x40 == 0 else "jmp_") + ("s" if static else "d")
    if op in (0x4E75, 0x4E77, 0x4E73):
        return "ret"
    if op >> 12 == 6:
        return "bra" if op >> 8 == 0x60 else "bcc"
    if op & 0xF0F8 == 0x50C8:
        return "dbcc"
    return "seq"


VISIBLE = {
    "all": None,
    "back": {"back"},
    "back+dyn": {"back", "ret", "call_d", "jmp_d", "jmp_s"},
    "back+dyn+call": {"back", "ret", "call_d", "jmp_d", "jmp_s", "call_s"},
}


class Trace:
    def __init__(self, entries):
        n = len(entries)
        self.n = n
        self.pc = [x[0] for x in entries]
        self.op = [x[1] for x in entries]
        self.size = [2 * mt.ext_word_count(op) for op in self.op]
        self.kind = [classify(op) for op in self.op]
        # arrival[i]: how instruction i was reached ('' = fall-through): the kind of the transfer, 'back' for
        # a backward branch
        self.arrival = [""] * n
        for i in range(n - 1):
            if self.pc[i + 1] != self.pc[i] + self.size[i]:
                k = self.kind[i]
                if k in ("bra", "bcc", "dbcc") and self.pc[i + 1] <= self.pc[i]:
                    k = "back"
                elif k == "seq":
                    k = "odd"
                self.arrival[i + 1] = k
        self.term = [k in ("call_s", "call_d", "ret", "jmp_d") for k in self.kind]

    def form(self, i):
        """The block entered at trace index i: (set of pcs, bytes, instructions)."""
        S = {}
        nbytes = 0
        j = i
        while j < self.n and len(S) < 400:
            p = self.pc[j]
            if p not in S:
                if nbytes + self.size[j] > CAP:
                    break
                S[p] = self.size[j]
                nbytes += self.size[j]
            if self.term[j]:
                break
            if j + 1 < self.n and self.arrival[j + 1] and self.arrival[j + 1] not in ("call_s",):
                if self.arrival[j + 1] == "back" and self.pc[j + 1] not in S:
                    break  # a backward branch out of the block ends it
            j += 1
        return S, nbytes


def simulate(tr, T=128, table=4096, vis="all", arena=262144, policy="lru", g=0.9, c=435):
    seen = VISIBLE[vis]
    tab_tag = [None] * table
    tab_cnt = [0] * table
    blocks = {}  # entry pc -> [set, host bytes, hits, last use]
    order = collections.OrderedDict()  # for fifo / lru
    used = 0
    built = 0
    rebuilt = 0
    ever = set()
    instr_built = 0
    inblock = 0
    entries = 0
    evicted = 0
    cur = None
    tick = 0
    for i in range(tr.n):
        pc = tr.pc[i]
        arr = tr.arrival[i]
        tick += 1
        # execution accounting
        if cur is not None:
            if pc in cur[0]:
                inblock += 1
            else:
                cur = None
        if cur is None and arr and pc in blocks:
            cur = blocks[pc]
            entries += 1
            inblock += 1
            cur[2] += 1
            if policy == "lru":
                order.move_to_end(pc)
        # counting + promotion
        if arr and arr != "odd" and (seen is None or arr in seen) and pc not in blocks:
            h = ((pc >> 1) * 2654435761 >> 8) % table
            if tab_tag[h] != pc:
                tab_tag[h] = pc
                tab_cnt[h] = 0
            if tab_cnt[h] < 255:
                tab_cnt[h] += 1
            if tab_cnt[h] >= T:
                S, nb = tr.form(i)
                hb = nb * EXPANSION
                if hb <= arena:
                    while used + hb > arena and order:
                        if policy == "lfu":
                            victim = min(order, key=lambda k: blocks[k][2])
                        else:
                            victim = next(iter(order))
                        used -= blocks[victim][1]
                        del blocks[victim]
                        del order[victim]
                        evicted += 1
                        if cur is not None and cur[0] is None:
                            cur = None
                    blocks[pc] = [S, hb, 0]
                    order[pc] = 1
                    used += hb
                    built += 1
                    instr_built += len(S)
                    if pc in ever:
                        rebuilt += 1
                    ever.add(pc)
                tab_tag[h] = None
                tab_cnt[h] = 0
    return dict(
        inblock=inblock / tr.n,
        built=built,
        rebuilt=rebuilt,
        instr_built=instr_built,
        entries=entries,
        evicted=evicted,
        net=(g * inblock - c * instr_built) / (SLOT * tr.n),
        per_entry=(inblock / entries if entries else 0),
    )


def main():
    args = sys.argv[1:]
    be = "--be" in args
    drop = None
    if "--drop" in args:
        k = args.index("--drop")
        drop = [int(x, 0) for x in args[k + 1].split(",")]
        del args[k : k + 2]
    g = 0.9
    c = 435
    if "--g" in args:
        k = args.index("--g")
        g = float(args[k + 1])
        del args[k : k + 2]
    if "--c" in args:
        k = args.index("--c")
        c = float(args[k + 1])
        del args[k : k + 2]
    rep = 1
    if "--repeat" in args:
        k = args.index("--repeat")
        rep = int(args[k + 1])
        del args[k : k + 2]
    path = [a for a in args if not a.startswith("--")][0]
    ents = load(path, be, drop)
    if rep > 1:
        # a periodic program (Dhrystone): tile the loop to get a steady state
        ents = [x for _ in range(rep) for x in ents]
    tr = Trace(ents)
    print(
        "%s: %d instructions, %d distinct pcs, %d taken transfers (g=%.1f cycles saved per block instruction, c=%d cycles per translated instruction)"
        % (os.path.basename(path), tr.n, len(set(tr.pc)), sum(1 for a in tr.arrival if a), g, c)
    )

    def row(label, **kw):
        r = simulate(tr, g=g, c=c, **kw)
        print(
            "  %-34s in blocks %5.1f%%  built %5d (rebuilt %4d, evicted %4d)  instr translated %6d  instr/entry %6.1f  net %+6.2f%% of slot time"
            % (
                label,
                100 * r["inblock"],
                r["built"],
                r["rebuilt"],
                r["evicted"],
                r["instr_built"],
                r["per_entry"],
                100 * r["net"],
            )
        )

    print("\nbounds:")
    row("everything at once (T=1, big arena)", T=1, arena=1 << 30, table=1 << 20)
    row("today: T=1, 256 KB FIFO ring", T=1, arena=262144, policy="fifo", table=1 << 20)
    print("\nthreshold x what the counter can see (table 4096, arena 256 KB, lru):")
    for vis in VISIBLE:
        for T in (16, 64, 128, 256):
            row("%s T=%d" % (vis, T), T=T, vis=vis)
    print("\ntable size (T=128, all):")
    for tb in (64, 256, 1024, 4096):
        row("table %d" % tb, T=128, table=tb)
    print("\narena size x replacement (T=128, all):")
    for ar in (8192, 16384, 32768, 65536, 262144):
        for pol in ("fifo", "lru", "lfu"):
            row("arena %dK %s" % (ar // 1024, pol), T=128, arena=ar, policy=pol)


if __name__ == "__main__":
    main()
