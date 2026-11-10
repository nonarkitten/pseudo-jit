"""ARM-level dead-code elimination for the generator's sequences: given which of the 68K's two flag groups the
code *after* an instruction still wants
(bit 0: N Z V C = ARM N Z C V; bit 1: X = the GE flags), cut out what only exists to produce the rest.

An ARM instruction already says what it reads and writes, so no per-opcode table is needed: a backward
liveness pass over the words, with the flags N Z C V G (GE)
as five more registers,
  * deletes a pure instruction (data processing, extends, bit-field, mul, loads from the cpu state at r5, mrs,
  uadd8 ...) all of whose results are dead,
  * clears the S bit of one whose flag results are dead but whose register result is not (`adds r0,r0,#0` ->
  `add r0,r0,#0` -> nothing),
  * deletes compares / tests / msr / uadd8 whose flags are dead (the X save `sbc r3,r3,r3 ; uadd8 r3,r3,r3`
  disappears when X is dead).
Branches inside a sequence (forward only) are followed properly; `bx lr` (also conditional) is a jump to the
end, where the live-out set holds. Anything it does not
know is a barrier: everything live before it, never deleted. Registers live at the end: r4-r15 (r0-r3 are
scratch between 68K instructions). Calls into runtime
routines (symbolic branches), svc and every store are barriers / side effects.

  live_in = dce(words, live_out)  ->  (new words, flag groups wanted before them)"""

import struct
import capstone
from capstone.arm_const import *

md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_ARM)
md.detail = True
N, Z, C, V, G = 1, 2, 4, 8, 16
NZCV, ALLF = N | Z | C | V, N | Z | C | V | G
GROUP_NZCV, GROUP_X = 1, 2


def group_flags(groups):
    return (NZCV if groups & GROUP_NZCV else 0) | (G if groups & GROUP_X else 0)


def flag_groups(f):
    return (GROUP_NZCV if f & NZCV else 0) | (GROUP_X if f & G else 0)


COND_READS = [Z, Z, C, C, N, N, V, V, C | Z, C | Z, N | V, N | V, N | Z | V, N | Z | V, 0, 0]
REGNUM = {getattr(__import__("capstone.arm_const", fromlist=["x"]), "ARM_REG_R%d" % i): i for i in range(13)}
REGNUM[ARM_REG_SP], REGNUM[ARM_REG_LR], REGNUM[ARM_REG_PC] = 13, 14, 15
REGS_LIVE_OUT = sum(1 << r for r in range(4, 16))
SIMD_GE = (
    "uadd8",
    "usub8",
    "sadd8",
    "ssub8",
    "uadd16",
    "usub16",
    "sadd16",
    "ssub16",
    "uasx",
    "usax",
    "sasx",
    "ssax",
)


class Ins:
    """What one word does: register / flag reads and writes, and whether it may be deleted when its results
    are dead.
    """

    __slots__ = (
        "w",
        "reads",
        "writes",
        "freads",
        "fkill",
        "fmay",
        "pure",
        "sdrop",
        "branch",
        "barrier",
        "cond",
    )


def analyse(w):
    if isinstance(w, tuple):  # a symbolic branch to a runtime routine: reads everything, may or may not leave
        i = Ins()
        i.w = w
        i.reads, i.writes, i.freads, i.fkill, i.fmay = 0xFFFF, 0, ALLF, 0, 0
        i.pure = i.sdrop = False
        i.branch = None
        i.barrier = True
        i.cond = w[1] != "al"
        return i
    w = int(w)
    i = Ins()
    i.w = w
    i.cond = w >> 28
    i.branch = None
    i.barrier = False
    i.pure = False
    i.sdrop = False
    i.fkill = i.fmay = 0
    i.freads = COND_READS[w >> 28] if w >> 28 < 14 else 0
    ci = next(md.disasm(struct.pack("<I", w), 0), None)
    r, wr = ci.regs_access() if ci is not None else ([], [])
    i.reads = sum(1 << REGNUM[x] for x in r if x in REGNUM)
    i.writes = sum(1 << REGNUM[x] for x in wr if x in REGNUM)
    if ci is None:
        i.barrier = True
        i.reads = 0xFFFF
        i.freads = ALLF
        return i
    mn = ci.mnemonic
    k = w >> 25 & 7
    conditional = (w >> 28) < 14
    if k == 0 and (w & 0x90) == 0x10:
        # a register-controlled shift reads Rs (capstone's access list does not include it)
        i.reads |= 1 << (w >> 8 & 15)
    if w & 0x0FFFFFFF == 0x012FFF1E:  # bx lr: a jump to the end
        i.branch = "end"
        i.reads |= 1 << 14
        return i
    if k == 5:  # b / bl <cc>
        if w >> 24 & 1:
            i.barrier = True
            i.reads = 0xFFFF
            i.freads = ALLF
            return i
        off = ((w & 0xFFFFFF) ^ 0x800000) - 0x800000
        i.branch = ("rel", off + 2)
        return i  # target = this index + 2 + off (the caller resolves it)
    if (w >> 24 & 15) == 15 or mn in (
        "svc",
        "bl",
        "blx",
        "bx",
        "setend",
        "mcr",
        "mrc",
        "dsb",
        "dmb",
        "isb",
        "udf",
        "bkpt",
        "pld",
    ):
        i.barrier = True
        i.reads = 0xFFFF
        i.freads = ALLF
        i.writes = 0
        return i
    if i.writes >> 15 & 1:
        i.barrier = True
        i.reads = 0xFFFF
        i.freads = ALLF
        return i  # writes pc
    # ---- flag effects ----
    # data processing (not mul / extra load-store / misc)
    dp = (
        k in (0, 1)
        and not (k == 0 and (w & 0x90) == 0x90)
        and not (k == 0 and (w & 0x0F900000) == 0x01000000)
    )
    if dp:
        opc = w >> 21 & 15
        s = w >> 20 & 1
        if (
            not s and opc in (8, 9, 10, 11) and not (k == 1 and opc in (8, 10))
        ):  # TST..CMN without S: msr (immediate) and friends -- only movw / movt are ours
            i.barrier = True
            i.reads = 0xFFFF
            i.freads = ALLF
            return i
        if opc in (5, 6, 7):
            i.freads |= C  # adc sbc rsc
        if k == 0 and (w & 0x0E0) == 0x060 and not w & 0x10 and (w >> 7 & 31) == 0:
            i.freads |= C  # rrx
        if s:
            if opc in (2, 3, 4, 5, 6, 7, 10, 11):
                i.fkill = NZCV
            else:
                i.fkill = N | Z
                i.fmay = C  # logical: N Z, the shifter's carry maybe
            if (w >> 28) >= 14:
                pass
            i.sdrop = opc not in (8, 9, 10, 11)  # tst teq cmp cmn have nothing but flags
            i.pure = True
            if (w >> 12 & 15) == 15:
                i.barrier = True
        else:
            i.pure = True
        if opc in (8, 9, 10, 11):
            i.pure = True
    elif k == 0 and (w & 0x0FC000F0) == 0x00000090:  # mul / mla
        i.pure = True
        if w >> 20 & 1:
            i.fkill = N | Z
            i.fmay = C
            i.sdrop = True
    elif mn in SIMD_GE:
        i.fkill = G
        i.pure = True
    elif mn == "sel":
        i.freads |= G
        i.pure = True
    elif mn == "mrs":
        i.freads = ALLF
        i.pure = True
    elif mn == "msr":
        m = w >> 16 & 15
        i.pure = True  # a flag writer only
        if w >> 22 & 1 or m & 3:
            i.barrier = True
            i.reads = 0xFFFF
            i.freads = ALLF
            i.pure = False
            return i  # SPSR / control / extension fields: not ours
        if m & 8:
            i.fkill |= NZCV
        if m & 4:
            i.fkill |= G
    elif mn in (
        "movw",
        "movt",
        "sxth",
        "sxtb",
        "uxth",
        "uxtb",
        "ubfx",
        "sbfx",
        "bfi",
        "bfc",
        "clz",
        "rev",
        "rev16",
        "revsh",
        "nop",
        "sxtah",
        "uxtah",
    ):
        i.pure = True
    elif mn.startswith(("ldr", "ldrb", "ldrh", "ldrsb", "ldrsh")) and not mn.startswith("ldrex"):
        # a load from the cpu state (r5) is pure; one from guest memory may fault or hit a device: keep it
        if (w >> 16 & 15) == 5 and not (w >> 21 & 1) and (w >> 24 & 1) and (w >> 12 & 15) != 15:
            i.pure = True
    if conditional:  # a conditional instruction may not execute: it kills nothing
        i.fmay |= i.fkill
        i.fkill = 0
        i.freads |= 0
    if i.barrier:
        i.reads = 0xFFFF
        i.freads = ALLF
    return i


def dce(words, live_out_groups):
    """-> (the words that remain, the flag groups wanted before them). The sequence must be branch-free or
    have forward branches only.
    """
    ins = [analyse(w) for w in words]
    n = len(ins)
    target = [None] * n
    for k, i in enumerate(ins):
        if i.branch == "end":
            target[k] = n
        elif i.branch is not None:
            target[k] = k + i.branch[1]
    # backward: live registers / flags before each instruction
    live_f = [0] * (n + 1)
    live_r = [0] * (n + 1)
    live_f[n] = group_flags(live_out_groups)
    live_r[n] = REGS_LIVE_OUT
    keep = [True] * n
    newword = list(words)
    for k in range(n - 1, -1, -1):
        i = ins[k]
        # live after this instruction: the next one, plus the branch target
        af, ar = live_f[k + 1], live_r[k + 1]
        if target[k] is not None:
            if target[k] > n or target[k] <= k:
                raise ValueError("backward or out-of-range branch")
            tf, tr = live_f[target[k]], live_r[target[k]]
            if i.cond == 14 or (not isinstance(i.w, tuple) and i.w >> 28 == 14):
                af, ar = tf, tr  # unconditional: only the target
            else:
                af, ar = af | tf, ar | tr
        if isinstance(i.w, tuple) and i.w[1] == "al":
            af, ar = ALLF, 0xFFFF  # a tail call into a runtime routine: all of it is wanted
        if i.barrier:
            live_f[k] = ALLF | af
            live_r[k] = 0xFFFF
            continue
        if i.pure and target[k] is None:
            fout = i.fkill | i.fmay
            if (
                (i.writes & ar) == 0
                and (fout & af) == 0
                and not (i.writes == 0 and fout == 0 and not _removable_nop(i))
            ):
                keep[k] = False
                live_f[k] = af
                live_r[k] = ar
                continue  # all results dead: delete
            if i.sdrop and (fout & af) == 0 and (newword[k] >> 20 & 1):
                newword[k] = int(newword[k]) & ~(1 << 20)
                i.fkill = i.fmay = 0  # flag results dead: drop the S bit
        # transfer
        lf = (af & ~i.fkill) | i.freads | (0)
        lr = (ar & ~(i.writes if not (i.cond < 14) else 0)) | i.reads
        live_f[k], live_r[k] = lf, lr
    out = [w for k, w in enumerate(newword) if keep[k]]
    # a deleted instruction that a forward branch skipped over changes the offsets: relocate
    if not all(keep) or any(newword[k] != words[k] for k in range(n)):
        pos = [0] * (n + 1)
        for k in range(n):
            pos[k + 1] = pos[k] + (1 if keep[k] else 0)
        for k in range(n):
            if not keep[k]:
                continue
            i = ins[k]
            if i.branch is not None and i.branch != "end":
                tgt = pos[target[k]]
                here = pos[k]
                w = int(newword[k])
                off = tgt - here - 2
                newword[k] = (w & 0xFF000000) | (off & 0xFFFFFF)
            if i.branch == "end":
                pass
        out = [newword[k] for k in range(n) if keep[k]]
        # a `bx lr` that is now last and unconditional stays (the caller's end); conditional ones were already
        # branches to the end only in normalised bodies
    return out, flag_groups(live_f[0])


def _removable_nop(i):
    return True
