"""Edge sequence of the probed node (ch1) per cycle, as medians per ordinal, relative to the ASn fall (ch2).
Capture dirs: <label>_r / <label>_s.
ch1 = probed node (M0:<n>), ch2 ASn, ch3 UDSn, ch5 RnW, ch6 DTACKn, ch7 CLK7 pin.  Windows start 2.2 clk
before the AS fall for the request-side nodes (42, 17), else 0.2 before.
"""

import sys, numpy as np

CLK = 1 / 7.09379e6


def load(d):
    a = np.loadtxt(d + "/digital.csv", delimiter=",", skiprows=1)
    a = a[a[:, 0] > 0.003]
    return a[:, 0], a[:, 1:].astype(np.int8)


def edges(t, x, lvl):
    i = np.nonzero((x[1:] == lvl) & (x[:-1] != lvl))[0] + 1
    return t[i]


def fa(arr, base, lo, hi):
    i = np.searchsorted(arr, base + lo)
    j = np.minimum(i, len(arr) - 1)
    return np.where((i < len(arr)) & (arr[j] <= base + hi), arr[j], np.nan)


def stat(x, ref):
    d = (x - ref) / CLK
    d = d[np.isfinite(d)]
    return (np.median(d), np.mean(d), len(d)) if len(d) > 20 else (np.nan, np.nan, len(d))


def main(path, request_side):
    t, v = load(path)
    asf, asr = edges(t, v[:, 2], 0), edges(t, v[:, 2], 1)
    asf = asf[2:-3]
    nxt = np.concatenate([asf[1:], [asf[-1] + 6 * CLK]])
    dk, ck = edges(t, v[:, 6], 0), edges(t, v[:, 7], 1)
    pe = np.sort(np.concatenate([edges(t, v[:, 1], 0), edges(t, v[:, 1], 1)]))
    pol = v[np.minimum(np.searchsorted(t, pe), len(t) - 1), 1]
    start = asf - (2.2 if request_side else 0.2) * CLK
    i0 = np.searchsorted(pe, start)
    print(path.split("/")[-1])
    r = lambda lab, x: print(
        "   %-28s median %5.2f clk  (%4.0f ns)  mean %5.2f   n=%d"
        % ((lab,) + (lambda s: (s[0], s[0] * CLK * 1e9, s[1], s[2]))(stat(x, asf)))
    )
    r("ASn rise (pin)", fa(asr, asf, 0, 6 * CLK))
    r("DTACKn fall (pin)", fa(dk, asf, -CLK, 6 * CLK))
    r("next ASn fall", nxt)
    j = np.searchsorted(ck, asf, side="right") - 1
    ph = (asf - ck[j]) / CLK
    print(
        "   AS fall after the CLK7 rising edge: mean %.2f clk (%.0f ns), 10-90%% %.2f-%.2f"
        % (ph.mean(), ph.mean() * CLK * 1e9, np.percentile(ph, 10), np.percentile(ph, 90))
    )
    for k in range(5):
        idx = i0 + k
        ok = (
            (idx < len(pe)) & (pe[np.minimum(idx, len(pe) - 1)] < nxt - 0.2 * CLK)
            if not request_side
            else (idx < len(pe))
        )
        tt = np.where(ok, pe[np.minimum(idx, len(pe) - 1)], np.nan)
        pp = np.where(ok, pol[np.minimum(idx, len(pe) - 1)], -1)
        if np.isfinite(tt).sum() < 100:
            continue
        s = stat(tt, asf)
        rise = (pp == 1).mean() / max(1e-9, (pp >= 0).mean())
        print(
            "   probe edge #%d  %-5s  median %5.2f clk (%4.0f ns)  mean %5.2f  n=%d"
            % (k + 1, "RISE" if rise > 0.5 else "FALL", s[0], s[0] * CLK * 1e9, s[1], s[2])
        )


for p in sys.argv[1:]:
    node = p.rstrip("/").split("/")[-1].split("_")[0]
    main(p, node in ("n42", "n17"))
