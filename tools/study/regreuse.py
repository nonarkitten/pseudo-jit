#!/usr/bin/env python3
"""How often is a data register's value still "in hand" when the next instruction wants it? From a dynamic opcode
trace (8-byte entries, little-endian: PC, opcode; as written
by tests/qemu/trace_tinybasic.c or smoke_dhrystone_trace.c) and the program's code image (so the extension
words can be decoded), counts for every read of a D register the
distance, in instructions, to the previous access of that register (read or write) since the last region
boundary. A read within k instructions of an earlier access could
take its value from an ARM register instead of an LDR if the translated code kept it there (what a forwarding
pass over a block would do).
Regions: A = basic blocks (reset at every branch instruction and every PC discontinuity); B = superblocks
(reset only at a PC discontinuity -- taken jumps, calls, returns --
so a not-taken conditional branch does not end the region).
  tools/study/regreuse.py trace.bin image.bin [base_address] [--be]     (--be: the entries are big-endian, as smoke_dhrystone_trace.c writes them)
"""

import sys, struct, collections
import capstone

md = capstone.Cs(capstone.CS_ARCH_M68K, capstone.CS_MODE_M68K_000)
md.detail = True
be = "--be" in sys.argv
args = [a for a in sys.argv[1:] if a != "--be"]
trace = open(args[0], "rb").read()
image = open(args[1], "rb").read()
base = int(args[2], 0) if len(args) > 2 else 0
NOWRITE = {"cmp", "cmpi", "cmpa", "cmpm", "tst", "btst", "pea", "jmp", "jsr", "chk", "movem"}
WRITEONLY = {
    "move",
    "moveq",
    "movea",
    "lea",
    "clr",
    "scc",
    "st",
    "sf",
    "shi",
    "sls",
    "scc",
    "scs",
    "sne",
    "seq",
    "svc",
    "svs",
    "spl",
    "smi",
    "sge",
    "slt",
    "sgt",
    "sle",
}
BRANCH = ("b", "db")


def d_of(reg):
    return reg - 1 if 1 <= reg <= 8 else None


def decode(pc):
    off = pc - base
    i = next(md.disasm(image[off : off + 12], pc), None)
    if i is None:
        return None
    base_m = i.mnemonic.split(".")[0]
    reads, writes, roles = [], [], {}
    ops = i.operands
    for k, o in enumerate(ops):
        dst = k == len(ops) - 1
        if o.type == 1:  # a register
            d = d_of(o.reg)
            if d is None:
                continue
            if dst and len(ops) >= 1 and (len(ops) > 1 or base_m not in ("jmp", "jsr", "pea")):
                if base_m in NOWRITE:
                    reads.append(d)
                    roles[d] = "d"
                elif base_m in WRITEONLY:
                    writes.append(d)
                else:
                    reads.append(d)
                    writes.append(d)
                    roles[d] = "d"
            else:
                reads.append(d)
                roles.setdefault(d, "s")
        elif o.type == 3:  # memory: an index register is read
            try:
                d = d_of(o.mem.index_reg)
                if d is not None:
                    reads.append(d)
            except Exception:
                pass
    if base_m.startswith("db") and ops and ops[0].type == 1 and d_of(ops[0].reg) is not None:
        d = d_of(ops[0].reg)
        reads.append(d)
        writes.append(d)
    return i.size, base_m, reads, writes, roles


entries = [struct.unpack_from(">II" if be else "<II", trace, o) for o in range(0, len(trace) - 7, 8)]
print("%d instructions traced" % len(entries))
cache = {}
for region in ("A", "B"):
    last = {}
    t = 0
    n_reads = 0
    n_writes = 0
    hits = collections.Counter()
    hits_w = collections.Counter()
    first = 0
    prev_end = None
    dist = collections.Counter()
    ninstr = 0
    role = collections.Counter()
    for pc, op in entries:
        if pc not in cache:
            cache[pc] = decode(pc)
        dec = cache[pc]
        if dec is None:
            last = {}
            prev_end = None
            continue
        size, mn, reads, writes, roles = dec
        if prev_end is not None and pc != prev_end:
            last = {}
        t += 1
        ninstr += 1
        for r in set(reads):
            n_reads += 1
            if r in last:
                d = t - last[r][0]
                dist[min(d, 9)] += 1
                if d == 1 and last[r][1] == "w":
                    role[roles.get(r, "s")] += 1
                for k in (1, 2, 3, 4, 8):
                    if d <= k:
                        hits[k] += 1
                        if last[r][1] == "w":
                            hits_w[k] += 1
            else:
                first += 1
        for r in set(reads):
            last[r] = (t, "r") if r not in writes else last.get(r, (t, "r"))
        for r in writes:
            n_writes += 1
            last[r] = (t, "w")
        for r in set(reads) - set(writes):
            last[r] = (t, "r")
        prev_end = pc + size
        if region == "A" and (
            mn.startswith("b")
            and mn not in ("btst", "bset", "bclr", "bchg")
            or mn.startswith("db")
            or mn in ("jsr", "jmp", "rts", "rte", "trap", "bsr")
        ):
            last = {}
    print(
        "\nregion %s (%s): %d D-register reads (%.2f per instruction), %d writes (%.2f per instruction)"
        % (
            region,
            "basic blocks" if region == "A" else "superblocks through not-taken branches",
            n_reads,
            n_reads / ninstr,
            n_writes,
            n_writes / ninstr,
        )
    )
    for k in (1, 2, 3, 4, 8):
        print(
            "   a read within %d instruction%s of an earlier access: %5.1f%% of the reads (%4.1f%% of them after a write)"
            % (k, "s" if k > 1 else " ", 100 * hits[k] / n_reads, 100 * hits_w[k] / max(1, hits[k]))
        )
    print(
        "   adjacent handoffs by role in the consumer: as its destination operand %d (same register as the result: no move), as a source %d (a move or a swap)"
        % (role["d"], role["s"])
    )
    print(
        "   first touch in the region (an LDR is needed whatever happens): %.1f%%" % (100 * first / n_reads)
    )
    print(
        "   distances:",
        " ".join(
            "%s:%.0f%%" % (("%d" % d if d < 9 else "9+"), 100 * c / n_reads) for d, c in sorted(dist.items())
        ),
    )
