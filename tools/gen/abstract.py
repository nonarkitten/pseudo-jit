"""Procedural abstraction of the DRSR: bodies of rarely run instructions share
the words they have in common.

A body U + T, where the tail T is also the tail of other bodies, becomes U + `b T'` and T' is stored once (a
tail merge: no link register, nothing saved, nothing
clobbered; the `bx lr` in T' still returns to whoever called the body). A run M anywhere else in a body (U + M
+ V) becomes U + `add r4,pc,#0 ; b M'` + V, with
M' = M + `bx r4` stored once: r4 is the second link register (the interpreter's PC lives in cpu->ipc and
cached bodies never use r4), so lr stays what it was. Therefore
a body that uses r4 itself cannot call, and a shared run (which has r4 live) cannot call either. Relative
branches that jump over a replaced run are re-aimed; a branch
into the middle of one, or a run that cuts a computed `add pc` jump, is not replaced.

Which bodies may be rewritten is a size-versus-speed budget (budget_eligible): a rewritten body costs one or
two more taken branches and is no longer inlined into a slot or
copied by the loop compactor, so the bodies are taken in ascending order of how often their opcodes execute
(tools/gen/weights.py: the mean of the normalised Dhrystone, Tiny
BASIC and Kickstart-static profiles) until the weight of the opcodes affected reaches the budget eps. Sweep
(DRSR bytes): eps 0 67,200; 0.001 65,280; 0.003 64,128 (the
knee: no Dhrystone / Tiny BASIC instruction pays); 0.01 62,416; everything 61,728 (80,000 without the pass).

analyse(bodies, chosen) -> Library;  Library.map: original body (tuple) -> abstracted words (REF markers),
Library.routines: id -> words; expand() undoes it.
"""

import collections, os

# D0 lives in r4: no second link register, so only tail merges (`b T'`) remain
PIN_D0 = os.environ.get("PJIT_PIN_D0", "1") == "1"
from arm32 import add as arm_add, bx as arm_bx

REF = "REF"
ADD_R4_PC = arm_add(4, 15, ("imm", 0))  # pc reads as .+8: the word after the `b` that follows
BX_R4 = arm_bx(4)
MIN_INNER, MAX_INNER = 5, 48


def is_br(w):
    return (
        not isinstance(w, tuple)
        and (int(w) >> 25 & 7) == 5
        and (int(w) >> 28) != 0xF
        and not (int(w) >> 24 & 1)
    )


def br_target(k, w):
    return k + 2 + (((int(w) & 0xFFFFFF) ^ 0x800000) - 0x800000)


def is_pcadd(w):  # a computed jump: add pc, pc, rX, lsl #n over the next few words
    if isinstance(w, tuple):
        return False
    w = int(w)
    return (w >> 26 & 3) == 0 and (w >> 12 & 15) == 15 and (w >> 28) != 0xF and (w & 0x0FFFFFF0) != 0x012FFF10


def jumps(b):
    """[(k, [target indices], is_computed)] of the relative control flow inside a body."""
    out = []
    for k, w in enumerate(b):
        if isinstance(w, tuple):
            continue
        if is_br(w):
            out.append((k, [br_target(k, w)], False))
        elif is_pcadd(w):
            out.append((k, [k + 2, k + 3, k + 4, k + 5], True))
    return out


def mentions(w, r):
    """Conservatively: does the word name register r in any register field (or a load / store multiple's
    list)?
    """
    if isinstance(w, tuple):
        return False
    w = int(w)
    if (w >> 25 & 7) == 4:
        return bool(w >> r & 1) or (w >> 16 & 15) == r
    return r in (w >> 16 & 15, w >> 12 & 15, w >> 8 & 15, w & 15)


def not_in_run(w):
    """A word an inner run may not contain: a symbol, a word that reads pc or unconditionally leaves the body
    (bx rX, b, bl).
    """
    if isinstance(w, tuple):
        return True
    w = int(w)
    if (w >> 28) == 0xE and (w & 0x0FFFFFF0) == 0x012FFF10:
        return True
    if (w >> 25 & 7) == 5 and ((w >> 28) == 0xE or (w >> 24 & 1)):
        return True
    if is_pcadd(w) or is_br(w):
        return False
    return mentions(w, 15)


def window_ok(jmp, s, e):
    """May the words s..e-1 be replaced by a call? Jumps inside stay inside (or go to e, the end of the run);
    nothing outside jumps into the middle; a computed jump
    outside may not have the run in reach."""
    for k, ts, comp in jmp:
        if s <= k < e:
            if comp and k + 5 >= e:
                return False
            if any(not (s <= t <= e) for t in ts):
                return False
        else:
            if comp:
                if not (k + 6 <= s or k >= e):
                    return False
                continue
            if any(s < t < e for t in ts):
                return False
    return True


def tail_ok(jmp, s):
    """May the tail starting at word s be moved out? Nothing before it may jump into it (other than to its
    first word), and nothing in it may jump before it.
    """
    for k, ts, comp in jmp:
        if k < s:
            if any(t > s for t in ts):
                return False
        else:
            if any(t < s for t in ts):
                return False
    return True


class Library:
    def __init__(self):
        self.map = {}
        self.routines = []
        self.inner = set()
        self.saved = 0

    def routine(self, words, inner=False):
        self.routines.append(list(words))
        if inner:
            self.inner.add(len(self.routines) - 1)
        return len(self.routines) - 1


def replace_window(b, s, e, call):
    """b with words s..e-1 replaced by the words `call`; relative branches that jump over the change are
    re-aimed.
    """

    def newpos(i):
        return i if i <= s else i - (e - s) + len(call)

    out = list(b[:s]) + list(call) + list(b[e:])
    for k, ts, comp in jumps(b):
        if s <= k < e or comp:
            continue
        t = ts[0]
        nk, nt = newpos(k), newpos(t)
        if nt - nk == t - k:
            continue
        out[nk] = (int(b[k]) & 0xFF000000) | ((nt - nk - 2) & 0xFFFFFF)
    return out


def budget_eligible(bodies, freq, eps):
    """The bodies to rewrite when at most the fraction eps of the (weighted) executed instructions may pay for
    it: ascending by the summed weight of their users,
    while the running total (each opcode counted once) stays within eps.  eps = 0 rewrites only the bodies
    none of whose users appears in any profile; eps = inf
    rewrites every body. -> (the chosen keys, the weight they cover, the opcodes they cover)"""
    order = sorted(bodies, key=lambda k: sum(freq.get(op, 0) for op in bodies[k]))
    chosen, ops, cum = set(), set(), 0.0
    for k in order:
        new = bodies[k] - ops
        w = sum(freq.get(op, 0) for op in new)
        if cum + w > eps:
            break
        cum += w
        chosen.add(k)
        ops |= bodies[k]
    return chosen, cum, ops


def analyse(bodies, chosen, min_save=4, log=None):
    """bodies: {key tuple: set of user opcodes}; chosen: the set of body keys that may be rewritten
    (budget_eligible). -> Library
    """
    lib = Library()
    items = {}  # name -> words; ('B', key) for a body, ('R', rid) for a shared routine
    for key, users in bodies.items():
        if len(key) < 6 or key not in chosen:
            continue
        items[("B", key)] = list(key)
    while True:
        best = None  # (saving, kind, run, count)
        tails = collections.Counter()
        jmps = {}
        grams = collections.defaultdict(list)
        for name, b in items.items():
            jmps[name] = jumps(b)
            for s in range(1 if name[0] == "R" else 0, len(b) - 3):
                if tail_ok(jmps[name], s):
                    tails[tuple(b[s:])] += 1
            if name[0] == "B" and not PIN_D0 and not any(mentions(w, 4) for w in b):
                for s in range(0, len(b) - MIN_INNER + 1):
                    grams[tuple(b[s : s + MIN_INNER])].append((name, s))
        for t, c in tails.items():
            if c < 2:
                continue
            L = len(t)
            save = c * (L - 1) - L
            if save >= min_save and (
                best is None or save > best[0] or (save == best[0] and L > len(best[2]))
            ):
                best = (save, "tail", t, c)
        inner = collections.Counter()
        for gram, occ in grams.items():
            if len(occ) < 2 or any(not_in_run(w) for w in gram):
                continue
            for name, s in occ:
                b = items[name]
                jm = jmps[name]
                for L in range(MIN_INNER, min(MAX_INNER, len(b) - s) + 1):
                    if not_in_run(b[s + L - 1]):
                        break
                    if window_ok(jm, s, s + L):
                        inner[tuple(b[s : s + L])] += 1
        for run, c in inner.items():
            if c < 2:
                continue
            L = len(run)
            save = c * (L - 2) - (L + 1)
            if save >= min_save and (best is None or save > best[0]):
                best = (save, "inner", run, c)
        if best is None:
            break
        save, kind, run, c = best
        L = len(run)
        if kind == "tail":
            rid = lib.routine(run)
            items[("R", rid)] = list(run)
            call = [(REF, "al", rid)]
        else:
            rid = lib.routine(list(run) + [BX_R4], inner=True)
            items[("R", rid)] = list(run) + [BX_R4]
            call = [ADD_R4_PC, (REF, "al", rid)]
        for name in list(items):
            if name == ("R", rid):
                continue
            b = items[name]
            if kind == "tail":
                n = len(b)
                if (
                    n >= L + (1 if name[0] == "R" else 0)
                    and tuple(b[n - L :]) == run
                    and tail_ok(jumps(b), n - L)
                ):
                    items[name] = b[: n - L] + call
            elif name[0] == "B" and not any(mentions(w, 4) for w in b):
                s = 0
                while s + L <= len(b):
                    if tuple(b[s : s + L]) == run and window_ok(jumps(b), s, s + L):
                        b = replace_window(b, s, s + L, call)
                        s += len(call)
                    else:
                        s += 1
                items[name] = b
        lib.saved += save
        if log:
            log("%s %d words x %d: saves %d" % (kind, L, c, save))
    for (kind, key), b in items.items():
        if kind == "B" and list(b) != list(key):
            lib.map[key] = b
        elif kind == "R":
            lib.routines[key] = b
    return lib


def expand(words, lib):
    """The inverse of the rewriting: every call put back as the words it stands for (branches re-aimed as
    needed).
    """
    words = list(words)
    while True:
        for i, w in enumerate(words):
            if isinstance(w, tuple) and w[0] == REF:
                break
        else:
            return words
        if i > 0 and words[i - 1] is ADD_R4_PC:
            words = replace_window(words, i - 1, i + 1, lib.routines[w[2]][:-1])
        else:
            words = replace_window(words, i, i + 1, lib.routines[w[2]])
