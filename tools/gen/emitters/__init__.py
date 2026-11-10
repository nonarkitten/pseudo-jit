"""The instruction emitters, one mixin per group of 68K instructions.  `gen.Gen` is these mixins plus the
dispatcher.

    ea.py       effective addresses and the data registers      (ea_load / ea_store, dload / dstore)
    alu.py      the ALU core, flags, MOVE / ADD / SUB / AND / OR / EOR / CMP families
    shift.py    shifts and rotates
    misc.py     CLR NEG NOT ADDQ SUBQ EXT SWAP TST MUL CMPM LEA EXG bit-ops Scc
    xops.py     ADDX SUBX NEGX
    flow.py     Bcc BRA BSR DBcc JMP JSR RTS RTR TRAP ...
    sr.py       SR / CCR / USP, RTE STOP RESET
    complex.py  DIV LINK UNLK PEA MOVEP MOVEM ABCD SBCD NBCD CHK TAS

A method named `emit_<NAME>` is the emitter that tools/gen/optab_rows.py names for some opcodes; the other
methods are the helpers they
share.  Conventions (registers, flags, the packed EA) are described at the top of gen.py.
"""
