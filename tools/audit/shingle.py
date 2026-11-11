#!/usr/bin/env python3
"""Licence audit: a copy detector. Tokenises source (comments and whitespace ignored), hashes 12-token shingles and compares them with the upstream sources the code
might have come from, which tools/audit/fetch_corpora.sh puts under local/corpora/.

  shingle.py index NAME DIR [DIR...]    build local/corpora/NAME.npy (the sorted shingle hashes of every source file under the directories)
  shingle.py scan LISTFILE|-            per file: the share of its shingles found in each corpus, and the longest shared run (tokens)
  shingle.py blobs [--phrases]          the same for every distinct source blob in the whole history of the repository ($REPO, default .); with --phrases the comment-phrase
                                        comparison of `phrases` instead
  shingle.py show FILE CORPUS [N]       the N longest shared runs of FILE with CORPUS, as text (a path that does not exist is read from stdin)
  shingle.py phrases LISTFILE|-         per file: how many 5-word phrases of its COMMENTS also occur in a corpus (finds ports that changed the code syntax)

Read the result with judgement. A shared run of 20 or more tokens is worth a look (library idioms and one-line register accesses give a dozen); a share above 3% of a
file is worth a look; tables of constants are discounted already. A file copied with its licence header shares nearly everything with its source (TI's CP15 file shares 98% with
StarterWare's); a ported file shares tens of percent; a few lines of idiom give a run of 40 tokens or so.
A port that rewrote the register access in its own macros shares almost no code tokens but keeps the comments, constants and the order of the steps: `phrases` finds those
(licence boilerplate is filtered out; 6 or more shared phrases in a file is worth a look).
$CORPORA overrides the directory of the indexes."""

import os, re, subprocess, sys, zlib
import numpy as np

K = 12
HERE = os.path.dirname(os.path.abspath(__file__))
CORP = os.environ.get("CORPORA", os.path.join(HERE, "..", "..", "local", "corpora"))
EXT = (".c", ".h", ".s", ".S", ".inc", ".cpp", ".cc", ".asm", ".hpp", ".lds", ".ld", ".py")
SKIP_PARTS = ("/fame/", "/lzma/", "/unzip/", "/.git/", "/st/joy", "/st/keyboard", "/docs/", "/doc/")
_ids = {}
P = np.uint64(1000003)

BLOCK = re.compile(r"/\*.*?\*/", re.S)
LINE = re.compile(r"(//[^\n]*|^[ \t]*[@;#!][^\n]*)", re.M)
TOK = re.compile(r"[A-Za-z_.$][A-Za-z_0-9.$]*|0[xX][0-9a-fA-F]+|\d+|\S")


def tokens(text):
    text = BLOCK.sub(" ", text)
    text = re.sub(r"(?m)(?<![\w\"'])//[^\n]*", " ", text)
    text = re.sub(
        r"(?m)^[ \t]*[@;][^\n]*", " ", text
    )  # assembler comment lines (a '#' line may be a preprocessor directive: kept)
    text = re.sub(r"(?m)@[ \t][^\n]*$", " ", text)  # trailing '@ ...' (ARM asm)
    return TOK.findall(text)


NUMISH = re.compile(r"^(0[xX][0-9a-fA-F]+|\d+|[,{};])$")


def dataish_mask(toks):
    """True where a K-window is (almost) all numbers and separators: a table of constants, which proves nothing about copying"""
    m = len(toks) - K + 1
    isnum = np.fromiter((1 if NUMISH.match(t) else 0 for t in toks), dtype=np.int32, count=len(toks))
    c = np.concatenate(([0], np.cumsum(isnum)))
    return (c[K : K + m] - c[:m]) >= K - 1


def shingles(toks, drop_data=True):
    n = len(toks)
    if n < K:
        return np.zeros(0, dtype=np.uint64), 0
    ids = np.fromiter(
        (_ids[t] if t in _ids else _ids.setdefault(t, zlib.crc32(t.encode()) | 1) for t in toks),
        dtype=np.uint64,
        count=n,
    )
    m = n - K + 1
    h = ids[:m].copy()
    for j in range(1, K):
        h = h * P + ids[j : j + m]
    if drop_data:
        h = h[~dataish_mask(toks)]
    return h, n


def runs_longest(mask):
    best = cur = 0
    for v in mask:
        cur = cur + 1 if v else 0
        if cur > best:
            best = cur
    return best + K - 1 if best else 0


def source_files(dirs):
    for d in dirs:
        for root, _, files in os.walk(d):
            for f in files:
                p = os.path.join(root, f)
                if f.endswith(EXT) and not any(s in p.replace("\\", "/") + "/" for s in SKIP_PARTS):
                    yield p


def cmd_index(name, dirs):
    os.makedirs(CORP, exist_ok=True)
    parts, nfiles, ntok = [], 0, 0
    for p in source_files(dirs):
        try:
            t = open(p, encoding="utf-8", errors="replace").read()
        except OSError:
            continue
        h, n = shingles(tokens(t))
        if len(h):
            parts.append(np.unique(h))
        nfiles += 1
        ntok += n
    allh = np.unique(np.concatenate(parts)) if parts else np.zeros(0, dtype=np.uint64)
    np.save(os.path.join(CORP, name + ".npy"), allh)
    import json

    reg = os.path.join(CORP, "corpora.json")
    known = json.load(open(reg)) if os.path.exists(reg) else {}
    known[name] = [os.path.abspath(d) for d in dirs]
    json.dump(known, open(reg, "w"), indent=1)
    print(f"{name}: {nfiles} files, {ntok} tokens, {len(allh)} distinct shingles")


def load_corpora():
    c = {}
    for f in sorted(os.listdir(CORP)):
        if f.endswith(".npy"):
            c[f[:-4]] = np.load(os.path.join(CORP, f))
    return c


def report(label, text, corpora, minshare=0.0, quiet=True):
    h, n = shingles(tokens(text))
    if not len(h):
        return None
    out = []
    for name, arr in corpora.items():
        pos = np.searchsorted(arr, h)
        pos[pos == len(arr)] = 0
        hit = arr[pos] == h if len(arr) else np.zeros(len(h), dtype=bool)
        share = hit.mean()
        if share > minshare:
            out.append((name, share, runs_longest(hit), int(hit.sum())))
    return n, out


def cmd_scan(listfile):
    corpora = load_corpora()
    paths = [l.rstrip("\n") for l in (sys.stdin if listfile == "-" else open(listfile))]
    for p in paths:
        if not p.endswith(EXT) or not os.path.isfile(p):
            continue
        r = report(p, open(p, encoding="utf-8", errors="replace").read(), corpora, 0.005)
        if r and r[1]:
            print(f"{p}  ({r[0]} tokens)")
            for name, share, run, cnt in sorted(r[1], key=lambda x: -x[1]):
                print(
                    f"     {name:12s} share {100 * share:5.1f}%  longest run {run:4d} tokens  ({cnt} shingles)"
                )


def cmd_blobs(with_phrases=False):
    repo = os.environ.get("REPO", ".")
    corpora = None if with_phrases else load_corpora()
    idx = phrase_index() if with_phrases else None
    lst = subprocess.run(
        ["git", "-C", repo, "rev-list", "--objects", "--all"], capture_output=True, text=True
    ).stdout.splitlines()
    seen = {}
    for l in lst:
        sha, _, path = l.partition(" ")
        if path and path.endswith(EXT):
            seen.setdefault(sha, path)
    print(f"{len(seen)} distinct source blobs in the history of {repo}", file=sys.stderr)
    batch = subprocess.Popen(
        ["git", "-C", repo, "cat-file", "--batch"], stdin=subprocess.PIPE, stdout=subprocess.PIPE
    )
    results = []
    for sha, path in seen.items():
        batch.stdin.write((sha + "\n").encode())
        batch.stdin.flush()
        hdr = batch.stdout.readline().split()
        size = int(hdr[2])
        data = batch.stdout.read(size)
        batch.stdout.read(1)
        if size > 3_000_000 or b"\0" in data[:4096]:
            continue
        text = data.decode("utf-8", "replace")
        if with_phrases:
            phrase_report(path, text, idx, sha[:10] + " ")
            continue
        r = report(path, text, corpora, 0.02)
        if r and r[1]:
            results.append((path, sha[:10], r))
    for path, sha, (n, out) in sorted(results, key=lambda x: -max(o[1] for o in x[2][1])):
        for name, share, run, cnt in sorted(out, key=lambda x: -x[1]):
            print(f"{path:48s} {sha}  {n:7d} tok  {name:12s} {100 * share:5.1f}%  run {run:4d}")


BOILER = re.compile(
    r"warrant|merchantab|hope that|useful but|liabil|redistribut|permission|copyright|licen[cs]e|software|disclaim|endorse|conditions|damages|contributors|notice|gnu|free software|fitness|possibility|neither the name|advised|noninfring|in no event|without modification|binary forms|reproduce|particular purpose"
)


def comment_phrases(text, n=5):
    """the set of n-word phrases that occur inside the comments of a source text (licence boilerplate and repetitive phrases left out)"""
    out = set()
    for m in re.finditer(
        r"//\s*([^\n]+)|/\*+\s*(.*?)\s*\*+/|^[ \t]*[@;]\s*([^\n]+)|#\s+([^\n]+)", text, re.S | re.M
    ):
        s = (
            re.sub(r"[\s*]+", " ", (m.group(1) or m.group(2) or m.group(3) or m.group(4) or ""))
            .strip()
            .lower()
        )
        w = re.findall(r"[a-z0-9_]+", s)
        for k in range(len(w) - n + 1):
            p = " ".join(w[k : k + n])
            if not BOILER.search(p) and len(set(w[k : k + n])) > 2:
                out.add(p)
    return out


def phrase_index():
    import json, pickle

    reg = os.path.join(CORP, "corpora.json")
    cache = os.path.join(CORP, "phrases.pkl")
    if os.path.exists(cache) and os.path.getmtime(cache) >= os.path.getmtime(reg):
        return pickle.load(open(cache, "rb"))
    idx = {}
    for name, dirs in json.load(open(reg)).items():
        ix = {}
        for p in source_files(dirs):
            try:
                t = open(p, encoding="utf-8", errors="replace").read()
            except OSError:
                continue
            for ph in comment_phrases(t):
                ix.setdefault(ph, os.path.relpath(p, dirs[0]))
        idx[name] = ix
    pickle.dump(idx, open(cache, "wb"))
    return idx


def phrase_report(label, text, idx, extra=""):
    ph = comment_phrases(text)
    if len(ph) < 5:
        return
    for name, ix in idx.items():
        hit = sorted(x for x in ph if x in ix)
        if len(hit) >= 4:
            src = max(
                sorted({ix[x] for x in hit}), key=lambda s: sum(1 for x in hit if ix[x] == s)
            )  # sorted: the same file on every run
            print(
                f"{len(hit):3d}/{len(ph):4d} {label:46s} {extra}{name:11s} {src[-44:]:44s} e.g. '{hit[0][:50]}'"
            )


def cmd_phrases(listfile):
    idx = phrase_index()
    paths = [l.rstrip("\n") for l in (sys.stdin if listfile == "-" else open(listfile))]
    for p in paths:
        if not p.endswith(EXT) or not os.path.isfile(p):
            continue
        phrase_report(p, open(p, encoding="utf-8", errors="replace").read(), idx)


if __name__ == "__main__":
    c = sys.argv[1]
    if c == "index":
        cmd_index(sys.argv[2], sys.argv[3:])
    elif c == "scan":
        cmd_scan(sys.argv[2])
    elif c == "blobs":
        cmd_blobs("--phrases" in sys.argv[2:])
    elif c == "phrases":
        cmd_phrases(sys.argv[2])


def cmd_show(path, corpus, top=4):
    corpora = load_corpora()
    arr = corpora[corpus]
    text = open(path, encoding="utf-8", errors="replace").read() if os.path.exists(path) else sys.stdin.read()
    toks = tokens(text)
    m = len(toks) - K + 1
    ids = np.fromiter(
        (_ids[t] if t in _ids else _ids.setdefault(t, zlib.crc32(t.encode()) | 1) for t in toks),
        dtype=np.uint64,
        count=len(toks),
    )
    h = ids[:m].copy()
    for j in range(1, K):
        h = h * P + ids[j : j + m]
    pos = np.searchsorted(arr, h)
    pos[pos == len(arr)] = 0
    hit = (arr[pos] == h) & ~dataish_mask(toks)
    runs, i = [], 0
    while i < m:
        if hit[i]:
            j = i
            while j < m and hit[j]:
                j += 1
            runs.append((i, j + K - 1))
            i = j
        else:
            i += 1
    runs.sort(key=lambda r: r[0] - r[1])
    print(f"{path} vs {corpus}: {sum(1 for r in runs)} runs, {int(hit.sum())} shingles of {m}")
    for a, b in runs[:top]:
        print(f"  [{b - a} tokens] " + " ".join(toks[a:b])[:700])


if len(sys.argv) > 1 and sys.argv[1] == "show":
    cmd_show(sys.argv[2], sys.argv[3], int(sys.argv[4]) if len(sys.argv) > 4 else 4)
