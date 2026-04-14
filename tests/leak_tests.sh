#!/bin/bash
# =============================================================
#  Minishell — Memory Leak Tests (valgrind)
#  Usage : ./tests/leak_tests.sh [./minishell]
# =============================================================

MINISHELL="${1:-./minishell}"
PASS=0
FAIL=0

RED='\033[0;31m'
GRN='\033[0;32m'
YLW='\033[0;33m'
BOLD='\033[1m'
RST='\033[0m'

# Valgrind flags: suppress readline internals, check everything else
VFLAGS="--leak-check=full \
        --track-origins=yes \
        --error-exitcode=42 \
        --suppressions=tests/readline.supp \
        --quiet"

# Generate readline suppression file (readline has known internal leaks)
generate_supp() {
    cat > tests/readline.supp << 'SUPP'
{
   readline_internal
   Memcheck:Leak
   ...
   obj:*/libreadline*
}
{
   readline_history
   Memcheck:Leak
   ...
   fun:add_history
}
{
   readline_init
   Memcheck:Leak
   ...
   fun:readline
}
SUPP
}

run_leak() {
    local desc="$1"
    local input="$2"

    result=$(printf '%s\nexit\n' "$input" \
        | valgrind $VFLAGS "$MINISHELL" 2>&1)
    vg_exit=$?

    if [ "$vg_exit" = "0" ] || [ "$vg_exit" = "1" ]; then
        PASS=$((PASS + 1))
        printf "  ${GRN}✓${RST}  no leaks: %s\n" "$desc"
    else
        FAIL=$((FAIL + 1))
        printf "  ${RED}✗${RST}  LEAK: %s\n" "$desc"
        printf '%s\n' "$result" | grep -E "LEAK|ERROR|Invalid" | head -5
    fi
}

if ! command -v valgrind &>/dev/null; then
    printf "${YLW}valgrind not found — skipping leak tests${RST}\n"
    exit 0
fi

printf "${BOLD}Memory Leak Tests${RST}  →  %s\n\n" "$MINISHELL"
generate_supp

run_leak "echo basic"           "echo hello"
run_leak "export + unset"       "export X=42 && unset X"
run_leak "pipe"                 "echo a | cat"
run_leak "heredoc"              "cat << E
test
E"
run_leak "subshell"             "(echo sub)"
run_leak "redirect >"           "echo x > /tmp/msh_leak_test && rm /tmp/msh_leak_test"
run_leak "wildcard"             "ls *.c 2>/dev/null"
run_leak "and-or chain"         "true && false || echo ok"
run_leak "100 exports"          "$(for i in $(seq 1 100); do printf 'export V%d=%d && ' $i $i; done)echo done"
run_leak "cd chain"             "cd /tmp && cd /var && cd -"

printf "\n${BOLD}Results: ${GRN}%d clean${RST}  ${RED}%d leaked${RST}\n" "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ] && exit 0 || exit 1
