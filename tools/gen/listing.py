#!/usr/bin/env python3
"""A readable listing of every opcode's generated sequences: the emitter that handles it, then the cached path
and (when it differs)
the interpretive path, one instruction per line, branches to runtime routines by name.   tools/gen/listing.py > build/listing/opcodes.txt
"""

import sys, os

sys.path.insert(0, os.path.dirname(__file__))
import capstone
from gen import Gen, read_offsets

md = capstone.Cs(capstone.CS_ARCH_ARM, capstone.CS_MODE_ARM)


def txt(w):
    if isinstance(w, tuple):
        return f"{w[0]}{'' if w[1] == 'al' else w[1]}\t{w[2]}"
    i = next(md.disasm(w.to_bytes(4, "little"), 0), None)
    return f"{i.mnemonic}\t{i.op_str}" if i else f".word\t0x{w:08X}"


g = Gen(read_offsets())
for op in range(65536):
    seq, seqi = g.generate(op), g.generate(op, interp=True)
    print(f"{op:04X}  {g.winner[op] or 'illegal'}")
    for w in seq:
        print(f"        {txt(w)}")
    if seqi != seq:
        print("      interpretive:")
        for w in seqi:
            print(f"        {txt(w)}")
