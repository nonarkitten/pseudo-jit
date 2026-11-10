"""Runs a generated (or old) ARM instruction sequence under Unicorn with the runtime's register conventions, so
it can be
compared with the 68000 oracle (tools/gen/check/oracle1.c) without QEMU: r5 = the dispatch table base with
cpu_t just below,
D0-D7 big-endian at r5 + CPU_D0 + 4n, A0-A7 in r6-r13, 68K N Z V C in ARM N Z C V, X in the GE flags (GE[3:0]
= 1111 when X is clear; the byte at CPU_SR + 1 is only a decoy here), ambient
big-endian data."""

import struct, os

# D0 lives in r4 (tools/gen/gen.py PIN_D0): it goes in and comes out there, cpu_t's slot is a decoy
PIN_D0 = os.environ.get("PJIT_PIN_D0", "1") == "1"
from unicorn import Uc, UC_ARCH_ARM, UC_MODE_ARM, UC_HOOK_CODE, UC_HOOK_INTR
from unicorn.arm_const import *

CODE, STATE, SENTINEL, HELPERS, UNRESOLVED = 0x1000000, 0x20000, 0x30000, 0x400000, 0x31000
R5 = STATE + 0x1000
CPSR_E, CPSR_SYS = 0x200, 0x1F


class Sim:
    def __init__(self, offsets):
        self.d0 = offsets["CPU_D0"]
        self.xb = offsets["CPU_SR"] + 1
        self.sr_off = offsets["CPU_SR"]
        self.usp_off = offsets["CPU_USP"]
        self.ssp_off = offsets["CPU_SSP"]
        self.last_usp = 0
        self.uc = Uc(UC_ARCH_ARM, UC_MODE_ARM)
        self.sym = {}
        self.r5 = R5
        self.cpu_size = offsets["CPU_SIZE"]
        # the runtime helper routines (make check-gen builds host/helpers.bin), for sequences that branch to
        # them
        try:
            # the build directory whose helper routines run (a pinned-D0 build has its own)
            B = os.path.join(os.environ.get("PJIT_BUILD", "build/default"), "host")
            blob = open(B + "/helpers.bin", "rb").read()
            self.uc_helpers = blob
            for l in open(B + "/helpers.sym"):
                p = l.split()
                if len(p) == 3 and p[1] in "TtWwDdBb":
                    self.sym[p[2]] = int(p[0], 16)
        except OSError:
            self.uc_helpers = None
        if "pjit_arena" in self.sym:
            # cpu_t is the start of the arena, r5 the DT base after it
            self.r5 = self.sym["pjit_arena"] + self.cpu_size
        self.trapped = None
        self.alloc = 0

        def on_intr(
            uc, intno, ud
        ):  # an `svc` in the sequence: the runtime would raise a 68K exception; record which one and stop
            pc = uc.reg_read(UC_ARM_REG_PC)
            self.trapped = struct.unpack("<I", bytes(uc.mem_read(pc - 4, 4)))[0] & 0xFFFFFF
            uc.emu_stop()

        self.uc.hook_add(UC_HOOK_INTR, on_intr)
        self.uc.mem_map(CODE, 0x1000000)
        self.slot = {}
        self.uc.mem_map(STATE, 0x4000)
        self.uc.mem_map(SENTINEL, 0x1000)
        self.uc.mem_map(UNRESOLVED, 0x1000)
        self.uc.mem_write(UNRESOLVED, struct.pack("<I", 0xE7F000F0) * 1024)
        self.uc.mem_map(0x4000, 0x1000)
        # the memory area; the guest's code pages (inline data is read as data)
        self.uc.mem_map(0x2000, 0x2000)
        if self.uc_helpers:
            self.uc.mem_map(HELPERS, (len(self.uc_helpers) + 0xFFF) & ~0xFFF)
            self.uc.mem_write(HELPERS, self.uc_helpers)

    CCN = {
        "eq": 0,
        "ne": 1,
        "cs": 2,
        "cc": 3,
        "mi": 4,
        "pl": 5,
        "vs": 6,
        "vc": 7,
        "hi": 8,
        "ls": 9,
        "ge": 10,
        "lt": 11,
        "gt": 12,
        "le": 13,
        "al": 14,
    }

    def encode(self, w, addr):
        """A symbolic branch to a runtime routine becomes a real branch to where the helper blob put it."""
        if not isinstance(w, tuple):
            return w
        kind, cond, name = w
        if isinstance(name, tuple):
            name = next((n for n in name if n in self.sym), name[0])
        # a runtime routine we do not model (pjit_goto, the irq stubs): a page of undefined instructions, so
        # running into one fails loudly
        target = self.sym.get(name, UNRESOLVED)
        return (
            self.CCN[cond] << 28
            | (0x0B000000 if kind == "bl" else 0x0A000000)
            | ((target - (addr + 8)) >> 2) & 0xFFFFFF
        )

    def run(
        self,
        words,
        d,
        a,
        ccr,
        x_garbage=0x0F,
        area=None,
        r1=0xDEADBEEF,
        r2=0xDEADBEEF,
        sr_hi=0x27,
        guest=None,
    ):
        """d: 8 data registers, a: 8 address registers, ccr: 5 bits (X N Z V C = bit4..bit0). -> (d, a, ccr)."""
        uc = self.uc
        n = bool(ccr & 8)
        z = bool(ccr & 4)
        v = bool(ccr & 2)
        c = bool(ccr & 1)
        # the mode first: switching to System mode swaps the banked sp/lr, so set the registers after
        uc.reg_write(
            UC_ARM_REG_CPSR,
            CPSR_SYS | CPSR_E | n << 31 | z << 30 | c << 29 | v << 28 | (0 if ccr & 0x10 else 0xF) << 16,
        )
        # one address per distinct sequence, written once: Unicorn would keep translated code for a rewritten
        # address
        key = tuple(words)
        if key not in self.slot:
            self.slot[key] = CODE + self.alloc
            self.alloc += (
                4 * len(words) + 255
            ) & ~255  # (a block is longer than a single instruction's sequence)
            uc.mem_write(
                self.slot[key],
                b"".join(
                    struct.pack("<I", self.encode(w, self.slot[key] + 4 * i)) for i, w in enumerate(words)
                ),
            )
        entry = self.slot[key]
        for i in range(8):
            uc.mem_write(
                self.r5 + self.d0 + 4 * i, struct.pack(">I", d[i] if not (PIN_D0 and i == 0) else 0xCAFEBABE)
            )
        # a decoy: X is in GE now, so a sequence that reads this byte gets the wrong X
        uc.mem_write(self.r5 + self.xb, bytes([(0 if ccr & 0x10 else 0x10) | (x_garbage & 0x0F)]))
        # the SR's system byte: supervisor, interrupt mask 7 (the oracle's reset state)
        uc.mem_write(self.r5 + self.sr_off, bytes([sr_hi]))
        uc.mem_write(self.r5 + self.usp_off, bytes(4))
        uc.mem_write(self.r5 + self.ssp_off, bytes(4))  # both stack pointers start at 0
        regs = [
            UC_ARM_REG_R6,
            UC_ARM_REG_R7,
            UC_ARM_REG_R8,
            UC_ARM_REG_R9,
            UC_ARM_REG_R10,
            UC_ARM_REG_R11,
            UC_ARM_REG_R12,
            UC_ARM_REG_R13,
        ]
        for r, v in zip(regs, a):
            uc.reg_write(r, v)
        for r in (UC_ARM_REG_R0, UC_ARM_REG_R3, UC_ARM_REG_R4):
            uc.reg_write(r, 0xDEADBEEF)
        if PIN_D0:
            uc.reg_write(UC_ARM_REG_R4, d[0])
        uc.reg_write(UC_ARM_REG_R1, r1)
        uc.reg_write(UC_ARM_REG_R2, r2)  # the fill's prefix: source / destination extension value
        if area is not None:
            uc.mem_write(0x4000, area)
        if guest is not None:
            uc.mem_write(0x2000, bytes(guest[0x2000:0x4000]))
        uc.reg_write(UC_ARM_REG_R5, self.r5)
        uc.reg_write(UC_ARM_REG_LR, SENTINEL)
        self.trapped = None
        uc.emu_start(entry, SENTINEL, count=400000)
        if self.trapped is not None:
            return None  # trapped: see `trapped`
        cp = uc.reg_read(UC_ARM_REG_CPSR)
        dd = [struct.unpack(">I", bytes(uc.mem_read(self.r5 + self.d0 + 4 * i, 4)))[0] for i in range(8)]
        aa = [uc.reg_read(r) for r in regs]
        if PIN_D0:
            dd[0] = uc.reg_read(UC_ARM_REG_R4)
        x = 0 if (cp >> 16) & 1 else 1  # GE0 is set when X is clear
        garea = bytes(uc.mem_read(0x4000, 0x1000))
        sr_hi = bytes(uc.mem_read(self.r5 + self.sr_off, 1))[0]
        usp_mem = struct.unpack(">I", bytes(uc.mem_read(self.r5 + self.usp_off, 4)))[0]
        # the user stack pointer: in cpu_t while supervisor, live in A7 while user
        self.last_usp = usp_mem if sr_hi & 0x20 else aa[7]
        return (
            dd,
            aa,
            sr_hi << 8
            | (x << 4)
            | (cp >> 31 & 1) << 3
            | (cp >> 30 & 1) << 2
            | (cp >> 28 & 1) << 1
            | (cp >> 29 & 1),
            garea,
        )
