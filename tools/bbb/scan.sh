#!/bin/zsh
# Warm Dhrystone and Tiny BASIC numbers on the BeagleBone over several code layouts (timings move 5-10% with alignment: never judge a change from one build).
#   tools/bbb/scan.sh <label> "<assembler switches>" "<C switches>" <pad words...>
# e.g.  tools/bbb/scan.sh tiered "" "" 0 160 400 1000
#       tools/bbb/scan.sh T=64 "-Wa,--defsym,PJIT_TIER_T=64" "" 0 160       (the threshold; or make TIER_T=64 via MAKEX)
#       tools/bbb/scan.sh arena=64K "" "-DBLK_RING_KB=64" 0 160             (the block arena)
# MAKEX="PIN_D0=1" passes more variables to make.  Every pad is built at once in its own variant build/$VARIANT/p<pad> ($VARIANT default: scan; JOBS at a time,
# default all CPUs); only the runs on the board are one after the other.
# Prints "<label> dhry pad N: <cycles per run> <mispredicts>" and "<label> TB pad N: <cycles>".
setopt null_glob
cd "$(dirname "$0")/../.."
L=$1; AX=$2; CX=$3; shift 3
V=${VARIANT:-scan}
J=${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || echo 4)}
export V AX CX MAKEX
build_one() {
  n=$1; mkdir -p "build/$V/p$n"; rm -f "build/$V/p$n/fail"
  make ${=MAKEX} VARIANT="$V/p$n" BLK_CEXTRA="$CX" BBB_ASEXTRA="$AX -Wa,--defsym,PJIT_PAD=$n" bbb-bbb_dhrystone bbb-bbb_tinybasic >/dev/null 2>&1 || touch "build/$V/p$n/fail"
}
printf '%s\n' "$@" | xargs -P "$J" -L1 zsh -c "$(functions build_one); build_one \$0"
for n in "$@"; do
  B2="build/$V/p$n"
  [ -f "$B2/fail" ] && { echo "$L pad $n: build failed"; continue; }
  for t in dhrystone tinybasic; do
    r=""
    for try in 1 2 3; do                                  # the board is flaky about booting: retry an empty result
      out=$(python3 -u tools/bbb/run.py "$B2"/bbb/bbb_$t.bin 2>&1)
      if [ $t = dhrystone ]; then r=$(echo "$out" | sed -n 's/.*WARM\] cycles per run, translated code kept: \([0-9]*\) \([0-9]*\) \([0-9]*\).*mispredicts \([0-9]*\).*/\1 \2 \3 (warm runs 1-3), mispredicts \4/p' | head -1)
      else r=$(echo "$out" | grep -E 'WARM\] cached run 1' | awk '{print $5}'); fi
      [ -n "$r" ] && break
    done
    if [ $t = dhrystone ]; then echo "$L dhry pad $n: $r"; else echo "$L TB pad $n: $r"; fi
  done
done
