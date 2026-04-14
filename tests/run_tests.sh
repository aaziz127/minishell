#!/bin/bash
# =============================================================
#  Minishell — Test Runner
#  Usage : ./tests/run_tests.sh [category]
#  Categories: builtins pipes redirs vars logic wildcards errors stress
#  Without arg: run all categories
# =============================================================

MINISHELL="${1:-./minishell}"
# If first arg is a category name, use ./minishell
if [ -f "$1" ] || [ -x "$1" ]; then
    MINISHELL="$1"
    shift
fi
CATEGORY="${1:-all}"

PASS=0
FAIL=0
TOTAL=0
FAILED_TESTS=""

RED='\033[0;31m'
GRN='\033[0;32m'
YLW='\033[0;33m'
BLU='\033[0;34m'
CYN='\033[0;36m'
RST='\033[0m'
BOLD='\033[1m'

# ─── Core test function ──────────────────────────────────────
# run_test "description" "input_cmd" "expected_stdout" [expected_exit]
run_test() {
    local desc="$1"
    local input="$2"
    local expected_out="$3"
    local expected_exit="${4:-0}"
    TOTAL=$((TOTAL + 1))

    actual_out=$(printf '%s\n' "$input" | $MINISHELL 2>/dev/null)
    actual_exit=$?

    # Normalize trailing newlines for comparison
    actual_out=$(printf '%s' "$actual_out" | sed 's/[[:space:]]*$//')
    expected_out=$(printf '%s' "$expected_out" | sed 's/[[:space:]]*$//')

    if [ "$actual_out" = "$expected_out" ] && [ "$actual_exit" = "$expected_exit" ]; then
        PASS=$((PASS + 1))
        printf "  ${GRN}✓${RST}  %s\n" "$desc"
    else
        FAIL=$((FAIL + 1))
        FAILED_TESTS="$FAILED_TESTS\n  • $desc"
        printf "  ${RED}✗${RST}  %s\n" "$desc"
        if [ "$actual_out" != "$expected_out" ]; then
            printf "      ${YLW}expected stdout:${RST} %q\n" "$expected_out"
            printf "      ${YLW}actual   stdout:${RST} %q\n" "$actual_out"
        fi
        if [ "$actual_exit" != "$expected_exit" ]; then
            printf "      ${YLW}expected exit:${RST} %s\n" "$expected_exit"
            printf "      ${YLW}actual   exit:${RST} %s\n" "$actual_exit"
        fi
    fi
}

# run_test_stderr: only check exit code
run_test_exit() {
    local desc="$1"
    local input="$2"
    local expected_exit="$3"
    TOTAL=$((TOTAL + 1))

    actual_exit=$(printf '%s\n' "$input" | $MINISHELL 2>/dev/null; echo $?)
    actual_exit=$(printf '%s\n' "$input" | $MINISHELL 2>/dev/null; printf "%d" $?)

    if [ "$actual_exit" = "$expected_exit" ]; then
        PASS=$((PASS + 1))
        printf "  ${GRN}✓${RST}  %s\n" "$desc"
    else
        FAIL=$((FAIL + 1))
        FAILED_TESTS="$FAILED_TESTS\n  • $desc"
        printf "  ${RED}✗${RST}  %s  (expected exit %s, got %s)\n" \
               "$desc" "$expected_exit" "$actual_exit"
    fi
}

# ─── Section header ──────────────────────────────────────────
section() {
    printf "\n${BOLD}${BLU}══ %s ══${RST}\n" "$1"
}

# ─── TEST CATEGORIES ─────────────────────────────────────────

test_builtins() {
    section "BUILTINS"

    # echo
    run_test "echo basic"             "echo hello"             "hello"
    run_test "echo multiple args"     "echo foo bar baz"       "foo bar baz"
    run_test "echo -n no newline"     "echo -n hi"             "hi"
    run_test "echo empty"             "echo"                   ""
    run_test "echo -n -n"             "echo -n -n hello"       "hello"

    # pwd
    run_test "pwd returns path"       "pwd" "$(pwd)"

    # cd + pwd
    run_test "cd /tmp then pwd"       "cd /tmp && pwd"         "/tmp"
    run_test "cd .."                  "cd /tmp && cd .. && pwd" "/"
    run_test "cd with no arg"         "export HOME=/tmp && cd && pwd" "/tmp"
    run_test "cd -"                   "cd /tmp && cd /var && cd - && pwd" "/tmp"

    # env
    run_test "env contains PATH"      "env | grep -c PATH"     "1"

    # export + echo
    run_test "export simple"          "export X=hello && echo \$X" "hello"
    run_test "export overwrite"       "export X=a && export X=b && echo \$X" "b"
    run_test "export no value"        "export NOVAL && echo \${NOVAL:-empty}" "empty"

    # unset
    run_test "unset removes var"      "export Y=42 && unset Y && echo \${Y:-gone}" "gone"
    run_test "unset nonexistent"      "unset NOPE"             ""  0

    # exit codes
    run_test_exit "exit 0"            "exit 0"                 0
    run_test_exit "exit 42"           "exit 42"                42
    run_test_exit "exit 255"          "exit 255"               255
    run_test_exit "exit 256 wraps"    "exit 256"               0
    run_test_exit "exit 1"            "exit 1"                 1
}

test_variables() {
    section "VARIABLES"

    run_test "dollar HOME"            "echo \$HOME"            "$HOME"
    run_test "dollar USER"            "echo \$USER"            "$USER"
    run_test "dollar PATH non-empty"  "echo \$PATH" "$PATH"
    run_test "undefined var is empty" "echo \$UNDEFINED_VAR_XYZ" ""
    run_test "dollar question mark"   "true && echo \$?"       "0"
    run_test "dollar ? after false"   "false ; echo \$?"       "1"
    run_test "double quote expansion" "X=world && echo \"hello \$X\"" "hello world"
    run_test "single quote no expand" "X=world && echo 'hello \$X'"  'hello $X'
    run_test "mixed quotes"           "X=42 && echo \"\$X\" '\$X'" "42 \$X"
    run_test "var in middle of word"  "X=foo && echo bar\${X}baz" "barfoobaz"
    run_test "empty double quote"     "echo \"\""              ""
    run_test "dollar in dquote"       "A=x && echo \"=\$A=\""  "=x="
}

test_pipes() {
    section "PIPES"

    run_test "simple pipe"            "echo hello | cat"       "hello"
    run_test "pipe to wc"             "echo -n hello | wc -c"  "       5"
    run_test "triple pipe"            "echo foo | cat | cat"   "foo"
    run_test "pipe count lines"       "printf 'a\nb\nc\n' | wc -l" "       3"
    run_test "pipe grep"              "printf 'foo\nbar\nbaz\n' | grep ba" "$(printf 'bar\nbaz')"
    run_test "pipe chain 4"           "echo test | cat | cat | cat" "test"
    run_test "ls pipe grep .c"        "ls *.c 2>/dev/null | head -1 | grep -c '.c'" "1"
    run_test_exit "pipe exit is last" "true | false"           1
    run_test_exit "pipe exit last 2"  "false | true"           0
}

test_redirections() {
    section "REDIRECTIONS"

    TMP=$(mktemp -d)

    # output redirect
    run_test "redirect >"  \
        "echo hello > $TMP/out && cat $TMP/out"  "hello"
    # append
    run_test "redirect >>" \
        "echo a > $TMP/app && echo b >> $TMP/app && cat $TMP/app" \
        "$(printf 'a\nb')"
    # input redirect
    echo "from file" > "$TMP/inp"
    run_test "redirect <"  \
        "cat < $TMP/inp"  "from file"
    # heredoc
    run_test "heredoc <<" \
        "cat << DELIM
hello heredoc
DELIM"  "hello heredoc"
    # heredoc no expansion inside single-like
    run_test "heredoc content"  \
        "cat << END
line1
line2
END"  "$(printf 'line1\nline2')"
    # redirect stdout of builtin
    run_test "echo > file" \
        "echo saved > $TMP/sv && cat $TMP/sv"  "saved"
    # truncate on >
    run_test "truncate >"  \
        "echo long > $TMP/tr && echo x > $TMP/tr && cat $TMP/tr"  "x"

    rm -rf "$TMP"
}

test_logic() {
    section "LOGICAL OPERATORS && ||"

    run_test "&& true true"     "echo a && echo b"         "$(printf 'a\nb')"
    run_test "&& false skip"    "false && echo b"          ""
    run_test "|| false exec"    "false || echo b"          "b"
    run_test "|| true skip"     "true || echo b"           ""
    run_test "chain &&&&"       "echo 1 && echo 2 && echo 3" "$(printf '1\n2\n3')"
    run_test "mix && ||"        "false && echo x || echo y"  "y"
    run_test "mix || &&"        "true || echo x && echo y"   "y"
    run_test "; separator"      "echo a ; echo b"          "$(printf 'a\nb')"
    run_test "; after fail"     "false ; echo ok"          "ok"

    # export && echo — THE BUG THAT WAS FIXED
    run_test "export && echo (lazy expand)" \
        "export LAZYVAR=fixed && echo \$LAZYVAR"  "fixed"
    run_test "export chain 3"   \
        "export A=1 && export B=2 && echo \$A\$B"  "12"
    run_test "unset && echo"    \
        "export DEL=yes && unset DEL && echo \${DEL:-gone}"  "gone"

    # subshell
    run_test "subshell basic"   "(echo inside)"            "inside"
    run_test "subshell isolate" \
        "export ISO=outer && (export ISO=inner) && echo \$ISO" "outer"
    run_test "subshell pipe"    \
        "(echo sub1 && echo sub2) | wc -l"  "       2"
}

test_wildcards() {
    section "WILDCARDS *"

    TMP=$(mktemp -d)
    touch "$TMP/file.c" "$TMP/main.c" "$TMP/test.h" "$TMP/README.md"
    cd "$TMP" || return

    # Count .c files
    run_test "glob *.c count" \
        "ls *.c | wc -l"  "       2"
    # Count .h files
    run_test "glob *.h count" \
        "ls *.h | wc -l"  "       1"
    # No match → literal
    run_test "glob no match literal" \
        "echo *.xyz"  "*.xyz"
    # * alone matches all visible
    run_test "glob * count" \
        "ls * | wc -l"  "       4"
    # single-quoted star is NOT globbed
    run_test "quoted star is literal" \
        "echo '*.c'"  "*.c"
    # double-quoted star is NOT globbed
    run_test "dquoted star is literal" \
        "echo \"*.c\""  "*.c"

    cd - > /dev/null || true
    rm -rf "$TMP"
}

test_errors() {
    section "ERROR HANDLING"

    run_test_exit "command not found"    "notacommand_xyz"           127
    run_test_exit "permission denied"    "chmod -x /tmp && /tmp"     126
    run_test_exit "bad redirect file"    "cat < /no/such/file"       1
    run_test "syntax error pipe end"     "|"           ""  2
    run_test "syntax error empty &&"     "&& echo x"   ""  2
    run_test "exit too many args"        "exit 1 2"    "exit" 1
    run_test_exit "exit non numeric"     "exit abc"    255
    run_test "cd bad dir"               \
        "cd /no/such/dir/xyz 2>&1 | head -1 | grep -c minishell"  "1"
}

test_stress() {
    section "STRESS TESTS"

    # Long pipeline
    run_test "pipeline 10 cats" \
        "echo stress | cat|cat|cat|cat|cat|cat|cat|cat|cat|cat" \
        "stress"

    # Many exports
    run_test "100 exports" \
        "$(for i in $(seq 1 100); do printf 'export V%d=%d && ' $i $i; done)echo \$V100" \
        "100"

    # Large echo
    run_test "echo 500 chars" \
        "python3 -c \"print('x'*500)\" | wc -c" \
        "     501"

    # Heredoc then command
    run_test "heredoc + pipe" \
        "cat << E | wc -l
line1
line2
line3
E" "       3"

    # Nested subshells
    run_test "nested subshells" \
        "((echo deep))" \
        "deep"

    # Rapid fire builtins
    run_test "20 echos &&" \
        "$(for i in $(seq 1 20); do printf 'echo %d && ' $i; done)echo done | tail -1" \
        "done"

    # Pipe from subshell
    run_test "subshell pipe wc" \
        "(echo a && echo b && echo c) | wc -l" \
        "       3"

    # Variable expansion chain
    run_test "var chain expand" \
        "export A=hello && export B=\$A && echo \$B" \
        "hello"

    # Redirect in pipeline
    TMP=$(mktemp)
    run_test "redirect in pipe" \
        "echo piped > $TMP && cat $TMP | cat" \
        "piped"
    rm -f "$TMP"

    # Long args list
    run_test "100 args to echo" \
        "echo $(seq 1 100 | tr '\n' ' ') | wc -w" \
        "     100"

    # Wildcard in pipeline
    TMP2=$(mktemp -d)
    touch "$TMP2/a.c" "$TMP2/b.c" "$TMP2/c.c"
    cd "$TMP2" || true
    run_test "glob in pipe" \
        "ls *.c | wc -l" \
        "       3"
    cd - > /dev/null || true
    rm -rf "$TMP2"
}

test_edge_cases() {
    section "EDGE CASES"

    run_test "empty input"          ""                          ""
    run_test "whitespace only"      "   "                       ""
    run_test "multiple semicolons"  "echo a ; ; echo b" ""  2
    run_test "semicolon only"       ";"                 ""  2
    run_test "echo with spaces"     "echo   a   b"      "a b"
    run_test "single quote empty"   "echo ''"           ""
    run_test "double quote empty"   "echo \"\""         ""
    run_test "backslash in dquote"  "echo \"a\\\\b\""   'a\b'
    run_test "dollar at end"        "echo end\$"        'end$'
    run_test "var with underscore"  "export _MY_V=ok && echo \$_MY_V" "ok"
    run_test "redirect append twice" \
        "echo x > /tmp/msh_t && echo y >> /tmp/msh_t && wc -l < /tmp/msh_t" \
        "       2"
    run_test "pipe empty left"      "echo | cat"        ""
    run_test "path absolute"        "/bin/echo direct"  "direct"
    run_test "path relative"        "echo relative"     "relative"
    run_test "\$? after true"       "true ; echo \$?"   "0"
    run_test "\$? after false"      "false ; echo \$?"  "1"
    run_test "\$? after 42"         "exit 42 ; echo \$?" "" 42
}

# ─── SUMMARY ─────────────────────────────────────────────────
print_summary() {
    printf "\n${BOLD}══════════════════════════════${RST}\n"
    printf "${BOLD}  Results: ${GRN}%d passed${RST}  ${RED}%d failed${RST}  / %d total\n" \
           "$PASS" "$FAIL" "$TOTAL"
    if [ "$FAIL" -gt 0 ]; then
        printf "\n${RED}Failed tests:${RST}%b\n" "$FAILED_TESTS"
        printf "${BOLD}══════════════════════════════${RST}\n"
        exit 1
    fi
    printf "${BOLD}══════════════════════════════${RST}\n"
}

# ─── DISPATCH ────────────────────────────────────────────────
printf "${BOLD}${CYN}Minishell Test Suite${RST}  →  %s\n" "$MINISHELL"

case "$CATEGORY" in
    builtins)   test_builtins ;;
    vars)       test_variables ;;
    pipes)      test_pipes ;;
    redirs)     test_redirections ;;
    logic)      test_logic ;;
    wildcards)  test_wildcards ;;
    errors)     test_errors ;;
    stress)     test_stress ;;
    edge)       test_edge_cases ;;
    all|*)
        test_builtins
        test_variables
        test_pipes
        test_redirections
        test_logic
        test_wildcards
        test_errors
        test_edge_cases
        test_stress
        ;;
esac

print_summary
