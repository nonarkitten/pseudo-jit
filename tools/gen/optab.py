"""Which of the emitters handles each opcode: the rows in optab_rows.py applied in order (a later row overwrites
an earlier one).
Legality is in the rows' masks."""

import numpy as np
from optab_rows import ROWS


def load():
    names = sorted({r[2] for r in ROWS})
    idx = {n: i for i, n in enumerate(names)}
    ops = np.arange(65536)
    winner = np.full(65536, -1, dtype=np.int32)
    for m, e, n in ROWS:
        winner[(ops & m) == e] = idx[n]
    return [names[i] if i >= 0 else None for i in winner]
