#!/usr/bin/env python3
"""Compares tools/study/cyclemodel.py with the board on the straight-line segments of tools/study/blockbench.py
(timed by tests/bbb/bbb_blocks.c: tools/study/blocks_board.txt).
.venv/bin/python tools/study/fitmodel.py [--set=NAME=value ...] [--list]"""

import sys, os, json

sys.path.insert(0, os.path.dirname(__file__))
sys.path.insert(0, os.path.join(os.path.dirname(__file__), ".."))
sys.path[:0] = [os.path.join(os.path.dirname(os.path.abspath(__file__)), p) for p in ("../gen", "..")]
import cyclemodel


def load():
    segs = json.load(
        open(os.path.join(os.path.dirname(__file__), "..", "..", "tests", "data", "blocks_data.json"))
    )
    board = {}
    for l in open(os.path.join(os.path.dirname(__file__), "blocks_board.txt")):
        p = l.split()
        if p and p[0] == "[SEG]":
            board[int(p[1])] = int(p[3]) / 100.0
    return segs, board


def sim_cycles(words, iters=60):
    n = len(words)
    body = list(words) + [0xE2544001, 0x1A000000 | ((-(n + 1) - 2) & 0xFFFFFF)]  # subs r4 ; bne top
    sm = cyclemodel.Sim()
    seq = []
    base = 0x82600000
    stream = []
    for it in range(iters):
        for k, w in enumerate(body):
            stream.append((base + 4 * k, w))
    sm.run(stream)
    c1 = sm.cycle
    sm2 = cyclemodel.Sim()
    sm2.run(stream[: len(body) * (iters // 2)])
    return (c1 - sm2.cycle) / (iters - iters // 2)


def evaluate(segs, board, verbose=False):
    err = 0
    rows = []
    for i, s in enumerate(segs):
        if i not in board:
            continue
        m = sim_cycles(s["words"])
        b = board[i]
        rows.append((i, len(s["words"]), b, m, s["weight"]))
        err += abs(m - b) / b
    return err / len(rows), rows


if __name__ == "__main__":
    for kv in sys.argv[1:]:
        if kv.startswith("--set="):
            k, v = kv[6:].split("=")
            setattr(cyclemodel, k, float(v) if "." in v else int(v))
            cyclemodel._cache.clear()
    segs, board = load()
    e, rows = evaluate(segs, board)
    print(
        "mean relative error %.1f%% over %d segments; total board %.0f model %.0f cycles (weighted by executions: board %.0f model %.0f)"
        % (
            100 * e,
            len(rows),
            sum(r[2] for r in rows),
            sum(r[3] for r in rows),
            sum(r[2] * r[4] for r in rows),
            sum(r[3] * r[4] for r in rows),
        )
    )
    if "--list" in sys.argv:
        for i, n, b, m, w in rows:
            print(
                "  seg %2d %3d words  board %6.2f  model %6.2f  (%+.0f%%)  weight %d"
                % (i, n, b, m, 100 * (m - b) / b, w)
            )
