#!/usr/bin/env bash
# Runs the QEMU smoke tests that `make test` has built:      tests/run.sh <directory with the .elf files> <test name>...
#
# Each test (tests/qemu/*.c) is a bare-metal program that prints a report and ends with "PASS" or "FAIL" on QEMU's semihosting console.
# QEMU is the only way to run this code on an Apple-silicon or modern x86 Mac, neither of which has an AArch32 mode.
# The tests are independent: JOBS of them run at once (default: all CPUs), each into its own report; the reports are printed in order
# afterwards.  Tests named in TIMING_TESTS depend on the QEMU timer's real-time behaviour and run alone, after the others.
# Exits non-zero if any test failed.
set -u -o pipefail

ELFDIR="${1:?usage: tests/run.sh <elf directory> <test>...}"; shift
TESTS="$*"
QEMU="${QEMU:-qemu-system-arm}"
QEMU_ARGS="-M virt -cpu cortex-a15 -m 128M -nographic -semihosting"
JOBS="${JOBS:-$(sysctl -n hw.ncpu 2>/dev/null || nproc 2>/dev/null || echo 4)}"
TIMING="${TIMING_TESTS:-}"

run_test() {
    local name="$1" elf="$ELFDIR/$1.elf" log="$ELFDIR/$1.out.log"
    echo "== $name =="
    if [ ! -f "$elf" ]; then echo "  MISSING: $elf"; return 1; fi
    # the UART driver emits CRLF: strip the CR so patterns anchored with $ work; qemu is the head of a pipe, so ask PIPESTATUS for its code
    "$QEMU" $QEMU_ARGS -kernel "$elf" 2>&1 | tr -d '\r' > "$log"
    local qemu_rc=${PIPESTATUS[0]}
    sed 's/^/  /' "$log"
    if grep -q '^PASS$' "$log" && ! grep -qE '^FAIL$|MISMATCH' "$log"; then
        echo "  RESULT: PASS (qemu exit $qemu_rc)"; echo; return 0
    fi
    echo "  RESULT: FAIL (qemu exit $qemu_rc, no clean PASS marker found)"; echo; return 1
}
export ELFDIR QEMU QEMU_ARGS
export -f run_test

echo "== running under $("$QEMU" --version | head -n1) =="
echo
PAR=""
for t in $TESTS; do case " $TIMING " in *" $t "*) ;; *) PAR="$PAR $t";; esac; done
printf '%s\n' $PAR | xargs -P "$JOBS" -I{} bash -c 'run_test {} > "$ELFDIR/{}.report" 2>&1; echo $? > "$ELFDIR/{}.rc"'
for t in $TIMING; do
    case " $TESTS " in *" $t "*) bash -c "run_test $t" > "$ELFDIR/$t.report" 2>&1; echo $? > "$ELFDIR/$t.rc";; esac
done

pass=0; fail=0
for t in $TESTS; do
    cat "$ELFDIR/$t.report"
    if [ "$(cat "$ELFDIR/$t.rc" 2>/dev/null)" = 0 ]; then pass=$((pass + 1)); else fail=$((fail + 1)); fi
done
echo "== summary: $pass passed, $fail failed =="
[ "$fail" -eq 0 ]
