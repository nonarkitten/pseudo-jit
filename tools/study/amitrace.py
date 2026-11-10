#!/usr/bin/env python3
"""Records the instruction stream of an Amiga program run under vamos (amitools' AmigaOS emulator) in the trace
format the block tools read (tools/study/blockdyn.py,
tools/study/blockstudy.py, ...): <prefix>.trace = 8-byte big-endian entries (PC, opcode word), <prefix>.img =
the memory around the program's first instruction
(its loaded and relocated code), <prefix>.base = that image's address.

Needs amitools and machine68k 0.3.0 (a different, throwaway virtualenv: `python3 -m venv ami; ami/bin/pip
install amitools machine68k==0.3.0`; the project's
.venv does not have them). Run it with that environment's python:
    ami/bin/python3 tools/study/amitrace.py prefix max_instructions [--span 0x20000] -- <vamos options>
    program args...
e.g.  ami/bin/python3 tools/study/amitrace.py lha_a 3000000 -- -C 68000 -q lha_68k a out.lzh file.txt"""

import sys, struct


def main():
    a = sys.argv[1:]
    prefix, limit = a[0], int(a[1], 0)
    a = a[2:]
    span = 0x20000
    if a and a[0] == "--span":
        span = int(a[1], 0)
        a = a[2:]
    assert a and a[0] == "--"
    vargs = a[1:]
    from amitools.vamos.trace import mgr

    out = open(prefix + ".trace", "wb")
    count = [0]
    buf = []
    state = {"img": False}

    def setup(self, with_regs):
        cpu = self.cpu
        mem = self.machine.mem
        REG_PC = mgr.REG_PC

        def hook():
            pc = cpu.r_reg(REG_PC)
            if not state["img"]:  # the first instruction: snapshot the loaded program
                state["img"] = True
                base = (pc & ~0xFFF) - 0x1000
                data = bytearray()
                for ad in range(base, base + span, 2):
                    data += struct.pack(">H", mem.r16(ad))
                open(prefix + ".img", "wb").write(data)
                open(prefix + ".base", "w").write("0x%x\n" % base)
            if count[0] < limit:
                buf.append(struct.pack(">II", pc, mem.r16(pc)))
                count[0] += 1
                if len(buf) >= 65536:
                    out.write(b"".join(buf))
                    buf.clear()

        self.machine.set_instr_hook(hook)

    mgr.TraceManager.setup_cpu_instr_trace = setup
    from amitools.tools import vamos

    sys.argv = ["vamos", "-I"] + vargs
    try:
        vamos.main()
    finally:
        out.write(b"".join(buf))
        out.close()
        sys.stderr.write("amitrace: %d instructions recorded\n" % count[0])


main()
