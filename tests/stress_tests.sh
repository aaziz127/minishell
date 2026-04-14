#!/bin/bash
# =============================================================
#  Minishell — Stress & Robustness Tests
#  Tests limits, concurrent processes, large data, rapid fire
#  Usage : ./tests/stress_tests.sh [./minishell]
# =============================================================

MINISHELL="${1:-./minishell}"
PASS=0
FAIL=0

RED='\033[0;31m'
GRN='\033[0;32m'
YLW='\033[0;33m'
BOLD='\033[1m'
RST='\033[0m'

ok()   { PASS=$((PASS+1)); printf "  ${GRN}✓${RST}  %s\n" "$1"; }
fail() { FAIL=$((FAIL+1)); printf "  ${RED}✗${RST}  %s  →  %s\n" "$1" "$2"; }

run() {
    local desc="$1"; local cmd="$2"
    local exp_out="$3"; local exp_exit="${4:-0}"
    local actual_out actual_exit

    actual_out=$(printf '%s\n' "$cmd" | "$MINISHELL" 2>/dev/null)
    actual_exit=$?
    actual_out=$(printf '%s' "$actual_out" | sed 's/[[:space:]]*$//')
    exp_out=$(printf '%s' "$exp_out" | sed 's/[[:space:]]*$//')

    if [ "$actual_out" = "$exp_out" ] && [ "$actual_exit" = "$exp_exit" ]; then
        ok "$desc"
    else
        fail "$desc" "out='$actual_out' exit=$actual_exit (want out='$exp_out' exit=$exp_exit)"
    fi
}

section() { printf "\n${BOLD}── %s ──${RST}\n" "$1"; }

# ─── 1. PIPELINE DEPTH ───────────────────────────────────────
section "Pipeline depth"

run "pipe 5"  "echo x|cat|cat|cat|cat|cat"          "x"
run "pipe 10" "echo x|cat|cat|cat|cat|cat|cat|cat|cat|cat|cat" "x"
run "pipe 20" "echo depth$(printf '|cat%.0s' {1..20})" "depth"

# ─── 2. ARGUMENT COUNT ───────────────────────────────────────
section "Argument count"

ARGS100=$(seq 1 100 | tr '\n' ' ')
run "100 args to echo wc" \
    "echo $ARGS100 | wc -w" "     100"

ARGS1000=$(seq 1 1000 | tr '\n' ' ')
run "1000 args - last is 1000" \
    "echo $ARGS1000 | awk '{print \$NF}'" "1000"

# ─── 3. VARIABLE CHAINING ────────────────────────────────────
section "Variable chains"

run "export && echo chain 3" \
    "export A=1 && export B=2 && export C=3 && echo \$A\$B\$C" "123"

# Build a chain of 50 exports
CMD50=$(for i in $(seq 1 50); do printf 'export V%d=%d && ' $i $i; done)
run "50 exports chain last" \
    "${CMD50}echo \$V50" "50"

# ─── 4. HEREDOC STRESS ───────────────────────────────────────
section "Heredoc stress"

run "heredoc 10 lines" \
    "cat << E
$(for i in $(seq 1 10); do echo "line$i"; done)
E" "$(for i in $(seq 1 10); do echo "line$i"; done)"

run "heredoc in pipeline" \
    "cat << E | wc -l
$(for i in $(seq 1 5); do echo "x"; done)
E" "       5"

run "heredoc then another cmd" \
    "cat << E
hello
E
echo world" "$(printf 'hello\nworld')"

# ─── 5. REDIRECT STRESS ──────────────────────────────────────
section "Redirect stress"

TMP=$(mktemp -d)

run "10 appends then wc" \
    "$(for i in $(seq 1 10); do printf 'echo line >> %s/f && ' $i $TMP/f; done)wc -l < $TMP/f" \
    "      10"

run "redirect chain different files" \
    "echo a > $TMP/1 && echo b > $TMP/2 && cat $TMP/1 $TMP/2" \
    "$(printf 'a\nb')"

rm -rf "$TMP"

# ─── 6. SUBSHELL NESTING ─────────────────────────────────────
section "Subshell nesting"

run "nested 2 deep"  "((echo deep))"                    "deep"
run "nested 3 deep"  "(((echo deeper)))"                "deeper"
run "subshell pipe"  "(echo a && echo b) | wc -l"       "       2"
run "subshell iso"   "export X=out && (export X=in) && echo \$X" "out"
run "subshell exit"  "(exit 5); echo \$?"               "5"

# ─── 7. WILDCARD STRESS ──────────────────────────────────────
section "Wildcard stress"

TMP2=$(mktemp -d)
for i in $(seq 1 20); do touch "$TMP2/file$i.c"; done
touch "$TMP2/main.h" "$TMP2/README.md"
cd "$TMP2" || exit 1

run "20 .c files glob" \
    "ls *.c | wc -l" "      20"
run "1 .h file glob"   \
    "ls *.h | wc -l" "       1"
run "glob then pipe"   \
    "ls *.c | grep file5" "file5.c"
run "no match literal" \
    "echo *.xyz" "*.xyz"
run "quoted no expand" \
    "echo '*.c'" "*.c"

cd - > /dev/null || true
rm -rf "$TMP2"

# ─── 8. SIGNAL RESISTANCE ────────────────────────────────────
section "Process stability"

# Minishell should not crash on these inputs
run "empty command"    ""       ""  0
run "only spaces"      "   "    ""  0
run "many semicolons"  "echo a ; echo b ; echo c" \
    "$(printf 'a\nb\nc')" 0
run "false exit code"  "false"  ""  1
run "true exit code"   "true"   ""  0

# ─── 9. MIXED STRESS ─────────────────────────────────────────
section "Mixed stress"

TMP3=$(mktemp -d)
touch "$TMP3/a.c" "$TMP3/b.c"
cd "$TMP3" || exit 1

run "glob + pipe + wc" \
    "ls *.c | wc -l" "       2"
run "export + glob" \
    "export DIR=. && ls \$DIR/*.c | wc -l" "       2"
run "redirect + glob" \
    "ls *.c > /tmp/msh_glob_out && wc -l < /tmp/msh_glob_out && rm /tmp/msh_glob_out" \
    "       2"
run "&& + export + echo" \
    "export MX=stress && echo \$MX | cat" "stress"

cd - > /dev/null || true
rm -rf "$TMP3"

# ─── 10. RAPID FIRE ──────────────────────────────────────────
section "Rapid fire commands"

run "20 echoes semicolon" \
    "$(for i in $(seq 1 20); do printf 'echo %d ; ' $i; done)echo done | tail -1" \
    "done"

run "10 pipes counting" \
    "$(printf 'echo'; for i in $(seq 1 9); do printf '|cat'; done) | wc -c" \
    "       1"

# ─── SUMMARY ─────────────────────────────────────────────────
printf "\n${BOLD}══════════════════════════${RST}\n"
printf "${BOLD}Stress: ${GRN}%d passed${RST}  ${RED}%d failed${RST}\n" "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ] && exit 0 || exit 1
