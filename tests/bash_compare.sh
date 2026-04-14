#!/bin/bash
# =============================================================
#  Minishell — Bash Comparison Tests
#  Runs same command in both minishell and bash, compares output.
#  Usage : ./tests/bash_compare.sh [./minishell]
# =============================================================

MINISHELL="${1:-./minishell}"
PASS=0
FAIL=0
SKIP=0

RED='\033[0;31m'
GRN='\033[0;32m'
YLW='\033[0;33m'
BLU='\033[0;34m'
BOLD='\033[1m'
RST='\033[0m'

cmp_test() {
    local desc="$1"
    local cmd="$2"

    bash_out=$(printf '%s\n' "$cmd" | bash 2>/dev/null)
    msh_out=$(printf '%s\n' "$cmd" | "$MINISHELL" 2>/dev/null)

    if [ "$bash_out" = "$msh_out" ]; then
        PASS=$((PASS + 1))
        printf "  ${GRN}✓${RST}  %s\n" "$desc"
    else
        FAIL=$((FAIL + 1))
        printf "  ${RED}✗${RST}  %s\n" "$desc"
        printf "      ${YLW}bash :${RST} %q\n" "$bash_out"
        printf "      ${YLW}msh  :${RST} %q\n" "$msh_out"
    fi
}

cmp_exit() {
    local desc="$1"
    local cmd="$2"

    printf '%s\n' "$cmd" | bash 2>/dev/null; bash_exit=$?
    printf '%s\n' "$cmd" | "$MINISHELL" 2>/dev/null; msh_exit=$?

    if [ "$bash_exit" = "$msh_exit" ]; then
        PASS=$((PASS + 1))
        printf "  ${GRN}✓${RST}  %s  (exit=%d)\n" "$desc" "$bash_exit"
    else
        FAIL=$((FAIL + 1))
        printf "  ${RED}✗${RST}  %s  (bash=%d msh=%d)\n" "$desc" "$bash_exit" "$msh_exit"
    fi
}

printf "${BOLD}${BLU}Bash Comparison Tests${RST}  →  %s\n" "$MINISHELL"

printf "\n${BOLD}echo / printf${RST}\n"
cmp_test "echo hello"          "echo hello"
cmp_test "echo -n"             "echo -n hi"
cmp_test "echo multiple"       "echo a b c"
cmp_test "echo empty"          "echo"
cmp_test "echo special chars"  "echo 'tab\there'"

printf "\n${BOLD}variables${RST}\n"
cmp_test "undefined var"       "echo \$TOTALLY_UNDEFINED_XYZ"
cmp_test "dollar ?"            "true; echo \$?"
cmp_test "dollar ? false"      "false; echo \$?"
cmp_test "export echo"         "export _T42=value && echo \$_T42"
cmp_test "single quote"        "echo 'no \$expand'"
cmp_test "double quote expand" "export _V=hi && echo \"say \$_V\""
cmp_test "empty dquote"        "echo \"\""

printf "\n${BOLD}builtins${RST}\n"
cmp_test "pwd"                 "pwd"
cmp_test "cd /tmp + pwd"       "cd /tmp && pwd"
cmp_test "env has PATH"        "env | grep -c '^PATH='"
cmp_test "export no args fmt"  "export | grep -c 'declare -x PATH='"
cmp_exit "exit 0"              "exit 0"
cmp_exit "exit 42"             "exit 42"
cmp_exit "exit 255"            "exit 255"

printf "\n${BOLD}pipes${RST}\n"
cmp_test "pipe basic"          "echo test | cat"
cmp_test "pipe wc"             "echo -n hello | wc -c"
cmp_test "pipe grep"           "printf 'foo\nbar\n' | grep foo"
cmp_exit "pipe exit code"      "true | false"

printf "\n${BOLD}redirections${RST}\n"
TMP=$(mktemp -d)
cmp_test "redirect >" \
    "echo redir > $TMP/r && cat $TMP/r"
cmp_test "redirect >>" \
    "echo a > $TMP/a && echo b >> $TMP/a && cat $TMP/a"
echo "content" > "$TMP/src"
cmp_test "redirect <" \
    "cat < $TMP/src"
rm -rf "$TMP"

printf "\n${BOLD}logical operators${RST}\n"
cmp_test "&& true"     "echo a && echo b"
cmp_test "&& false"    "false && echo b"
cmp_test "|| false"    "false || echo b"
cmp_test "|| true"     "true || echo b"
cmp_test "; chain"     "echo 1 ; echo 2"
cmp_test "mix && ||"   "false && echo x || echo y"

printf "\n${BOLD}wildcards${RST}\n"
TMP2=$(mktemp -d)
touch "$TMP2/a.c" "$TMP2/b.c" "$TMP2/test.h"
cd "$TMP2" || exit 1
cmp_test "glob *.c"    "ls *.c | sort"
cmp_test "glob *.h"    "ls *.h"
cmp_test "no match"    "echo *.xyz"
cmp_test "quoted glob" "echo '*.c'"
cd - > /dev/null || true
rm -rf "$TMP2"

printf "\n${BOLD}error codes${RST}\n"
cmp_exit "not found"   "command_not_found_xyzabc"
cmp_exit "syntax |"    "|"
cmp_exit "syntax &&"   "&&"

printf "\n${BOLD}══════════════════════════${RST}\n"
printf "${BOLD}Results: ${GRN}%d match bash${RST}  ${RED}%d differ${RST}  ${YLW}%d skipped${RST}\n" \
       "$PASS" "$FAIL" "$SKIP"
[ "$FAIL" -eq 0 ] && exit 0 || exit 1
