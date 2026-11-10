#!/usr/bin/env python3
"""A rough Cortex-A8 pipeline model over an executed-instruction trace (see tools/study/cyclerun.py).

In-order, dual-issue: two instructions per cycle if they are independent, at most one load/store (and one
multiply) per cycle, a branch only as the second of a pair; results are ready ALU +1, shifted-operand ALU +2,
load +3, multiply +4; MRS/MSR/SETEND/CPS/SVC/CP15 issue alone and block for a few cycles; LDM/STM take
ceil(n/2) cycles alone. A taken branch costs one bubble; a mispredict costs 13 cycles: conditional branches go
through a 2-bit-counter table, direct branches are always predicted, returns through a return stack and
other indirect branches by last target. L1 hits only (no cache or TLB misses).
It is a model, not the chip: use it to rank changes, not to quote a number.

The defaults are fitted to the BeagleBone: 40 straight-line segments of generated code timed as loops (tests/bbb/bbb_blocks.c, tools/study/fitmodel.py), the control-flow\nloops of tests/bbb/bbb_branch.c (tools/study/branchcal.py) and the cached Dhrystone / Tiny BASIC runs; docs/HISTORY.md says how well each agrees.
"""

import struct, collections
from capstone import *
from capstone.arm import *

MISPREDICT = 13
LOAD_LAT = 2  # cycles from a load's issue to its consumer (the board's micro-benchmarks say about 2)
SHIFT_LAT = 1  # a shifted-operand ALU op
# a store reads its data register this many cycles late (and so may pair with / follow closely the instruction
# that produces it)
STORE_SLACK = 1
# may an instruction that reads the flags issue in the same cycle as the one that sets them? 0 no, 1 a branch
# may, 2 anything may
COND_PAIR = 0
# 1: an instruction may issue in the same cycle as the older ALU op whose result it reads (it then sees it one
# cycle later)
DEP_PAIR = 0
TAKEN_BUBBLE = 0  # extra cycles of a correctly predicted taken branch
md = Cs(CS_ARCH_ARM, CS_MODE_ARM)
md.detail = True
SERIAL = {
    "mrs": 9,
    "msr": 1,
    "msr_g": 27,
    "setend": 5,
    "cps": 5,
    "cpsid": 5,
    "cpsie": 5,
    "svc": 20,
    "mcr": 6,
    "mrc": 6,
    "mcrr": 6,
    "mrrc": 6,
    "isb": 6,
    "dsb": 6,
    "dmb": 3,
    "wfi": 2,
    "nop_serial": 0,
}
CC = ("eq", "ne", "cs", "cc", "mi", "pl", "vs", "vc", "hi", "ls", "ge", "lt", "gt", "le", "hs", "lo")
BRANCH = ("b", "bl", "bx", "blx")
NOT_SPLIT = ("bic", "bics", "bfi", "bfc", "bkpt", "rbit", "rev", "rev16", "revsh")


def base_name(insn):
    m = insn.mnemonic
    if insn.cc in (ARM_CC_AL, ARM_CC_INVALID) or m in NOT_SPLIT:
        return m
    for c in CC:
        if m.endswith(c):
            return m[:-2]
    return m


class Ins:
    __slots__ = (
        "sdata",
        "name",
        "pc",
        "kind",
        "reads",
        "writes",
        "lat",
        "ls",
        "mul",
        "serial",
        "nregs",
        "shifted",
        "rflags",
        "wflags",
        "branch",
        "link",
        "is_return",
        "cond",
        "writeback",
    )


_cache = {}


def decode(pc, word):
    key = (pc, word)
    if key in _cache:
        return _cache[key]
    i = Ins()
    i.pc = pc
    i.name = "invalid"
    code = struct.pack("<I", word)
    insn = next(md.disasm(code, pc), None)
    if insn is None:
        i.kind, i.reads, i.writes = "alu", (), ()
        i.lat = 1
        i.ls = i.mul = i.serial = False
        i.nregs = 0
        i.shifted = False
        i.rflags = i.wflags = False
        i.branch = None
        i.link = False
        i.is_return = False
        i.cond = False
        i.writeback = False
        _cache[key] = i
        return i
    b = base_name(insn)
    i.name = b
    if b == "msr" and "apsr_g" in insn.op_str:
        b = i.name = "msr_g"
    rr, rw = insn.regs_access()
    names = lambda ids: tuple(insn.reg_name(r) for r in ids)
    reads = set(names(rr))
    writes = set(names(rw))
    i.rflags = (
        ("cpsr" in reads)
        or insn.cc not in (ARM_CC_AL, ARM_CC_INVALID)
        or b in ("adc", "sbc", "rsc", "adcs", "sbcs", "rscs", "mrs")
    )
    i.wflags = insn.update_flags or b in ("msr",) or "cpsr" in writes
    reads.discard("cpsr")
    writes.discard("cpsr")
    i.cond = insn.cc not in (ARM_CC_AL, ARM_CC_INVALID)
    i.reads = tuple(reads)
    i.writes = tuple(writes)
    i.shifted = (
        any(
            op.type == ARM_OP_REG and op.shift.type != ARM_SFT_INVALID and op.shift.type != 0
            for op in insn.operands
        )
        if insn.operands
        else False
    )
    i.serial = b in SERIAL
    i.mul = b.startswith(("mul", "mla", "smul", "umul", "smla", "umla"))
    i.ls = b.startswith(("ldr", "str")) or b in ("push", "pop") and False
    multi = b.startswith(("ldm", "stm")) or b in ("push", "pop", "vpush", "vpop")
    i.nregs = len(insn.operands) - (1 if b.startswith(("ldm", "stm")) else 0) if multi else 0
    i.writeback = insn.writeback
    i.sdata = None
    if (
        b.startswith("str")
        and insn.operands
        and insn.operands[0].type == ARM_OP_REG
        and not b.startswith(("strex", "strd"))
    ):
        i.sdata = insn.reg_name(insn.operands[0].reg)
        for op in insn.operands[1:]:
            if op.type == ARM_OP_MEM and i.sdata in (
                insn.reg_name(op.mem.base) if op.mem.base else "",
                insn.reg_name(op.mem.index) if op.mem.index else "",
            ):
                i.sdata = None
    i.kind = "multi" if multi else "mem" if i.ls else "mul" if i.mul else "serial" if i.serial else "alu"
    i.lat = (
        4
        if i.mul
        else (
            LOAD_LAT
            if i.kind in ("mem", "multi") and (b.startswith("ldr") or b.startswith("ldm") or b == "pop")
            else SHIFT_LAT if i.shifted else 1
        )
    )
    if b == "mrs":
        i.lat = 2
    i.link = b in ("bl", "blx")
    pcw = "pc" in writes
    i.branch = None
    if b in ("b", "bl"):
        i.branch = "direct"
    elif b in ("bx", "blx") and not insn.operands[0].type == ARM_OP_IMM:
        rn = insn.reg_name(insn.operands[0].reg)  # capstone leaves the target register out of regs_access
        i.reads = tuple(set(i.reads) | {rn})
        i.branch = "return" if rn == "lr" and b == "bx" else "indirect"
    elif b == "blx":
        i.branch = "direct"
    elif pcw:
        i.branch = (
            "return" if (b in ("pop", "ldm", "ldmia") or ("lr" in reads and b == "mov")) else "indirect"
        )
    i.is_return = i.branch == "return"
    _cache[key] = i
    return i


class Sim:
    def __init__(self):
        self.cycle = 0
        self.slots = 0
        self.ready = collections.defaultdict(int)
        self.flag_ready = 0
        self.ls_used = self.mul_used = False
        self.bht = {}
        self.last_target = {}
        self.ras = []
        self.instrs = 0
        self.stall = collections.Counter()
        self.bucket = collections.Counter()
        self.mis = collections.Counter()
        self.group_single = False
        self.group_writes = set()
        self.group_wflags = False
        self.by = {}
        self.flag_by = ""
        self.dep = collections.Counter()  # who produced each register, and what the stalls were waiting for

    def new_group(self, to):
        self.cycle = to
        self.slots = 0
        self.ls_used = self.mul_used = False
        self.group_writes = set()
        self.group_wflags = False

    def run(self, seq, bucket_of=None):
        n = len(seq)
        for idx in range(n):
            pc, word = seq[idx][:2]
            # an explicit next pc: the trace was reordered
            nxt = seq[idx][2] if len(seq[idx]) > 2 else (seq[idx + 1][0] if idx + 1 < n else pc + 4)
            i = decode(pc, word)
            c0 = self.cycle
            # operands
            t = self.cycle
            why = None
            for r in i.reads:
                rr_ = self.ready[r] - (STORE_SLACK if r == i.sdata else 0)
                if rr_ > t:
                    t = rr_
                    why = self.by.get(r, "?")
            # a branch reads the flags in the same cycle that sets them (if COND_PAIR)
            fr = self.flag_ready - (1 if (COND_PAIR and i.branch) else 0)
            if i.rflags and fr > t:
                t = fr
                why = self.flag_by
            if t > self.cycle:
                self.stall["dependency"] += t - self.cycle
                self.new_group(t)
                self.dep[(why, i.name)] += t - c0
            if i.serial or i.kind == "multi":
                if self.slots:
                    self.stall["single-issue"] += 1
                    self.new_group(self.cycle + 1)
            elif self.slots == 1:
                # pairing rules
                bad = False
                if (i.kind == "mem" and self.ls_used) or (i.kind == "mul" and self.mul_used):
                    bad = True
                if (
                    any(r in self.group_writes and not (r == i.sdata and STORE_SLACK) for r in i.reads)
                    and not DEP_PAIR
                ):
                    bad = True
                if i.rflags and self.group_wflags and not (COND_PAIR == 2 or (COND_PAIR == 1 and i.branch)):
                    bad = True
                if self.group_single:
                    bad = True
                if bad:
                    self.stall["pairing"] += 1
                    self.new_group(self.cycle + 1)
            elif self.slots >= 2:
                self.new_group(self.cycle + 1)
            # issue
            self.slots += 1
            self.instrs += 1
            if i.kind == "mem":
                self.ls_used = True
            if i.kind == "mul":
                self.mul_used = True
            self.group_single = i.serial or i.kind == "multi"
            for r in i.writes:
                self.ready[r] = self.cycle + i.lat
                self.group_writes.add(r)
                self.by[r] = i.name
            if i.wflags:
                self.flag_ready = self.cycle + 1
                self.group_wflags = True
                self.flag_by = i.name
            if i.kind == "multi":
                extra = max(1, (i.nregs + 1) // 2)
                self.new_group(self.cycle + extra)
                for r in i.writes:
                    self.ready[r] = max(self.ready[r], self.cycle)
            elif i.serial:
                pen = SERIAL.get(i.name, 3)
                self.stall["serialising"] += pen
                self.new_group(self.cycle + max(1, pen))
            # branches
            if i.branch:
                taken = nxt != pc + 4
                wrong = False
                if i.branch == "direct":
                    if i.cond:
                        ctr = self.bht.get(pc, 2)
                        wrong = (ctr >= 2) != taken
                        self.bht[pc] = min(3, ctr + 1) if taken else max(0, ctr - 1)
                elif i.branch == "return":
                    pred = self.ras.pop() if self.ras else None
                    wrong = pred != nxt
                else:
                    wrong = self.last_target.get(pc) != nxt
                    self.last_target[pc] = nxt
                if i.link:
                    self.ras.append(pc + 4)
                if wrong:
                    self.stall["mispredict"] += MISPREDICT
                    self.new_group(self.cycle + 1 + MISPREDICT)
                    if bucket_of:
                        self.mis[(bucket_of(pc), i.branch)] += 1
                elif taken:
                    self.stall["taken-branch bubble"] += TAKEN_BUBBLE
                    self.new_group(self.cycle + 1 + TAKEN_BUBBLE)
            if bucket_of:
                end = self.cycle
                self.bucket[bucket_of(pc)] += max(0, end - c0)
        return self.cycle + (1 if self.slots else 0)
