#!/usr/bin/env python3
"""Runs a tracing build under QEMU (single-stepped, filtered to the runtime, arena, DRSR and cache), replays
the executed ARM instructions through tools/study/cyclemodel.py and reports modelled Cortex-A8 cycles per run,
from
the difference of two run lengths. Needs the capstone Python package (python3 -m venv v; v/bin/pip install
capstone).

  make build/smoke_dhrystone_dr.elf
  v/bin/python tools/study/cyclerun.py build/smoke_dhrystone_dr.elf 10 30 [--m68k trace_dhrystone_a.bin
  trace_dhrystone_b.bin 20]
"""

import sys, os, re, subprocess, bisect, collections

sys.path.insert(0, os.path.dirname(__file__))
import cyclemodel

for _kv in sys.argv:
    if _kv.startswith("--set="):
        _k, _v = _kv[6:].split("=")
        setattr(cyclemodel, _k, int(_v))
for _kv in sys.argv:
    if _kv.startswith("--set="):
        _k, _v = _kv[6:].split("=")
        setattr(cyclemodel, _k, int(_v))

_pos = [x for x in sys.argv[1:] if not x.startswith("--")]
elf, ra, rb = _pos[0], int(_pos[1]), int(_pos[2])
nm = subprocess.run(["arm-none-eabi-nm", "-n", "-S", elf], capture_output=True, text=True).stdout.splitlines()
syms = {}
slist = []
for l in nm:
    p = l.split()
    if len(p) >= 3 and p[-2] in "TtWwBbDd":
        syms[p[-1]] = int(p[0], 16)
        slist.append((int(p[0], 16), p[-1]))
sizes = {l.split()[3]: int(l.split()[1], 16) for l in nm if len(l.split()) == 4}
addrs = [a for a, _ in slist]


def sym(a):
    return slist[bisect.bisect_right(addrs, a) - 1][1]


# the bases are printed at run time
out = (
    subprocess.run(
        [
            "timeout",
            "20",
            "qemu-system-arm",
            "-M",
            "virt",
            "-cpu",
            "cortex-a15",
            "-m",
            "128M",
            "-nographic",
            "-semihosting",
            "-kernel",
            elf,
        ],
        capture_output=True,
        text=True,
    ).stdout
    if False
    else ""
)
try:
    p = subprocess.run(
        [
            "qemu-system-arm",
            "-M",
            "virt",
            "-cpu",
            "cortex-a15",
            "-m",
            "128M",
            "-nographic",
            "-semihosting",
            "-kernel",
            elf,
        ],
        capture_output=True,
        text=True,
        timeout=8,
    )
    out = p.stdout
except subprocess.TimeoutExpired as e:
    out = (e.stdout or b"").decode() if isinstance(e.stdout, bytes) else (e.stdout or "")
# the block runtime (BLK=1 build): the runtime text, the ring of translated blocks
cache = syms["blk_ring"]
drsr = 0
drsr_size = 0
text_lo = syms["abcd"]
text_hi = slist[bisect.bisect_right(addrs, syms["blk_dbtpl"])][0]  # the next symbol after the last of blk.s
arena = syms["pjit_arena"]
arena_hi = arena + sizes["pjit_arena"]
filt = "0x%x..0x%x,0x%x..0x%x,0x%x..0x%x" % (text_lo, text_hi, cache, cache + 0x40000, arena, arena_hi)
log = "/tmp/cyclerun.log"
subprocess.run(
    [
        "qemu-system-arm",
        "-M",
        "virt",
        "-cpu",
        "cortex-a15",
        "-m",
        "128M",
        "-nographic",
        "-semihosting",
        "-accel",
        "tcg,one-insn-per-tb=on",
        "-d",
        "in_asm,exec,nochain",
        "-dfilter",
        filt,
        "-D",
        log,
        "-kernel",
        elf,
    ],
    capture_output=True,
    timeout=900,
)

enc = {}
segs = [[]]
run_pc = syms["pjit_run"]
in_re = re.compile(r"^0x([0-9a-f]+):\s+([0-9a-f]{8})\s")
ex_re = re.compile(r"^Trace .*?\[\w+/(\w+)/")
for line in open(log, errors="replace"):
    m = in_re.match(line)
    if m:
        enc[int(m.group(1), 16)] = int(m.group(2), 16)
        continue
    m = ex_re.match(line)
    if m:
        pc = int(m.group(1), 16)
        if pc == run_pc:
            segs.append([])
        segs[-1].append((pc, enc.get(pc, 0xE1A00000)))
print("segment lengths", [len(x) for x in segs], file=sys.stderr)
segs = [s for s in segs if len(s) > 1000][-2:]
assert len(segs) == 2, "expected two runs, got %d segments" % len(segs)

import pickle

if "--save" in sys.argv:
    pickle.dump(segs, open(sys.argv[sys.argv.index("--save") + 1], "wb"))
    sys.exit(0)


def bucket(pc):
    if cache <= pc < cache + 0x40000:
        return "ring block %05x" % ((pc - cache) >> 8 << 8)
    if arena <= pc < arena_hi:
        return "arena table (movem)"
    return sym(pc)


res = []
for s in segs:
    sim = cyclemodel.Sim()
    c = sim.run(s, bucket)
    res.append((c, sim))
n = rb - ra
cyc = (res[1][0] - res[0][0]) / n
ins = (len(segs[1]) - len(segs[0])) / n
print(
    "%s: per run  %.0f ARM instructions, %.0f modelled A8 cycles (IPC %.2f)"
    % (os.path.basename(elf), ins, cyc, ins / cyc)
)
print("  where the cycles are lost (per run):")
for k in sorted(res[1][1].stall, key=lambda k: -(res[1][1].stall[k] - res[0][1].stall[k])):
    print("    %-22s %7.0f" % (k, (res[1][1].stall[k] - res[0][1].stall[k]) / n))
print("  dependency stalls by (producer -> consumer), cycles per run:")
dd = collections.Counter()
for k in res[1][1].dep:
    dd[k] = (res[1][1].dep[k] - res[0][1].dep[k]) / n
for k, v in dd.most_common(10):
    print("    %-22s %7.0f" % ("%s -> %s" % k, v))
print("  hottest code (cycles per run):")
tot = collections.Counter()
for k in res[1][1].bucket:
    tot[k] = (res[1][1].bucket[k] - res[0][1].bucket[k]) / n
for k, v in tot.most_common(12):
    print("    %-28s %7.0f" % (k, v))
print("  mispredicted branches per run (where):")
mis = collections.Counter()
for k in res[1][1].mis:
    mis[k] = (res[1][1].mis[k] - res[0][1].mis[k]) / n
for k, v in mis.most_common(8):
    print("    %-34s %6.1f" % ("%s (%s)" % k, v))
if "--sched" in sys.argv:
    import sched

    print("  instruction scheduling, upper bounds (tools/study/sched.py):")
    sched.study(segs, lambda pc: True, "every window of the trace reordered", n)
    sched.study(segs, lambda pc: cache <= pc < cache + 0x400000, "only code in the cache slots reordered", n)

    def coarse(pc):
        if drsr <= pc < drsr + drsr_size:
            return "shared body"
        if cache <= pc < cache + 0x400000:
            return "cache slot"
        if arena <= pc < arena_hi:
            return "table"
        return "core:" + sym(pc)

    sched.taken_branches(segs, coarse, n)
    sched.slot_branch_distances(segs, lambda pc: cache <= pc < cache + 0x400000, n)
    sched.study_inline_length(segs, lambda pc: drsr <= pc < drsr + drsr_size, n)
    sched.study_flat(segs, "everything inlined (no branches or returns to bodies)", n)
if "--m68k" in sys.argv:
    i = sys.argv.index("--m68k")
    import cycles68k

    ta = cycles68k.total(sys.argv[i + 1])[0]
    tb = cycles68k.total(sys.argv[i + 2])[0]
    runs = int(sys.argv[i + 3])
    c68 = (tb - ta) / runs
    print(
        "  68000: %.0f cycles per run (7.09 MHz: %.0f us); A8 model at 1 GHz: %.2f us per run  ->  %.0fx a 7 MHz 68000, i.e. a %.0f MHz-equivalent 68000"
        % (c68, c68 / 7.09, cyc / 1000.0, (c68 / 7.09) / (cyc / 1000.0), c68 / cyc * 1000.0)
    )
