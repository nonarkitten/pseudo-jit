"""Runs the asm scheduler (pjit/blksched.s) under Unicorn on a list of ARM words: schedule(words, d0) -> the
rescheduled words, or None if it left them alone.
Assembles pjit/blksched.s with arm-none-eabi-as / ld on first use."""

import os, sys, subprocess, struct

sys.path.insert(0, os.path.dirname(__file__))
from paths import build_path
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM
from unicorn.arm_const import *

CODE, WS, REG, STK = 0x10000000, 0x10100000, 0x10200000, 0x10300000
SENT = 0x10400000
_inst = None


def build(out=None):
    out = out or build_path("host", "blksched.bin")
    os.makedirs(build_path("host"), exist_ok=True)
    subprocess.check_call(
        [
            "arm-none-eabi-as",
            "-mcpu=cortex-a8",
            "-I",
            "pjit",
            "pjit/blksched.s",
            "-o",
            build_path("host", "blksched.o"),
        ]
    )
    subprocess.check_call(
        [
            "arm-none-eabi-ld",
            "-Ttext=0x%x" % CODE,
            "-e",
            "blk_sched",
            "-o",
            build_path("host", "blksched.elf"),
            build_path("host", "blksched.o"),
        ]
    )
    subprocess.check_call(["arm-none-eabi-objcopy", "-O", "binary", build_path("host", "blksched.elf"), out])
    syms = subprocess.check_output(["arm-none-eabi-nm", build_path("host", "blksched.elf")]).decode()
    return {l.split()[2]: int(l.split()[0], 16) for l in syms.splitlines() if len(l.split()) == 3}


class Sched:
    def __init__(self, d0):
        syms = build()
        self.entry = syms["blk_sched"]
        self.size = syms["blk_sched_size"]
        blob = open(build_path("host", "blksched.bin"), "rb").read()
        self.uc = Uc(UC_ARCH_ARM, UC_MODE_ARM)
        self.uc.mem_map(CODE, (len(blob) + 0xFFFF) & ~0xFFFF)
        self.uc.mem_write(CODE, blob)
        self.uc.mem_map(WS, 0x10000)
        self.uc.mem_map(REG, 0x10000)
        self.uc.mem_map(STK, 0x10000)
        self.uc.mem_map(SENT, 0x1000)
        self.d0 = d0
        self.calls = 0
        self.done = 0
        self.insns = 0

    def __call__(self, words, d0=None):
        uc = self.uc
        words = [int(w) for w in words]
        uc.mem_write(REG, struct.pack("<%dI" % len(words), *words))
        uc.mem_write(WS, bytes(self.size))
        uc.reg_write(UC_ARM_REG_R0, REG)
        uc.reg_write(UC_ARM_REG_R1, REG + 4 * len(words))
        uc.reg_write(UC_ARM_REG_R2, WS)
        uc.reg_write(UC_ARM_REG_R3, (self.d0 if d0 is None else d0) & 0xFFFFFFFF)
        for r in range(4, 12):
            uc.reg_write(UC_ARM_REG_R4 + r - 4, 0x1000 + r)
        uc.reg_write(UC_ARM_REG_SP, STK + 0x8000)
        uc.reg_write(UC_ARM_REG_LR, SENT)
        self.calls += 1
        n = [0]
        uc.emu_start(self.entry, SENT, count=5000000)
        ok = uc.reg_read(UC_ARM_REG_R0)
        for r in range(4, 12):
            assert uc.reg_read(UC_ARM_REG_R4 + r - 4) == 0x1000 + r, "r%d not preserved" % r
        assert uc.reg_read(UC_ARM_REG_SP) == STK + 0x8000
        if ok != 1:
            return None
        self.done += 1
        return list(struct.unpack("<%dI" % len(words), bytes(uc.mem_read(REG, 4 * len(words)))))


def get(d0):
    global _inst
    if _inst is None:
        _inst = Sched(d0)
    return _inst
