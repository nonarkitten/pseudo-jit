"""Per-class (read / write) bus timeline from a Saleae capture: ch1 = X (GPMC request, via M0:42), ch2 ASn, ch5
RnW, ch6 DTACKn, ch7 CLK7.
All times in CLK7 periods after the ASn fall, medians; 'period' = next ASn fall (any class) histogram in
half-clock bins.
"""

import sys, numpy as np, collections

CLK = 1 / 7.09379e6


def load(d):
    a = np.loadtxt(d + "/digital.csv", delimiter=",", skiprows=1)
    a = a[a[:, 0] > 0.002]
    return a[:, 0], a[:, 1:].astype(np.int8)


def edges(t, x, lvl):
    i = np.nonzero((x[1:] == lvl) & (x[:-1] != lvl))[0] + 1
    return t[i]


def nxt(l, x):
    i = np.searchsorted(l, x, "right")
    return np.where(i < len(l), l[np.minimum(i, len(l) - 1)], np.nan)


def run(d):
    t, v = load(d)
    asf, asr = edges(t, v[:, 2], 0), edges(t, v[:, 2], 1)
    xf, xr = edges(t, v[:, 1], 0), edges(t, v[:, 1], 1)
    dk = edges(t, v[:, 6], 0)
    rw = v[
        np.searchsorted(t, asf - 0.2 * CLK) - 1 if False else np.minimum(np.searchsorted(t, asf), len(t) - 1),
        5,
    ]
    n = len(asf) - 1
    a = asf[:n]
    nf = asf[1 : n + 1]
    r = nxt(asr, a)
    ok = r < nf
    per = (nf - a) / CLK
    out = {}
    for name, sel in (("READ", rw[:n] == 1), ("WRITE", rw[:n] == 0)):
        m = sel & ok
        if m.sum() < 50:
            out[name] = "n=%d" % m.sum()
            continue
        f = lambda e: np.nanmedian((nxt(e, a[m]) - a[m]) / CLK)
        h = collections.Counter(np.round(per[m] * 2) / 2)
        top = ", ".join("%.1f:%d" % (k, c) for k, c in sorted(h.items(), key=lambda kv: -kv[1])[:4])
        out[name] = "n=%d  DTACK %.2f  Xrise %.2f  ASrise %.2f  Xfall(next) %.2f  period med %.2f  [%s]" % (
            m.sum(),
            f(dk),
            f(xr),
            np.nanmedian((r[m] - a[m]) / CLK),
            f(xf),
            np.median(per[m]),
            top,
        )
    return out


for d in sys.argv[1:]:
    o = run(d)
    print(d.split("/")[-1])
    [print("  %-6s %s" % (k, x)) for k, x in o.items()]
