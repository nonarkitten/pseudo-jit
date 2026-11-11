#!/usr/bin/env python3
"""Do the documents, comments and scripts of the tree point at things that exist?

    python3 tools/audit/refs.py

Reads every file that git knows (tracked, or untracked and not ignored) except docs/HISTORY.md, which is a log and names the paths of earlier days, and reports

  link   a Markdown link whose target is neither a file nor a directory (anchors are not checked);
  path   a path inside the repository (pjit/..., docs/..., tools/... and so on) that does not exist and is not a git-ignored file (a generated header, local/);
  doc    a Markdown file named in a document, a comment or a script that does not exist anywhere in the tree.

Paths with wildcards or placeholders (`tests/qemu/smoke_*.c`, `build/<variant>/`) and anything under build/ are not checked. The exit status is 1 when something is reported.
"""

import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
TOP = (
    "bootloader",
    "docs",
    "golden",
    "mk",
    "pjit",
    "platform",
    "test_progs",
    "tests",
    "tools",
    "LICENSES",
)
LOG = {"docs/HISTORY.md"}
BINARY = (
    ".pdf",
    ".zip",
    ".png",
    ".jpg",
    ".gif",
    ".bin",
    ".lzt",
    ".hex",
    ".gp6",
    ".rom",
    ".o",
    ".elf",
    ".adf",
    ".npy",
    ".svg",
)
GENERATED_DOCS = {"Home.md", "_Sidebar.md", "_Footer.md"}  # written by tools/wiki.py

LINK = re.compile(r"\[[^\]]*\]\(([^)\s]+)(?:\s+\"[^\"]*\")?\)")
PATH = re.compile(r"(?<![\w/.-])((?:%s)/[\w./+-]*[\w])(?=(.{0,2}))" % "|".join(TOP))
DOC = re.compile(r"(?<![\w/.-])([A-Za-z0-9_-]+\.md)(?![\w]|\.\w)")
END = set(" \t\n\r\"'`)]},;:|>!?")
UPSTREAM = (
    "bootloader/src/armv7a/",
)  # paths inside Texas Instruments' StarterWare, named where a file is credited to it


def ends(after):
    """Does the path end where the text goes on with `after`, rather than carry on into a wildcard or a placeholder?"""
    if after == "" or after[0] in END:
        return True
    return after[0] == "." and (len(after) == 1 or after[1] in END)


def git(*args):
    return subprocess.run(["git", "-C", ROOT, *args], capture_output=True, text=True).stdout


def tree():
    names = set(git("ls-files", "-z", "-c", "-o", "--exclude-standard").split("\0"))
    names -= set(git("ls-files", "-z", "-d").split("\0")) | {""}
    return sorted(f for f in names if os.path.isfile(os.path.join(ROOT, f)))


def main():
    files = tree()
    fileset = set(files)
    dirs = {os.path.dirname(f) for f in files}
    for d in list(dirs):
        while d:
            dirs.add(d)
            d = os.path.dirname(d)
    docs = {os.path.basename(f) for f in files if f.endswith(".md")} | GENERATED_DOCS
    ignored = {}

    def exists(p):
        if p in fileset or p in dirs or os.path.exists(os.path.join(ROOT, p)):
            return True
        if p not in ignored:
            ignored[p] = subprocess.run(["git", "-C", ROOT, "check-ignore", "-q", "--", p]).returncode == 0
        return ignored[p]

    found = []
    for f in files:
        if f in LOG or f.endswith(BINARY):
            continue
        try:
            lines = open(os.path.join(ROOT, f), encoding="utf-8").read().splitlines()
        except (UnicodeDecodeError, OSError):
            continue
        fenced = False
        for n, line in enumerate(lines, 1):
            if f.endswith(".md"):
                if line.lstrip().startswith("```"):
                    fenced = not fenced
                if not fenced:
                    for m in LINK.finditer(line):
                        t = m.group(1)
                        if re.match(r"(https?:|mailto:|#|<)", t):
                            continue
                        t = t.split("#")[0]
                        if t and not exists(os.path.normpath(os.path.join(os.path.dirname(f), t))):
                            found.append((f, n, "link", t))
            for m in PATH.finditer(line):
                p, after = m.group(1), m.group(2)
                if p.startswith(UPSTREAM) or not ends(after):
                    continue
                if not exists(p):
                    found.append((f, n, "path", p))
            for m in DOC.finditer(line):
                if m.group(1) not in docs:
                    found.append((f, n, "doc", m.group(1)))
    for f, n, kind, what in found:
        print("%s:%d: %-4s %s" % (f, n, kind, what))
    print("%d unresolved reference%s in %d files" % (len(found), "" if len(found) == 1 else "s", len(files)))
    return 1 if found else 0


if __name__ == "__main__":
    sys.exit(main())
