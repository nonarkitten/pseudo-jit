#!/usr/bin/env python3
"""What one small program costs in each tier, and what the tiers make of it: runs tests/qemu/smoke_tiers.c (a sum of four words with ADD.W (A0)+ / DBRA)
single-stepped under QEMU, cuts the trace into its runs (interpreted twice, then from the cache three times, the first one translating), and prints per run the ARM
instructions executed and the cycles tools/study/cyclemodel.py gives them, by where they ran, then the slot and block code the cache holds, disassembled.
The last runs are a program that is only the STOP: what entering and leaving pjit_run costs, to subtract from the others.
The model has no cache misses and no cost for cache maintenance: claim_line's cleaning and invalidating of 64 lines costs about 16,000 cycles on the board
(tests/bbb/bbb_claimcost.c), not what the model says. Needs capstone (the .venv has it).

  make VARIANT=tiers build/tiers/qemu/smoke_tiers.elf                 (slot cache: the tiered default; the program is too short to be promoted)
  make VARIANT=tblk BLK=1 build/tblk/qemu/smoke_tiers.elf             (blocks alone)
  .venv/bin/python tools/study/tiers.py build/tiers/qemu/smoke_tiers.elf [--seq]     (--seq: the order in which the routines ran, warm runs)
"""

import sys, os, re, glob, bisect, subprocess, tempfile, collections

sys.path.insert(0, os.path.dirname(__file__))
import cyclemodel
from capstone import Cs, CS_ARCH_ARM, CS_MODE_ARM

elf = sys.argv[1]
objdir = os.path.join(os.path.dirname(elf), "o")
RUNS = ["interpreted, first", "interpreted, second", "cached, first", "cached, second", "cached, third", "STOP alone, interpreted", "STOP alone, cached, first",
        "STOP alone, cached, second"]
OWN_OBJECT = ("blk.o", "blksched.o", "bcache.o", "blkstate.o", "tier.o")  # these are reported as a whole, the rest by routine

syms = []
for l in subprocess.run(["arm-none-eabi-nm", "-n", elf], capture_output=True, text=True).stdout.splitlines():
    p = l.split()
    if len(p) == 3 and p[1] in "tTwWrRdDbB" and not p[2].startswith("$"):
        syms.append((int(p[0], 16), p[2]))
addrs = [a for a, _ in syms]
where = {n: a for a, n in syms}
owner = {}
for o in glob.glob(os.path.join(objdir, "**", "*.o"), recursive=True):
    for l in subprocess.run(["arm-none-eabi-nm", "--defined-only", o], capture_output=True, text=True).stdout.splitlines():
        p = l.split()
        if len(p) == 3 and p[1] in "tT" and not p[2].startswith(("$", ".")):
            owner.setdefault(p[2], os.path.basename(o))


def sym(pc):
    i = bisect.bisect_right(addrs, pc) - 1
    return syms[i] if i >= 0 else (0, "?")


log = tempfile.NamedTemporaryFile(suffix=".log", delete=False).name
out = subprocess.run(["qemu-system-arm", "-M", "virt", "-cpu", "cortex-a15", "-m", "128M", "-nographic", "-semihosting", "-icount", "shift=0",
                      "-accel", "tcg,one-insn-per-tb=on", "-d", "in_asm,exec,nochain", "-D", log, "-kernel", elf],
                     capture_output=True, text=True, timeout=1800).stdout.replace("\r", "")
cache = int(re.search(r"DR cache at 0x([0-9a-f]+)", out).group(1), 16)
ring = where.get("blk_ring")


def place(pc):
    if cache <= pc < cache + (4 << 20):
        return "slot code"
    if ring is not None and ring <= pc < ring + 0x40000:
        return "block code"
    n = sym(pc)[1]
    if n == "pjit_arena":
        return "opcode table entries"
    if n == "pjit_drsr":
        return "shared bodies (DRSR)"
    o = owner.get(n, "")
    return o if o in OWN_OBJECT else n


mark = where["tiers_mark"]
enc, runs, k = {}, collections.defaultdict(list), 0
in_re = re.compile(r"^0x([0-9a-f]+):\s+([0-9a-f]{8})\s")
ex_re = re.compile(r"^Trace .*?\[\w+/(\w+)/")
with open(log, errors="replace") as f:
    for line in f:
        m = in_re.match(line)
        if m:
            enc[int(m.group(1), 16)] = int(m.group(2), 16)
            continue
        m = ex_re.match(line)
        if not m:
            continue
        pc = int(m.group(1), 16)
        if pc == mark:
            k += 1
            continue
        if k % 2 == 1 and k // 2 < len(RUNS):
            runs[k // 2].append((pc, enc.get(pc, 0xE1A00000)))
os.unlink(log)

total = {}
for i, name in enumerate(RUNS):
    seq = runs[i]
    sim = cyclemodel.Sim()
    cyc = sim.run(seq, place)
    total[i] = (len(seq), cyc)
    ins = collections.Counter(place(pc) for pc, _ in seq)
    print(f"== {name}: {len(seq)} ARM instructions, {cyc:.0f} modelled cycles")
    for b, c in sorted(sim.bucket.items(), key=lambda x: -x[1]):
        print(f"   {ins[b]:7d} instructions {c:8.0f} cycles  {b}")
    if "--seq" in sys.argv and i in (1, 3):
        rl = []
        for pc, _ in seq:
            b = place(pc)
            if rl and rl[-1][0] == b:
                rl[-1][1] += 1
            else:
                rl.append([b, 1])
        print("   in order: " + ", ".join(f"{b} {c}" for b, c in rl))

print("== the program alone (the run minus the STOP-alone run of the same kind): ARM instructions, modelled cycles")
for i, b in ((1, 5), (2, 7), (3, 7)):
    print(f"   {RUNS[i]:22s} {total[i][0] - total[b][0]:7d} {total[i][1] - total[b][1]:8.0f}")

md = Cs(CS_ARCH_ARM, CS_MODE_ARM)
for tag in ("SLOT", "BLOCK"):
    words = [(int(a, 16), int(w, 16)) for a, w in re.findall(r"^\[" + tag + r"\] ([0-9A-F]{8}) ([0-9A-F]{8})", out, re.M)]
    if not words:
        continue
    base, end = words[0][0], words[-1][0] + 4
    print(f"== {tag.lower()} code ({len(words)} words)" + (": 16 bytes per guest halfword, g+n = the guest byte offset" if tag == "SLOT" else ""))
    for a, w in words:
        ins = list(md.disasm(w.to_bytes(4, "little"), a))
        txt = f"{ins[0].mnemonic} {ins[0].op_str}" if ins else f".word 0x{w:08x}"
        t = re.match(r"b\w*\s+#0x([0-9a-f]+)$", txt)
        if t:
            to = int(t.group(1), 16)
            s0, sn = sym(to)
            txt += f"   (+0x{to - base:x})" if base <= to < end else f"   <{sn}{'' if to == s0 else '+0x%x' % (to - s0)}>"
        g = f"g+{(a - base) // 8:<3d}" if tag == "SLOT" and (a - base) % 16 == 0 else "      " if tag == "SLOT" else ""
        print(f"   {g}{a - base:+5x}: {w:08x}  {txt}")
print(out.splitlines()[-1] if out.strip() else "no output")
