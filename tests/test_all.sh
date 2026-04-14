#!/bin/bash
# =============================================================
#  Minishell — Master Test Launcher
#  Usage : make test   OR   ./tests/test_all.sh [./minishell]
# =============================================================

MINISHELL="${1:-./minishell}"
SUITE_DIR="$(dirname "$0")"
TOTAL_PASS=0
TOTAL_FAIL=0

RED='\033[0;31m'
GRN='\033[0;32m'
YLW='\033[0;33m'
CYN='\033[0;36m'
BOLD='\033[1m'
RST='\033[0m'

if [ ! -x "$MINISHELL" ]; then
    printf "${RED}Error:${RST} %s not found or not executable\n" "$MINISHELL"
    printf "Run ${BOLD}make${RST} first.\n"
    exit 1
fi

printf "${BOLD}${CYN}"
printf "╔══════════════════════════════════════╗\n"
printf "║   Minishell — Full Test Suite        ║\n"
printf "╚══════════════════════════════════════╝\n"
printf "${RST}"
printf "  Binary : %s\n\n" "$MINISHELL"

run_suite() {
    local name="$1"
    local script="$SUITE_DIR/$2"

    printf "${BOLD}▶ %s${RST}\n" "$name"
    t_start=$(date +%s%3N)

    output=$("$script" "$MINISHELL" 2>&1)
    exit_code=$?
    t_end=$(date +%s%3N)
    elapsed=$((t_end - t_start))

    # Extract pass/fail counts from output
    p=$(printf '%s' "$output" | grep -c '✓' || true)
    f=$(printf '%s' "$output" | grep -c '✗' || true)
    TOTAL_PASS=$((TOTAL_PASS + p))
    TOTAL_FAIL=$((TOTAL_FAIL + f))

    if [ "$exit_code" -eq 0 ]; then
        printf "  ${GRN}%d passed${RST}  in %dms\n\n" "$p" "$elapsed"
    else
        printf "  ${GRN}%d passed${RST}  ${RED}%d failed${RST}  in %dms\n" "$p" "$f" "$elapsed"
        # Show failed test lines
        printf '%s\n' "$output" | grep '✗' | head -10
        printf "\n"
    fi
}

chmod +x "$SUITE_DIR"/*.sh

run_suite "Unit Tests (builtins, vars, pipes, redirs, logic)" run_tests.sh
run_suite "Bash Comparison"                                   bash_compare.sh
run_suite "Stress & Robustness"                               stress_tests.sh

# Leak check (optional - needs valgrind)
if command -v valgrind &>/dev/null; then
    run_suite "Memory Leaks (valgrind)"                       leak_tests.sh
else
    printf "${YLW}⚠ valgrind not found — skipping leak tests${RST}\n\n"
fi

# Final banner
printf "${BOLD}${CYN}"
printf "╔══════════════════════════════════════╗\n"
printf "║  TOTAL : "
if [ "$TOTAL_FAIL" -eq 0 ]; then
    printf "${GRN}%3d passed${CYN}  %3d failed       ║\n" "$TOTAL_PASS" "$TOTAL_FAIL"
else
    printf "${RED}%3d passed  %3d FAILED${CYN}       ║\n" "$TOTAL_PASS" "$TOTAL_FAIL"
fi
printf "╚══════════════════════════════════════╝\n"
printf "${RST}"

[ "$TOTAL_FAIL" -eq 0 ] && exit 0 || exit 1
