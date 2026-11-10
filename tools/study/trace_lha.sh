#!/bin/bash
# Regenerates the LhA workload traces (compress and decompress of a text file) used to study block formation on compiled, loop-heavy code.
#   tools/study/trace_lha.sh <work dir> [path to lha_68k.lha, default ~/Downloads/lha_68k.lha] [instructions to record, default 4000000]
# Builds a throwaway virtualenv (amitools + machine68k 0.3.0 + lhafile) in <work dir>/ami, extracts the 68000 build of LhA from the archive, and runs it under vamos
# with tools/study/amitrace.py recording every instruction: <work dir>/lha_a.{trace,img,base} (lha a: compress LhA.guide) and <work dir>/x/lha_x.* (lha x: decompress it).
# LhA opens GUI libraries it never uses in command-line mode; vamos has no graphics / gadtools / ..., so they are given as fake libraries (.vamosrc).
# Then, e.g.:  .venv/bin/python tools/study/blockdyn.py <work dir>/lha_a.trace <work dir>/lha_a.img $(cat <work dir>/lha_a.base) --be --follow --tree --cap 480
set -e
W="$(mkdir -p "$1" && cd "$1" && pwd)"; ARC="${2:-$HOME/Downloads/lha_68k.lha}"; N="${3:-4000000}"; HERE="$(cd "$(dirname "$0")" && pwd)"
cd "$W"; PY="$W/ami/bin/python3"
if [ ! -x ami/bin/vamos ]; then
  /usr/bin/python3 -m venv ami
  ami/bin/pip install --quiet amitools "machine68k==0.3.0" lhafile
fi
cp "$ARC" lha_68k.lha
"$PY" -I - <<'PY'
import lhafile
a = lhafile.Lhafile('lha_68k.lha')
for i in a.infolist():
    if i.filename in ('lha_68k', 'LhA.guide'): open(i.filename, 'wb').write(a.read(i.filename))
PY
{ for l in graphics gadtools asl workbench icon diskfont iffparse; do printf '[%s.library]\nmode=fake\nversion=40\nnum_fake_funcs=150\n' "$l"; done; } > .vamosrc
rm -f t.lzh
"$PY" -I "$HERE/amitrace.py" lha_a "$N" -- -C 68000 lha_68k a t.lzh LhA.guide
mkdir -p x && cp t.lzh .vamosrc lha_68k x/ && cd x
"$PY" -I "$HERE/amitrace.py" lha_x "$N" -- -C 68000 lha_68k x t.lzh
cmp LhA.guide ../LhA.guide && echo "round trip ok: the decompressed file equals the original"
