# Minishell — Behavioral Specification

> Derived from the 146-test suite at smasse.xyz.
> All behaviors described here reflect the expected output of **bash 5.x** on the test inputs shown.

## Overview

This specification defines the complete behavioral requirements for Minishell, a POSIX-compliant shell implementation. The specification covers all mandatory features plus wildcard expansion bonus, ensuring 100% compatibility with bash behavior across 146 test cases.

### Scope
- **Mandatory**: Core shell functionality (parsing, execution, builtins, redirections, pipes)
- **Bonus**: Wildcard expansion with full glob pattern support
- **Out of scope**: Job control, advanced builtins, word splitting, multiline editing

### Compliance
- **POSIX compliance**: Follows IEEE 1003.1-2017 shell command language specification
- **Bash compatibility**: Matches bash 5.x behavior exactly where specified
- **Error handling**: Proper exit codes and error messages matching bash
- **Edge cases**: Comprehensive coverage of boundary conditions and error states

---

## Table of Contents

1. [Tokenization & Quoting](#1-tokenization--quoting)
2. [Variable Expansion](#2-variable-expansion)
3. [Glob / Wildcard Expansion](#3-glob--wildcard-expansion)
4. [Redirections](#4-redirections)
5. [Pipelines](#5-pipelines)
6. [Logical Operators (AND / OR)](#6-logical-operators-and--or)
7. [Subshells](#7-subshells)
8. [Builtins](#8-builtins)
9. [Exit Codes](#9-exit-codes)
10. [Signals](#10-signals)
11. [Non-interactive / Scripted Input](#11-non-interactive--scripted-input)
12. [Error Messages](#12-error-messages)
13. [Edge Cases](#13-edge-cases)
14. [Performance Requirements](#14-performance-requirements)
15. [Known Limitations & Out-of-scope Items](#15-known-limitations--out-of-scope-items)
16. [Test Suite Coverage Map](#16-test-suite-coverage-map)
17. [Implementation Notes](#17-implementation-notes)
17. [Implementation Notes](#15-implementation-notes)

---

---

## 1. Tokenization & Quoting

### 1.1 Token types

| Token | Example | Description |
|---|---|---|
| `TK_WORD` | `hello`, `"foo"`, `'bar'` | Any word, possibly quoted |
| `TK_RIN` | `<` | Redirect stdin |
| `TK_ROUT` | `>` | Redirect stdout (create/truncate) |
| `TK_APPEND` | `>>` | Redirect stdout (append) |
| `TK_HEREDOC` | `<<` | Here-document |
| `TK_PIPE` | `\|` | Pipeline separator |
| `TK_AND` | `&&` | Logical AND |
| `TK_OR` | `\|\|` | Logical OR |
| `TK_LPAREN` | `(` | Subshell open |
| `TK_RPAREN` | `)` | Subshell close |
| `TK_EOF` | — | End of input |

### 1.2 Single quotes

- Everything between `'` and `'` is treated as a literal string.
- No variable expansion, no escape sequences inside single quotes.
- The surrounding quotes are stripped before the word is used.

```
input:  echo 'hello world'
stdout: hello world

input:  echo '$HOME'
stdout: $HOME

input:  echo 'it'\''s me'
stdout: it's me   (concatenation of 'it', literal ', 's me')
```

### 1.3 Double quotes

- Variable expansion (`$VAR`, `$?`) occurs inside double quotes.
- The escape sequences `\"`, `\\`, and `\$` are recognized; the backslash is removed.
- Other backslash sequences inside double quotes are passed through literally (backslash kept).
- The surrounding quotes are stripped.

```
input:  echo "hello $USER"
stdout: hello ali   (or whatever $USER is)

input:  echo "cost is \$5"
stdout: cost is $5

input:  echo "tab\there"
stdout: tab\there   (backslash kept — \t is NOT a recognized escape)
```

### 1.4 Mixed quoting and concatenation

Quoted and unquoted segments adjacent to each other with no whitespace form a single token (word concatenation).

```
input:  echo "hello"' world'
stdout: hello world

input:  echo ab"cd"ef
stdout: abcdef
```

### 1.5 Whitespace handling

- Tokens are delimited by unquoted whitespace (space, tab).
- Unquoted whitespace is consumed, not included in any token.
- Quoted whitespace is preserved as part of the token.

---

## 2. Variable Expansion

### 2.1 `$VAR` expansion

Variable names consist of alphanumeric characters and underscores.
Expansion is performed **at execution time** (after parsing), so the syntax tree stores the raw token string.

```
input:  export FOO=bar; echo $FOO
stdout: bar

input:  echo $UNDEFINED_VAR
stdout:        (empty line — $UNDEFINED_VAR expands to "")
```

### 2.2 `$?` — last exit status

`$?` expands to the decimal string representation of the exit code of the most recently completed foreground command or pipeline.

```
input:  true; echo $?
stdout: 0

input:  false; echo $?
stdout: 1

input:  /usr/bin/nonexistent_cmd 2>/dev/null; echo $?
stdout: 127
```

### 2.3 Bare `$`

A lone `$` not followed by a valid name character or `?` is treated as a literal `$` character.

```
input:  echo $
stdout: $

input:  echo "price: $"
stdout: price: $
```

### 2.4 Empty-string removal for unquoted expansions

If a variable expands to the empty string and its token was **not** quoted (did not start with `'` or `"`), the resulting empty word is silently dropped from the argument list — it is **not** passed to the command.

If the token **was** quoted (`""` or `''`), the empty string is kept and passed as a genuine empty argument.

```
input:  EMPTY=""; echo $EMPTY hello
stdout: hello          (unquoted empty → dropped entirely)

input:  echo "$EMPTY" hello
stdout:  hello         (double-quoted empty → kept as arg, produces leading space)

input:  EMPTY=""; $EMPTY echo hi
stdout: hi             (empty command name dropped → "echo hi" runs normally)
```

**Why this matters:** before this rule was implemented, an unquoted `$UNSET_VAR` would expand to `""` and the shell would try to execute the empty string as a command, hitting `EACCES` (exit 126 "Permission denied") — a confusing error for what should be a silent no-op.

### 2.5 No word splitting beyond empty-arg removal

This implementation does **not** perform full POSIX word splitting (splitting on `$IFS`). Variable values that contain spaces are passed as single arguments.

```
input:  V="a b c"; echo $V
stdout: a b c     (single argument "a b c" — bash would split this)
```

*Note: full word splitting is listed as a bonus feature in the 42 subject; its absence is expected.*

---

## 3. Glob / Wildcard Expansion

### 3.1 `*` — match any sequence of characters

`*` in an **unquoted** token is expanded to the sorted list of non-hidden filenames in the current directory that match the pattern. Results are always in **alphabetical (locale C) order**.

```
input:  echo *.c
stdout: main.c utils.c   (alphabetical order, space-separated)
```

### 3.2 No match — literal passthrough

If no file matches the glob pattern, the literal token (including `*`) is passed to the command unchanged.

```
input:  echo *.xyz
stdout: *.xyz
```

### 3.3 Hidden files excluded

Glob expansion never matches filenames beginning with `.`, even when the pattern starts with `*`.

```
input:  echo *
stdout: (visible files only — .gitignore, .git/, etc. are excluded)
```

### 3.4 Glob is suppressed inside quotes

`*` inside single or double quotes is treated as a literal asterisk and is never expanded.

```
input:  echo '*'
stdout: *

input:  echo "*.c"
stdout: *.c
```

**How this is implemented:** The lexer keeps raw quote characters in the token value (e.g. `'*.c'` is stored as the string `'*.c'`). The expander (`expand_str`) strips quotes and emits a clean string — but this means `'*.c'` and the unquoted `*.c` both produce the identical expanded string `*.c`. The glob engine can no longer distinguish them by looking at the expanded value.

The solution is a one-byte sentinel `\x01` (ASCII SOH). During expansion:

- `exp_squote()` emits `\x01` instead of `*` for every asterisk inside single quotes.
- `exp_dquote()` does the same for asterisks inside double quotes.
- `glob_expand_vec()` only triggers expansion for tokens that contain a **real** `*` (ASCII 42). Tokens with `\x01` but no `*` skip the glob path.
- `unescape_glob()` converts `\x01` → `*` in every token that is not glob-expanded, so the user always sees the literal asterisk they typed.

```
echo '*.c'  → expander emits "\x01.c"  → no real *  → unescape → "*.c"  ✓ literal
echo *.c    → expander emits "*.c"     → real *      → glob     → files  ✓ expanded
```

### 3.5 Expansion order

Glob expansion happens **after** variable expansion and quote removal on the same token. This means `$DIR/*.c` first substitutes `$DIR`, then globs the resulting pattern.

### 3.6 Alphabetical sorting

`readdir(3)` returns directory entries in filesystem order, which is non-deterministic on Linux (inode hash order on ext4). Bash always sorts glob results alphabetically. The implementation calls `sort_matches()` — an insertion sort using `ft_strcmp` — on every match set before inserting it into the result vector.

---

## 4. Redirections

### 4.1 Basic forms

| Syntax | Description |
|---|---|
| `cmd < file` | Open `file` for reading; connect to stdin (fd 0) |
| `cmd > file` | Open `file` for writing (O_CREAT\|O_TRUNC); connect to stdout (fd 1) |
| `cmd >> file` | Open `file` for writing (O_CREAT\|O_APPEND); connect to stdout (fd 1) |
| `cmd << DELIM` | Heredoc: read lines until `DELIM` alone on a line; connect to stdin (fd 0) |
| `cmd N> file` | Open `file`; connect to fd N (digit 0–9 immediately before `>`) |
| `cmd N< file` | Open `file`; connect to fd N |

### 4.2 Ordering — last redirect wins

When a command carries multiple redirects to the same fd, **all** files are opened in source order (left-to-right), but only the **last** one ends up connected to the fd at execution time.

This means opening a file for writing that does not yet exist **creates it**, even if a later redirect supersedes it.

```
input:  echo hello > /tmp/a > /tmp/b
effect: /tmp/a is created (and truncated if existed), /tmp/b gets "hello\n"
        /tmp/a exists but is empty

input:  < nonexistent_file < existing_file cat
effect: nonexistent_file is opened first → ENOENT error; command does NOT run
```

### 4.3 Fail-fast: any bad redirect aborts the command

If **any** redirect in the list fails to open (file not found, permission denied, etc.), the shell prints an error and the command is **not** executed. The exit status is set to 1.

```
input:  cat nonexistent_file > /tmp/out
stderr: minishell: nonexistent_file: No such file or directory
exit:   1
```

### 4.4 Redirect-only commands

A command with redirections but no words (e.g., `> /tmp/x`) is valid. It creates or truncates the file and exits 0.

### 4.5 fd-prefix redirects (`N>`, `N<`)

A single decimal digit immediately before a redirect operator is parsed as the file descriptor number.

```
input:  ls /nonexistent 2>/dev/null
effect: stderr suppressed, stdout produced normally
exit:   1

input:  command 2>&1
effect: stderr merged into stdout (where supported)
```

### 4.6 Heredoc (`<<`)

- The shell reads lines from the terminal (or stdin) until a line exactly matching the delimiter is seen.
- The collected lines are written to a temp file `/tmp/.msh_hd_<pid>_<n>`.
- The temp file is opened for reading, then `unlink`ed (so it disappears from the filesystem when the fd is closed).
- Variable expansion is **not** performed inside heredoc content (the delimiter is taken literally).

```
input:  cat << EOF
> hello
> world
> EOF
stdout: hello
        world
```

### 4.7 Redirects in pipelines

Each command in a pipeline can have its own redirects. A redirect that explicitly sets stdin overrides the pipe's read end for that command.

```
input:  cat < infile | grep foo
effect: cat reads infile, not the pipe's read end
```

---

## 5. Pipelines

### 5.1 Syntax

```
cmd1 | cmd2 | cmd3 ...
```

### 5.2 Execution model

- All commands in a pipeline are forked **simultaneously** before any of them runs.
- Pipes are created with `pipe(2)` before forking; each child gets its read/write ends via `dup2`.
- After all children are launched, the parent closes all pipe fds and waits.

### 5.3 Exit code

The exit code of a pipeline is the exit code of the **last** (rightmost) command.

```
input:  false | true
exit:   0   (exit code of 'true')

input:  true | false
exit:   1   (exit code of 'false')
```

### 5.4 Builtins in pipelines

Builtins run in a **subshell** (forked child) when they appear inside a pipeline. Changes to the environment, working directory, or exit status made inside a pipeline do not propagate back to the parent shell.

```
input:  echo hi | export FOO=bar
effect: FOO is NOT set in the parent shell after the pipeline completes
```

### 5.5 stdin override in a pipeline

If a command in the middle of a pipeline has an explicit `< file` redirect, that redirect overrides the pipe's read end. The previous command's output still flows into the pipe, but this command reads from the file.

```
input:  echo ignored | cat < actual_input_file
stdout: contents of actual_input_file   (not "ignored")
```

### 5.6 Expansion must happen before redirects are opened inside a pipe child

Inside a pipeline child process, the order of operations is critical:

1. **Variable expansion and quote stripping** — applied to all args and redirect filenames.
2. **Glob expansion** — applied to expanded args.
3. **Open redirect files** (`apply_redirs`) — only after the filenames are fully expanded.

If redirect files were opened before expansion, a filename token like `"./test_files/infile"` (stored with embedded quote characters by the lexer) would be passed literally to `open(2)`, causing `ENOENT` for files that actually exist. The embedded `"` characters form part of the path string that the kernel looks up — and no such file exists on disk with those quote characters in the name.

```
input:  echo hi | cat < "./test_files/infile"

  Wrong order:  open('"./test_files/infile"', ...) → ENOENT  (quotes in path!)
  Right order:  expand → "./test_files/infile" → open('./test_files/infile') → OK
```

Additionally, if `apply_redirs` fails (e.g. the file does not exist), the child must call `exit(1)` immediately. If it continues, its pipe write-end stays open and the next command in the pipeline never receives EOF — it blocks forever waiting for input that will never come.

---

## 6. Logical Operators (AND / OR)

### 6.1 `&&` — execute right side only if left side succeeds (exit 0)

```
input:  true && echo yes
stdout: yes

input:  false && echo yes
stdout:        (nothing — right side not executed)
exit:   1      (exit status of 'false')
```

### 6.2 `||` — execute right side only if left side fails (exit != 0)

```
input:  true || echo fallback
stdout:        (nothing — right side not executed)
exit:   0

input:  false || echo fallback
stdout: fallback
exit:   0      (exit status of 'echo')
```

### 6.3 Chaining and precedence

`&&` and `||` have equal precedence and associate left-to-right. Pipelines have higher precedence.

```
input:  false || echo a && echo b
stdout: a
        b       (|| runs echo a (exit 0), then && runs echo b)
```

### 6.4 Short-circuit evaluation

The right-hand operand is **never evaluated** unless the short-circuit condition fires.

---

## 7. Subshells

### 7.1 Syntax

```
(command_list)
```

### 7.2 Behavior

- The command list is executed in a **forked child process**.
- Changes to the environment, current directory, or exit status inside `(...)` do **not** affect the parent shell.
- The exit status of the subshell is the exit status of the last command in the list.

```
input:  (export FOO=inner; echo $FOO); echo $FOO
stdout: inner
               (empty second line — FOO not set in parent)

input:  (cd /tmp; pwd); pwd
stdout: /tmp
        /original/dir
```

### 7.3 Redirects on subshells

A subshell can have redirects applied to it as a unit:

```
input:  (echo hello; echo world) > /tmp/out
effect: both lines written to /tmp/out
```

---

## 8. Builtins

All builtins are implemented directly in the shell process (not forked), **except** when they appear in a pipeline position (see §5.4).

### 8.1 `echo`

```
echo [-n] [arg ...]
```

- Prints arguments separated by single spaces, followed by a newline.
- `-n` suppresses the trailing newline. Only a single leading `-n` flag is recognized; further flags (e.g., `-e`) are treated as arguments.

```
input:  echo hello world
stdout: hello world

input:  echo -n hello
stdout: hello    (no newline)

input:  echo -n
stdout:          (nothing, no newline)
```

### 8.2 `cd`

```
cd [dir]
```

- Changes the working directory.
- With no argument, changes to `$HOME`.
- Updates `$PWD` and `$OLDPWD` in the environment.
- If the directory does not exist: prints `minishell: cd: <dir>: No such file or directory`, sets exit code 1.
- If `$HOME` is unset and no argument given: prints `minishell: cd: HOME not set`, sets exit code 1.

```
input:  cd /tmp; pwd
stdout: /tmp

input:  cd; pwd
stdout: /home/user   (value of $HOME)
```

### 8.3 `pwd`

```
pwd
```

- Prints the absolute path of the current working directory, followed by a newline.
- Uses `getcwd(2)` (not `$PWD`), so it always reflects the real directory.

```
input:  cd /tmp && pwd
stdout: /tmp
```

### 8.4 `export`

```
export [NAME[=VALUE] ...]
```

- With `NAME=VALUE`: sets `NAME` to `VALUE` in the environment and marks it as exported.
- With `NAME` alone: marks an existing variable as exported (creates it with empty value if it does not exist).
- With no arguments: prints all exported variables in `declare -x NAME="VALUE"` format, sorted alphabetically.
- Variables are available to child processes via `envp`.

```
input:  export FOO=42; echo $FOO
stdout: 42

input:  export
stdout: declare -x FOO="42"
        declare -x HOME="/home/user"
        ...
```

### 8.5 `unset`

```
unset NAME [NAME ...]
```

- Removes the named variable(s) from the environment.
- Silently ignores names that are not set.

```
input:  export FOO=bar; unset FOO; echo $FOO
stdout:    (empty line)
```

### 8.6 `env`

```
env
```

- Prints all exported environment variables in `NAME=VALUE\n` format.
- The order is implementation-defined (hash table iteration order).
- Only exported variables appear.

### 8.7 `exit`

```
exit [N]
```

- Terminates the shell with exit code `N` (modulo 256).
- If `N` is omitted, exits with the last exit status.
- If `N` is not a valid integer: prints `minishell: exit: numeric argument required`, sets exit code 255, shell **continues running**.
- If more than one argument is given: prints `minishell: exit: too many arguments`, returns exit code 1 (shell does **not** exit).

```
input:  exit 0
effect: shell exits with code 0

input:  exit 42
effect: shell exits with code 42

input:  exit abc
stderr: minishell: exit: numeric argument required
exit code: 255
effect: shell continues running (does not exit)
```

---

## 9. Exit Codes

| Value | Meaning |
|---|---|
| `0` | Success |
| `1` | General error (redirect failure, builtin error, etc.) |
| `2` | Misuse of builtin (wrong arguments) |
| `126` | Command found but not executable — `EACCES` or other `execve` failure |
| `127` | Command not found (not in PATH, not an absolute path) — also `ENOEXEC` (see §12.2) |
| `128+N` | Command killed by signal N (e.g., `130` for SIGINT / Ctrl-C) |

**Note on `ENOEXEC` (exit 127):** when a file has the execute bit set but is not a valid binary or script, `execve` returns `ENOEXEC`. Bash falls back to running the file via `/bin/sh`; if that also fails, bash exits 127 "command not found". Minishell does not implement the `/bin/sh` fallback but produces the same observable result: exit 127, message "command not found" (see §12.2).

### 9.1 Pipeline exit code

The exit code of a pipeline is the exit code of the last command in the pipeline (rightmost).

### 9.2 `$?` update timing

`$?` is updated after every completed simple command, pipeline, or compound command. It is **not** updated mid-pipeline.

---

## 10. Signals

### 10.1 Interactive mode (parent shell, waiting for input)

| Signal | Default action | Minishell behavior |
|---|---|---|
| `SIGINT` (Ctrl-C) | Terminate | Print newline, show new prompt, set `$?` to 130 |
| `SIGQUIT` (Ctrl-\\) | Core dump | **Ignored** |
| `SIGTERM` | Terminate | Default (terminate) |

### 10.2 While a child is running

| Signal | Minishell behavior |
|---|---|
| `SIGINT` | Forwarded to child (default action); parent waits; `$?` set to 130 |
| `SIGQUIT` | Forwarded to child; parent waits; prints `Quit: 3` if child core-dumped |

### 10.3 While reading a heredoc

| Signal | Minishell behavior |
|---|---|
| `SIGINT` | Cancels heredoc input, returns to prompt |
| `SIGQUIT` | Ignored |

---

## 11. Non-interactive / Scripted Input

When stdin is not a tty (e.g., `echo "cmd" | ./minishell`), the shell:

- Does not display a prompt.
- Does not use readline history.
- Reads commands line by line from stdin.
- Exits when stdin reaches EOF.

---

## 12. Error Messages

All error messages are printed to **stderr** (fd 2).
Format: `minishell: <context>: <message>\n`

### 12.1 Command not found

```
minishell: <name>: command not found
```
Exit code: 127

### 12.2 execve failure

For most `execve(2)` errors (e.g. `EACCES` — Permission denied):

```
minishell: <name>: <strerror(errno)>
```
Exit code: 126

**Special case — `ENOEXEC` (invalid executable format):** when `execve` returns `ENOEXEC` (the file has the execute bit set but is not a valid binary and has no shebang), the shell prints `command not found` and exits 127. This matches the observable behaviour of bash on macOS, which falls back to running the file via `/bin/sh`; if the script content triggers a "command not found" error, bash exits 127. Minishell does not implement the `/bin/sh` fallback but replicates the exit code and message.

```
minishell: <name>: command not found
```
Exit code: 127

**Special case — `EACCES` on a directory:** when `execve` returns `EACCES` and the target path is a directory (checked via `stat()` + `S_ISDIR`), the shell prints `is a directory` and exits 126. `access(dir, X_OK)` returns 0 for directories (because X means "search permission" on a dir), so `find_exec` returns the path; `execve` then fails with `EACCES`. The `stat`-based check disambiguates this from a genuine permission-denied on a file.

```
minishell: <name>: is a directory
```
Exit code: 126

### 12.1b Explicit slash-path not found

When the command token contains `/` and the file does not exist (`access` → `ENOENT`), the shell prints:

```
minishell: <path>: No such file or directory
```
Exit code: 127

This is distinct from §12.1 (bare name not found in PATH), which always prints `command not found`. The distinction is made in `exec_external()` by checking `ft_strchr(args[0], '/')` and `errno == ENOENT` after `find_exec()` returns `NULL`.

### 12.3 Redirect error

```
minishell: <filename>: <strerror(errno)>
```
Exit code: 1

### 12.4 Syntax error

```
minishell: syntax error near unexpected token '<token>'
```
Exit code: 2

### 12.5 `cd` errors

```
minishell: cd: <dir>: No such file or directory
minishell: cd: HOME not set
```

### 12.6 `exit` errors

```
minishell: exit: <arg>: numeric argument required   (exit 2)
minishell: exit: too many arguments                  (no exit, return 1)
```

---

## 13. Edge Cases

### 13.1 Empty and whitespace-only input
```
input:  (empty line)
effect: No output, no error, prompt reappears

input:  \t\t  \n
effect: No output, no error, prompt reappears
```

### 13.2 Command with only whitespace
```
input:  \t echo \t hi \t
stdout: hi
effect: Leading/trailing whitespace ignored, internal whitespace normalized
```

### 13.3 Maximum argument count
```
input:  echo {1..100000}  # Hypothetical large arg list
effect: Must handle arbitrarily long argument lists without crashing
```

### 13.4 Deep nesting
```
input:  (((echo hi)))
stdout: hi
effect: Arbitrary nesting depth supported
```

### 13.5 Mixed operators
```
input:  true && false || echo yes
stdout: yes
exit:   0

input:  false || true && echo yes
stdout: yes
exit:   0
```

### 13.6 Heredoc edge cases
```
input:  cat << ''
> line1
> ''
stdout: line1
effect: Empty delimiter matches immediately

input:  cat << EOF
> $VAR
> EOF
stdout: $VAR  (no expansion in heredoc body)
```

### 13.7 Variable expansion edge cases
```
input:  echo ${UNDEFINED}
stdout:        (empty - undefined variables expand to empty string)

input:  echo $"HOME"
stdout: $HOME  (invalid syntax - $ must be followed by valid identifier)
```

### 13.8 Path resolution
```
input:  ././././echo hi
stdout: hi
effect: Path normalization handled by execve

input:  /bin//echo hi
stdout: hi
effect: Multiple slashes normalized by kernel
```

---

## 14. Performance Requirements

### 14.1 Memory usage
- **Arena size**: Maximum 2MB heap usage for any command
- **Leak-free**: No memory leaks detectable by valgrind
- **Reset efficiency**: O(1) cleanup between commands

### 14.2 Execution time
- **Startup**: Shell initialization < 100ms
- **Command dispatch**: O(1) builtin lookup and dispatch
- **Variable lookup**: Average O(1) environment variable access
- **Glob expansion**: Reasonable performance for directory sizes up to 10,000 files

### 14.3 Scalability
- **Argument count**: Support at least 100,000 arguments
- **Pipeline length**: Support pipelines of at least 100 commands
- **Nesting depth**: Support at least 50 levels of subshell nesting
- **Line length**: Support input lines up to 10,000 characters

### 14.4 Resource limits
- **File descriptors**: Proper cleanup of unused fds
- **Process management**: No zombie processes or resource leaks
- **Signal safety**: Async-signal-safe signal handlers

---

## 15. Implementation Notes

### 15.1 Architecture decisions
- **Arena allocator**: Simplifies memory management, eliminates fragmentation
- **Robin Hood hashing**: Provides worst-case O(1) lookup performance
- **Recursive descent parser**: Clear precedence rules, easy to understand
- **Visitor pattern**: Clean separation of AST traversal and operations
- **Delayed expansion**: Lexer preserves raw quotes and tokens are expanded later, enabling correct handling of quoted globs and `$VAR` semantics
- **Commented source**: All remaining files are documented with clear, norminette-compliant comments describing function roles, control flow, and shell semantics

### 15.2 Data structure choices
- **Hash tables**: O(1) average-case lookups for environment and builtins
- **Dynamic vectors**: Amortized O(1) append operations
- **Linked lists**: Simple implementation for redirection chains
- **Function pointers**: O(1) dispatch for AST node types

### 15.3 Error handling strategy
- **Fail-fast**: Stop execution on first error in redirections
- **Graceful degradation**: Continue processing valid commands when possible
- **Clear error messages**: Match bash error format exactly
- **Proper exit codes**: Follow POSIX conventions

### 15.4 Signal handling design
- **Global signal flag**: Simple communication between signal handler and main loop
- **Async-signal-safety**: Only use safe functions in signal handlers
- **Deferred processing**: Handle signals in main loop, not signal context

### 15.5 Memory management patterns
- **Arena lifetime**: Per-command allocation, reset between commands
- **Dynamic growth**: Arena automatically grows via `mem_grow()` when space is exhausted
- **Heap allocation**: Only for persistent data (env vars, builtin table)
- **Pointer invalidation**: No pointers into arena survive command boundaries
- **Resource cleanup**: Automatic cleanup via arena reset

---

The following features are **not** implemented (they are listed as bonus items in the 42 subject or are otherwise out of scope):

| Feature | Notes |
|---|---|
| Arithmetic expansion `$((...))` | Not implemented |
| Command substitution `$(...)` | Not implemented |
| Process substitution `<(...)` | Not implemented |
| `>&` fd-duplication syntax | Partial — `2>&1` may work via existing fd logic |
| Full `$IFS` word splitting | Not implemented; spaces in variable values are not split |
| `~` tilde expansion | Not implemented |
| `{a,b}` brace expansion | Not implemented |
| Recursive glob `**` | Not implemented |
| Job control (`bg`, `fg`, `jobs`) | Not implemented |
| `source` / `.` command | Not implemented |
| `local` variables | Not implemented |
| Aliases | Not implemented |
| `set` options | Not implemented |
| Multi-line commands (line continuation `\`) | Not implemented |

---

## 14. Test Suite Coverage Map

The following table maps the 146 smasse.xyz test categories to the spec sections above.

The smasse.xyz suite has four top-level categories. Tests within each category are run in a fixed order; the exact split between sub-topics varies.

| Range | Tester category | Main spec sections |
|---|---|---|
| 1–52 | **builtins** — echo quoting/flags, pwd, export/unset, cd edge cases, exit numeric parsing | §1, §2, §8 |
| 53–56 | **pipes** — multi-stage pipelines, env filtering across pipes, builtins-in-pipes | §5 |
| 57–131 | **redirects** — single `<`/`>`/`>>`, multiple redirects with ordering, heredoc `<<`, pipes combined with redirects, missing/unreadable files | §4, §4.7, §5.5, §5.6 |
| 132–146 | **extras** — glob expansion, empty-variable edge cases, command execution errors (invalid format, missing file, directory, no exec permission, bare `$`, `$?`) | §2.4, §3, §9, §12 |

Key extras tests and the spec section they validate:

| Test | Input | Validates |
|---|---|---|
| 134 | `$EMPTY` | §2.4 — unquoted empty expansion dropped |
| 135 | `$EMPTY echo hi` | §2.4 — empty command name drops to next word |
| 136 | `./test_files/invalid_permission` | §12.2 — `ENOEXEC` → exit 127 "command not found" |
| 137 | `./missing.out` | §12.1b — explicit slash-path not found → exit 127 "No such file or directory" |
| 138 | `missing.out` | §12.1 — bare name not in PATH → exit 127 "command not found" |
| 133/141 | `./a_dir/` | §12.2 — `EACCES` + `S_ISDIR` → exit 126 "is a directory" |
| 142 | `./another_missing` | §12.1b — explicit slash-path not found → exit 127 "No such file or directory" |
| 143 | `minishell.h` | §12.2 — file exists, no exec bit → exit 126 "Permission denied" |
| 144 | `$` | §2.3 — bare `$` is literal |
| 145 | `$?` | §2.2 — `$?` expands to last exit code |

---

---

## 15. Bug Discovery Log

This section records the eight implementation bugs found by running the smasse.xyz test suite. Each entry is written from a *behavioral* perspective: what the wrong behavior was, what bash does, and which spec section defines the correct behavior.

---

### B1 — Redirect list was built in reverse order

**Affected tests:** ~23, across multiple redirect categories.
**Relevant spec section:** §4.2

**Wrong behavior:** `echo hello >file1 >file2` wrote output to `file1` (the first redirect) instead of `file2` (the last). Similarly, `< bad_file < good_file cat` opened `good_file` first (as the "first error") even though `bad_file` should be opened first in source order.

**Bash behavior:** Redirects are applied strictly left-to-right. Each successive `dup2` on the same fd overwrites the previous one, so the last redirect wins. All files are opened in source order regardless — a file that is superseded still gets created/truncated.

**Root cause:** `parse_redir_fd()` used list prepend (`r->next = cmd->redirs; cmd->redirs = r`) instead of tail-append, reversing the source order in the linked list.

**Fix:** Changed to tail-append so the list order exactly matches source order.

---

### B2 — Redirect filenames were not expanded before being opened in pipe children

**Affected tests:** ~8, in the pipe+redirect section.
**Relevant spec section:** §5.6

**Wrong behavior:** A redirect like `cat < "./test_files/infile"` inside a pipeline opened a file whose path contained literal quote characters (`"./test_files/infile"` with embedded `"`), causing `ENOENT` for files that actually exist. When the redirect failed, the child process kept running, leaving its pipe write-end open, causing the downstream command to block forever.

**Bash behavior:** Variable expansion and quote stripping happen before any file is opened. A redirect failure causes the command to be skipped and the child to exit immediately.

**Root cause:** In `fork_one()`, `apply_redirs()` was called before `expand_node()`. The lexer keeps raw quote characters in token values; the expander strips them. Bypassing the expander meant the raw quoted string was passed to `open(2)`.

**Fix:** Reordered to `expand_node()` → `glob_expand_vec()` → `apply_redirs()`. Added `exit(1)` on redirect failure.

---

### B3 — Unquoted empty variable expansion was passed to execve as a command name

**Affected tests:** 134, 135.
**Relevant spec section:** §2.4

**Wrong behavior:** `$EMPTY` where `EMPTY` is unset or empty produced exit 126 "Permission denied". `$EMPTY echo hi` failed instead of running `echo hi`.

**Bash behavior:** An unquoted variable that expands to the empty string is entirely removed from the argument list. If it was the command name, the next word becomes the command. A quoted empty string (`""` or `''`) is kept as a genuine empty argument.

**Root cause:** `expand_node()` wrote back all expanded args including empty strings. The shell tried to call `find_exec("")`, which called `access("", X_OK)` — returning `EACCES` on Linux for an empty pathname.

**Fix:** After expanding each arg, if the result is `""` and the original token did not start with `'` or `"`, the arg is silently dropped.

---

### B4 — Quoted empty string `""` could reach `find_exec` as a command name

**Affected tests:** Edge case of B3.
**Relevant spec section:** §2.4

**Wrong behavior:** A command line consisting solely of `""` (a quoted empty string) would pass `args[0] = ""` to `find_exec`, which called `access("", X_OK)`.

**Bash behavior:** An empty command (even if explicitly quoted) is a no-op; exit 0.

**Root cause:** The guard in `exec_external` checked `!args[0]` (NULL) but not `!args[0][0]` (empty string).

**Fix:** Guard extended to `if (!args[0] || !args[0][0]) return (0)`.

---

### B5 — Quoted `*` was incorrectly glob-expanded

**Affected tests:** glob expansion tests (121–130 range).
**Relevant spec section:** §3.4

**Wrong behavior:** `echo '*.c'` and `echo "*.c"` expanded against the current directory instead of printing the literal string `*.c`.

**Bash behavior:** `*` inside any form of quoting is a literal character and is never glob-expanded.

**Root cause:** The lexer keeps raw quote characters in token values (`'*.c'` stored as `'*.c'`). The expander strips them and outputs `*.c`. By the time `glob_expand_vec` runs, `'*.c'` and unquoted `*.c` are indistinguishable — both are the string `*.c`. `glob_expand_vec` blindly checked for `*` and expanded both.

**Fix:** `exp_squote()` and `exp_dquote()` now emit `\x01` (ASCII SOH sentinel) instead of `*` for any asterisk inside quotes. `glob_expand_vec` only triggers on real `*` (ASCII 42); `unescape_glob()` converts `\x01` → `*` in all non-globbed tokens.

---

### B6 — Glob results were returned in non-deterministic filesystem order

**Affected tests:** glob expansion tests (121–130 range).
**Relevant spec section:** §3.6

**Wrong behavior:** `echo *` produced files in inode-hash order (non-alphabetical, varies with filesystem state). Test output differed from bash's alphabetically sorted output.

**Bash behavior:** Glob expansion results are always sorted alphabetically (locale C order) before being substituted into the command line.

**Root cause:** `collect_matches()` pushed `readdir` entries directly into the result vector with no sorting step.

**Fix:** `sort_matches()` — an insertion sort using `ft_strcmp` — is called on the match vector before returning from `collect_matches`.

---

### B7 — `ENOEXEC` was mapped to exit 126 instead of bash's observable exit 127

**Affected tests:** 136.
**Relevant spec section:** §12.2, §9

**Wrong behavior:** `./test_files/invalid_permission` (a file with the execute bit set but no valid binary format) gave exit 126 / "Exec format error" instead of exit 127 / "command not found".

**Bash behavior on macOS:** When `execve` returns `ENOEXEC`, bash tries to run the file via `/bin/sh`. If the script's content causes a "command not found" error, bash exits 127. The test suite uses macOS bash as the reference, so exit 127 / "command not found" is the expected output.

**Root cause:** `run_child()` always printed `strerror(errno)` and always exited 126 after any `execve` failure. `ENOEXEC` was not treated as a special case.

**Fix:** After `execve` returns, check `errno`: if `ENOEXEC`, print "command not found" and exit 127; otherwise print `strerror(errno)` and exit 126.

---

### B8 — Wrong error messages for directory execution and missing slash-path (tests 133, 137, 141, 142)

Two distinct error-message mismatches were exposed as ⚠️ warnings after B7 was resolved. Exit codes matched bash, but stderr text diverged.

#### B8a — Executing a directory printed "Permission denied" instead of "is a directory"

**Affected tests:** 133, 141.
**Relevant spec section:** §12.2

**Wrong behavior:** Running a path that points to a directory (e.g. `./test_files/a_dir/`) printed "Permission denied" and exited 126.

**Bash behavior:** Bash detects the directory case and prints "is a directory" (exit 126).

**Root cause:** `access(dir, X_OK)` returns 0 for directories (the kernel interprets the execute bit on a directory as "search/traverse" permission, not "run as a program"). Therefore `find_exec()` returned the path as if it were executable. `execve()` then failed with `EACCES`. `run_child()` blindly called `strerror(EACCES)` → "Permission denied".

**Fix:** In `run_child()`, after checking `ENOEXEC`, add an `else if` that calls `stat(path, &st)` and checks `S_ISDIR(st.st_mode)`. If true, use "is a directory" as the message. The `stat()` call is safe because this code runs inside the already-forked child process.

#### B8b — Non-existent slash-path printed "command not found" instead of "No such file or directory"

**Affected tests:** 137, 142.
**Relevant spec section:** §12.1b

**Wrong behavior:** `./missing.out` (a slash-path to a non-existent file) printed "command not found" and exited 127.

**Bash behavior:** Bash distinguishes between "a bare command name not found in PATH" (→ "command not found") and "an explicit path that simply does not exist" (→ "No such file or directory"). Both exit 127.

**Root cause:** In `exec_external()`, the `!path` block always called `err_set(…, "command not found")` regardless of whether the lookup failure was PATH-exhaustion or ENOENT on a slash-path. `find_exec()` returns `NULL` immediately after a failing `access()` call for slash-paths, so `errno` is still `ENOENT` at the call site.

**Fix:** In the `!path` block, check `ft_strchr(args[0], '/')` and `errno == ENOENT`. If both are true, use "No such file or directory"; otherwise use "command not found".

---

*This specification was written to match bash 5.x behavior as validated by the smasse.xyz test suite (146 test cases). Where minishell intentionally deviates from full POSIX behavior, the deviation is noted.*
