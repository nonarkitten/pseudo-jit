"""The generator's switches (environment variables, set by the Makefile) and the sequence type."""

import os

# PJIT_X_LAZY=1 (make X_LAZY=1): ADD / SUB / NEG / ADDQ / SUBQ / shifts do not copy C into X.
# X is then only right where something
# wrote it explicitly (ADDX / SUBX / NEGX and their results, ROXL / ROXR, MOVE to CCR / SR, ...): code that
# needs X after an intervening
# MOVE, CMP or logical op (multi-precision loops with a CMPA in them, say) gets a stale X. It is a speed /
# exactness trade, off by default;
# tests/bbb/bbb_dhrystone.c measured the save at 8% of Dhrystone and half of a SUBQ / BNE loop on the Cortex-A8.
X_LAZY = os.environ.get("PJIT_X_LAZY", "0") == "1"

# PJIT_PIN_D0=1 (make PIN_D0=1): D0 lives in r4 in every mode and every tier (like A0-A7 in r6-r13); cpu->d0
# is only written when the state is parked (exceptions, helpers, C).
PIN_D0 = os.environ.get("PJIT_PIN_D0", "1") == "1"

# Buffee: the 68K function-code line FC2 (supervisor) is a GPIO (GPIO2_10 = LCD_DATA4) that every SR write
# updates (see _fc_sync); FC1 (IACK) is GPIO2_9, FC0 is tied high on the board
FC_GPIO = os.environ.get("PJIT_FC", "0") == "1"


class Seq(list):
    """A generated sequence. `split` > 0 marks the target layout of the 4-word DT: the first `split` words are
    the opcode's own (register numbers, slot addresses; they go
    in the DT entry and in the cache slot), the rest is a routine shared by every opcode of the family (it
    ends in `bx lr`; the DT entry's last word branches to it). Everything
    that only needs the flat sequence (the simulator, the rule checks, the block tables) sees an ordinary
    list.
    """

    def __init__(self, words=(), split=0):
        super().__init__(words)
        self.split = split
