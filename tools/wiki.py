#!/usr/bin/env python3
"""Builds a GitHub wiki from the documentation in this repository.

    python3 tools/wiki.py [--out build/wiki] [--repo https://github.com/OWNER/REPO] [--branch main]

A GitHub wiki is a flat git repository (<repo>.wiki.git) of Markdown pages; links between pages are page names, folders are not
navigable. This reads the pages listed in PAGES below, renames them, rewrites every relative link (to another page: a wiki link;
to a source file or directory: a link into the repository on GitHub), and writes `Home.md`, `_Sidebar.md` and `_Footer.md` into
--out. docs/wiki.md says how to push the result.
"""

import argparse
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))

# (wiki page name, source file, section of the sidebar)
PAGES = [
    ("Home", "docs/README.md", None),
    ("Getting-Started", "docs/getting-started.md", "Use"),
    ("Building-and-Testing", "docs/building.md", "Use"),
    ("Repository-Map", "docs/repo-map.md", "Use"),
    ("Tools", "docs/tools.md", "Use"),
    ("Tests", "tests/README.md", "Use"),
    ("Third-Party-Material", "docs/third-party.md", "Use"),
    ("How-PJIT-Works", "docs/design/OVERVIEW.md", "How it works"),
    ("Tiered-Execution", "docs/design/TIERING.md", "How it works"),
    ("Blocks", "docs/design/BLOCK_RUNTIME.md", "How it works"),
    ("Table-Generator", "tools/gen/README.md", "How it works"),
    ("Cortex-A8-Costs", "docs/design/CORTEX_A8.md", "How it works"),
    ("Bus-Interface", "docs/buffee/BUS_INTERFACE.md", "Buffee board"),
    ("Boot-Sequence", "docs/buffee/BOOT_SEQUENCE.md", "Buffee board"),
    ("Memory-Map-and-Bundles", "docs/buffee/BUFFEE_MAP.md", "Buffee board"),
    ("Bus-Arbitration", "docs/buffee/BUS_ARBITRATION.md", "Buffee board"),
    ("Settings-from-AmigaOS", "docs/buffee/BUFFEE_CLI.md", "Buffee board"),
    ("Bootloader", "bootloader/README.md", "Buffee board"),
    ("BeagleBone", "platform/beaglebone/README.md", "Buffee board"),
    ("Golden-Oracle", "golden/README.md", "Tests"),
    ("History", "docs/HISTORY.md", "Progress"),
    ("Plan", "docs/PLAN.md", "Progress"),
]
BY_PATH = {src: name for name, src, _ in PAGES}


def repo_url(arg):
    if arg:
        return arg.rstrip("/")
    try:
        url = subprocess.check_output(["git", "-C", ROOT, "remote", "get-url", "origin"], text=True).strip()
    except Exception:
        return "https://github.com/OWNER/REPO"
    m = re.match(r"(?:git@github\.com:|https://github\.com/)(.+?)(?:\.git)?$", url)
    return "https://github.com/" + m.group(1) if m else url


def rewrite_links(text, src, repo, branch):
    here = os.path.dirname(src)

    def fix(m):
        label, target = m.group(1), m.group(2)
        if re.match(r"^(https?:|mailto:|#)", target):
            return m.group(0)
        path, _, frag = target.partition("#")
        frag = "#" + frag if frag else ""
        full = os.path.normpath(os.path.join(here, path))
        if full in BY_PATH:
            return "[%s](%s%s)" % (label, BY_PATH[full], frag)
        disk = os.path.join(ROOT, full)
        if os.path.isdir(disk):
            return "[%s](%s/tree/%s/%s)" % (label, repo, branch, full)
        if os.path.exists(disk):
            return "[%s](%s/blob/%s/%s)" % (label, repo, branch, full)
        sys.stderr.write("wiki.py: %s links to %s, which does not exist\n" % (src, target))
        return m.group(0)

    return re.sub(r"\[([^\]]*)\]\(([^)\s]+)\)", fix, text)


def main():
    ap = argparse.ArgumentParser(description=__doc__.split("\n")[1])
    ap.add_argument("--out", default=os.path.join(ROOT, "build", "wiki"))
    ap.add_argument("--repo", help="GitHub URL of the repository (default: the origin remote)")
    ap.add_argument("--branch", default="main")
    a = ap.parse_args()
    repo = repo_url(a.repo)
    os.makedirs(a.out, exist_ok=True)
    for name, src, _ in PAGES:
        text = open(os.path.join(ROOT, src), encoding="utf-8").read()
        text = rewrite_links(text, src, repo, a.branch)
        text += "\n\n---\n*This page is generated from [`%s`](%s/blob/%s/%s); edit it there.*\n" % (
            src,
            repo,
            a.branch,
            src,
        )
        open(os.path.join(a.out, name + ".md"), "w", encoding="utf-8").write(text)
    sections = []
    for name, _, sec in PAGES:
        if sec and sec not in sections:
            sections.append(sec)
    side = ["**[Home](Home)**\n"]
    for sec in sections:
        side.append("**%s**\n" % sec)
        side += ["* [%s](%s)" % (n.replace("-", " "), n) for n, _, s in PAGES if s == sec]
        side.append("")
    open(os.path.join(a.out, "_Sidebar.md"), "w").write("\n".join(side))
    open(os.path.join(a.out, "_Footer.md"), "w").write("[Source repository](%s)\n" % repo)
    print("wrote %d pages and the sidebar to %s" % (len(PAGES), a.out))


if __name__ == "__main__":
    main()
