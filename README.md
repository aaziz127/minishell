*This project has been created as part of the 42 curriculum by alaziz, xxxxxx.*

# Minishell

*As beautiful as a shell.*

## Description

Minishell is an interactive, POSIX-inspired shell written in C for the 42 curriculum. It reproduces essential `bash` behaviour, including tokenization, parsing into an AST, variable expansion, I/O redirection, pipes, logical operators, subshells, builtins, signal handling, wildcards, and a readline-powered interactive line editor with history.

The goal is to build a shell engine that interprets command lines correctly, executes external programs with `fork`/`execve`, manages environment variables, and handles shell-specific syntax in a safe and maintainable way.

### Key Implementation Goals

- Correct shell syntax interpretation
- Robust environment variable and wildcard expansion
- Accurate redirections, pipes, and subshell semantics
- Clean builtin execution and signal handling
- Efficient temporary memory management using arena allocation

### Features

| Category | Behaviour |
|----------|-----------|
| **Prompt** | GNU readline with history (up/down arrows, line editing) and 🚀 rocket emoji |
| **Startup Banner** | Fancy ASCII art welcome screen with colors and animations |
| **Single quotes** | `'...'` — no expansion, every character literal |
| **Double quotes** | `"..."` — `$VAR` and `$?` expanded; `\"` `\\` `\$` are escape sequences |
| **Variables** | `$VAR` expanded; `$?` becomes last exit status; bare `$` stays literal |
| **Redirections** | `<` `>` `>>` `<<` (heredoc); multiple per command; last redirect wins per fd |
| **Pipes** | `cmd1 \| cmd2 \| …` chained arbitrarily |
| **Logical operators** | `&&` and `\|\|` with correct short-circuit (integrated by default) |
| **Semicolons** | `;` unconditional command separator |
| **Subshells** | `(cmd)` forks a child process for the grouped commands |
| **Wildcards** | `*` expanded against the current directory (bonus) |
| **Builtins** | `echo` `cd` `pwd` `export` `unset` `env` `exit` |
| **Error handling** | Invalid builtin arguments print error and continue (e.g., `exit dsds` stays in shell with exit code 255) |
| **$? tracking** | Accurate for every command, pipeline, builtin, and signal termination |
| **Signals** | Ctrl-C reprints prompt (exit 130); Ctrl-D exits; Ctrl-\ ignored |

---

## Instructions

### Requirements

- **macOS**: GNU readline (`brew install readline`)
- **Linux**: GNU readline (usually pre-installed)
- **Compiler**: GCC or Clang with C99 support
- Unix-like environment (macOS or Linux)

### Compilation

From the repository root:

```bash
make          # build the mandatory version
make bonus    # build with wildcard support
make clean    # remove object files
make fclean   # remove executables and object files
```

### Installation

If readline is not installed, install it with `brew install readline` on macOS or the system package manager on Linux.

### Execution

Run the shell from the repository root:

```bash
./minishell
```

The shell accepts commands interactively. Use `Ctrl-D` to exit or type `exit`.

### Example Session

```bash
$ ./minishell
🚀 minishell$ echo "Hello world"
Hello world
🚀 minishell$ export TEST=42
🚀 minishell$ echo $TEST
42
🚀 minishell$ ls | grep minishell
minishell
🚀 minishell$ ls -la && echo "success" || echo "failure"
total 64
drwxr-xr-x  12 user  staff   384 Jan 15 10:00 .
...
success
🚀 minishell$ exit
```

### Notes

- `make` builds the mandatory version which includes logical operators (`||` and `&&`) by default.
- `make bonus` enables wildcard expansion support using `*` patterns.
- If readline is not installed, install it with `brew install readline` on macOS or the system package manager on Linux.

---

## Resources

### References Used for Implementation

- **POSIX shell command language documentation** — Shell grammar and semantics
- **GNU readline manual** — Interactive line editing capabilities
- **Unix process control and `fork`/`execve` semantics** — Process management
- **Man pages**: `man bash`, `man execve`, `man pipe`, `man dup2`, `man waitpid`
- **Articles on recursive-descent parsing and shell grammars** — Parser design patterns
- **Tutorials on Robin Hood hashing and arena allocators** — Memory management strategies

### AI Usage

AI support was used throughout the project for:

- **Structure the README and project documentation** — Organized content for clarity and completeness, ensuring all 42 curriculum requirements are met
- **Clarify internal data structures and shell architecture** — Explained complex parsing strategies, memory management approaches, and execution flow
- **Generate a workflow for implementation and debugging** — Provided guidance on test-driven implementation methodology and systematic debugging approaches
- **Review and refine comments and design explanations** — Enhanced code readability and maintainability through improved documentation
- **Assist with problem-solving during implementation** — Helped identify issues with quote preservation, redirection handling, signal management, wildcard expansion, and exit status propagation
- **Support architectural decisions** — Validated design patterns and choices for memory management, environment handling.
- **Debugging assistance** — Helped trace and fix edge cases in pipe handling, heredoc , and signal safety

AI was used as a guiding support tool; the final implementation and code changes were written and reviewed by the project authors.

---

## Project Structure

```
minishell/
├── includes/
│   ├── types.h          — all structs, enums, and #defines
│   └── minishell.h      — all function prototypes grouped by source file
├── core/
│   ├── mem.c            — bump-pointer arena allocator
│   ├── mem1.c           - Arena allocator growing  
│   ├── strbuf.c         — dynamic string builder (backed by arena)
│   ├── str.c            — arena string utilities (dup, cat3, itoa)
│   ├── error.c          — recoverable error (set / emit / clear / syntax)
│   ├── error_fatal.c    — unrecoverable fatal error (prints + exit(1))
│   ├── env.c            — Robin Hood hash table: insert, update, resize
│   ├── env_getdel.c     — Robin Hood hash table: lookup and delete
│   ├── env_init.c       — env allocation, population from envp, free
│   ├── env_ops.c        — env print (export -x format), env_to_envp
│   ├── env_iter.c       — sequential iterator over occupied env slots
│   ├── vector.c         — generic pointer vector (arena-backed)
│   ├── tok_iter.c       — token stream iterator (peek / next / expect)
│   ├── visitor.c        — expand visitor (pre-walk AST before pipes)
│   ├── visitor_exec_mk.c— mk_exec_visitor: builds the execution dispatch table
│   ├── builtin_ht.c     — builtin hash table: new / get / set
│   ├── builtin_ht_free.c— builtin hash table: free
│   ├── shell_init.c     — shell_init and shell_free
│   └── tables_init.c    — dispatch_init and builtin_ht_init
├── parsing/
│   ├── lexer.c          — tokeniser (word / operator / quote scanning)
│   ├── lexer_utils.c    — operator classification helpers
│   ├── syntax.c         — pre-parse token sequence validation
│   ├── parser.c         — recursive-descent parser (list / pipe / node)
│   ├── parser_cmd.c     — command parsing primitives (mk_cmd, push_redir, …)
│   ├── parser_word.c    — parse_cmd top-level and word-token handler
│   ├── expander.c       — string expansion: variables, quotes, sentinels
│   └── expander_node.c  — node expansion: args, redirs, word splitting
├── execution/
│   ├── executor.c       — execute() entry point via visitor dispatch
│   ├── visitor_exec.c   — execution visitor handlers (xv_cmd, xv_pipe, …)
│   ├── exec_ops.c       — run_child, fork_exec, exec_cmd, exec_sub
│   ├── exec_utils.c     — find_exec (PATH search), wait_child, restore_fds
│   ├── pipes.c          — multi-stage pipeline orchestration
│   ├── redirections.c   — apply_redirs, handle_heredoc, open_file
│   └── signals.c        — signal handler and three setup modes
├── builtins/
│   ├── bi_echo.c
│   ├── bi_cd.c
│   ├── bi_pwd.c
│   ├── bi_export.c
│   ├── bi_unset.c
│   ├── bi_env.c
│   └── bi_exit.c
├── utils/
│   ├── ft_str.c         — ft_strlen, ft_strcmp, ft_strchr, ft_strdup
│   └── ft_utils.c       — ft_isspace, ft_isalpha, ft_isdigit, ft_isalnum, ft_valid_id
├── bonus/
│   └── wildcards.c      — glob_expand_vec, match_pat, sort_matches
├── main.c               — main, shell_loop, process_input, display_startup_banner
└── Makefile
```

---

## Architecture Overview

```
main.c
  ├─ display_startup_banner()  ← fancy welcome screen (interactive only)
  └─ shell_loop()
       ├─ readline()             ← GNU readline: line editing + history
       ├─ tokenize()             ← lexer.c / lexer_utils.c
       ├─ check_syntax()         ← syntax.c
       ├─ parse()                ← parser.c / parser_cmd.c / parser_word.c
       ├─ execute()              ← executor.c
       │    └─ visit()           ← visitor.c / visitor_exec.c
       │         ├─ xv_cmd   ── expand_node() → glob_expand_vec() → exec_cmd()
       │         ├─ xv_pipe  ── exec_pipe()   (forks all stages at once)
       │         ├─ xv_and / xv_or           (short-circuit, recursive visit)
       │         └─ xv_sub   ── exec_sub()   (fork → visit in child)
       └─ mem_reset()            ← reclaim all arena memory in O(1)
```

The entire per-command lifecycle runs inside a single **arena allocator** (`t_mem`). Tokens, AST nodes, strings, vectors — all live there. `mem_reset()` at the end of each command reclaims everything in O(1) by simply resetting a counter. No per-node `free()` calls are needed in the hot path.

The lexer preserves raw quote delimiters so the expander can distinguish quoted and unquoted content. Expansion occurs after parsing and uses sentinel values to suppress quoted glob expansion and preserve quoted empty strings.

---

## Pipeline: From Input to Execution

### 1. `readline()` — Interactive Input

GNU readline provides line editing (Ctrl-A/E, arrows, backspace), history (up/down), and the `🚀 minishell$` prompt. It returns a heap-allocated string that the caller must `free()`. On EOF (Ctrl-D) it returns NULL, and the shell prints `exit` and breaks the loop.

### 2. Tokenise — `tokenize()`

The lexer converts the raw input string into a flat `t_vec` of `t_token*`. Each token has:

- `type` — one of `TK_WORD`, `TK_PIPE`, `TK_OR`, `TK_AND`, `TK_RIN`, `TK_ROUT`, `TK_APPEND`, `TK_HEREDOC`, `TK_LPAREN`, `TK_RPAREN`, `TK_SEMI`, `TK_EOF`
- `value` — the raw string (quotes are **preserved** at this stage)

Quote preservation is intentional: the expander needs the original delimiters to determine quoting style. Stripping them here would lose the distinction between `*` (glob) and `'*'` (literal).

### 3. Syntax Check — `check_syntax()`

Validates token sequences before any fork is created:

- Rejects operators at position 0 (`| cmd`, `&& cmd`)
- Rejects consecutive binary operators (`cmd || && cmd`)
- Rejects redirects without a following word (`cmd >`)
- Rejects dangling operators at end of input

Returns `0` on error (exit status set to 2, bash-compatible).

### 4. Parse → AST — `parse()`

Builds a binary tree from the token stream. Grammar (lowest to highest precedence):

```
list  :=  pipe  ( ( '&&' | '||' | ';' )  pipe )*
pipe  :=  cmd   ( '|' cmd )*
cmd   :=  WORD* redir*          (simple command)
       |  '(' list ')'          (subshell)
```

### 5. Execute — `execute()`

Builds an execution visitor and calls `visit()`. Each node type dispatches to its handler. Expansion is **lazy**: `expand_node()` runs inside `xv_cmd` just before the command runs, not at parse time. This means `export FOO=bar && echo $FOO` works correctly: `$FOO` is not expanded during parsing (when it doesn't exist yet), but at the moment `echo` runs (when it does).

---

## Memory Management — The Arena Allocator

**File**: `core/mem.c`

A single 2 MB block (`MEM_CAP = 2097152`) is `malloc`'d at `shell_init()` and lives for the entire session. All per-command allocations use `mem_temp()`:

```c
void *mem_temp(t_mem *m, size_t size)
{
    // align size to 8 bytes
    // if m->used + aligned > m->cap → call mem_grow
    ptr = m->buf + m->used;
    m->used += aligned;
    return ptr;
}
```

If the arena runs out of space during `mem_temp()`, `mem_grow()` is called to expand the arena dynamically.

At the end of each command (after execution), `mem_reset()` resets `m->used = 0` — reclaiming all tokens, AST nodes, expanded strings, and vectors in a single instruction, with zero fragmentation.

**What lives in the arena**: tokens, AST nodes, string builder buffers, vectors (metadata + data arrays), expanded argument strings, envp arrays passed to execve.

**What lives on the heap** (malloc/free): the env hash table (`t_rh_env` and its slots, all `key`/`value` strings), the builtin hash table, the arena buffer itself, readline-returned input lines (freed after each command), the `t_shell` struct.

This split means the hot path (processing one command) has zero `free()` calls and zero risk of use-after-free in temporary data.

---

## Environment — Robin Hood Hash Table

**Files**: `core/env.c`, `core/env_getdel.c`, `core/env_init.c`, `core/env_ops.c`, `core/env_iter.c`

The environment is stored as an **open-addressing hash table** using **Robin Hood hashing**. The key insight of Robin Hood hashing: when inserting a new element and a collision occurs, if the existing occupant has a shorter probe sequence length (PSL) than the new element, they swap. This keeps the maximum PSL bounded and minimises average lookup time.

### Key Constants

| Define | Value | Meaning |
|--------|-------|---------|
| `RHE_INIT` | 64 | Initial slot count |
| `RHE_LN` | 7 | Load factor denominator |
| `RHE_LD` | 10 | Load factor numerator (resize when count*10 > cap*7, i.e. ~70% full) |
| `RH_EMPTY` | -1 | Sentinel PSL value for unused slots |

---

## Implementation Approach: Test-Driven Development

This project was developed using a test-driven implementation methodology. Each feature was built incrementally, with testing and debugging occurring continuously throughout the development process.

### Development Workflow

1. **Feature specification** — Define expected behavior by referencing bash
2. **Implementation** — Write the core logic
3. **Testing** — Run test cases and compare output with bash
4. **Debugging** — Identify and fix discrepancies
5. **Refinement** — Optimize and clean up code

### Testing Strategy

The shell was validated using the [minishell_tester](https://github.com/LucasKuhn/minishell_tester) suite, which provided comprehensive test coverage across all shell features.

```bash
# Clone the tester (run from minishell directory)
git clone https://github.com/LucasKuhn/minishell_tester.git
cd minishell_tester

# Run all tests
./tester

# Run specific test categories
./tester builtins    # Test builtin commands
./tester pipes       # Test pipeline functionality
./tester redirects   # Test I/O redirection
./tester wildcards   # Test wildcard expansion (bonus)
./tester syntax      # Test syntax error handling
```

---

## Problems Solved During Implementation

Throughout development, numerous challenges were encountered and resolved. This section documents the key problems and their solutions.

### 1. Redirect List Ordering

**Problem**: If `cmd->redirs` was built by prepending (`r->next = cmd->redirs; cmd->redirs = r`), the list ended up in reverse source order. When `apply_redirs` walked it, the first source redirect was applied last — so `ls >file1 >file2` sent output to `file1` instead of `file2`.

**Solution**: `push_redir()` now tail-appends each redirect, preserving source order. The last redirect wins — identical to bash behavior.

### 2. Expansion Timing in Pipe Children

**Problem**: In `fork_one()`, if `apply_redirs` ran before `expand_node()`, redirect filenames still contained raw token values including embedded quotes. `open("\"./file\"", ...)` would fail with ENOENT even though the file exists.

**Solution**: The order is now: `expand_node()` → `apply_redirs()`. This ensures filenames are properly processed before opening.

### 3. Empty String Handling

**Problem**: After expanding `$UNSET_VAR`, the result is `""`. Bash removes this from the args list (word-splitting). But `""` (an explicitly quoted empty string) must survive.

**Solution**: After expanding each argument, if the result is empty and the original token did not start with a quote character (`'` or `"`), the empty result is dropped. A token starting with `'` or `"` was explicitly quoted; its result is kept even if empty.

### 4. Robin Hood Hash Table Initialization

**Problem**: `RH_EMPTY` is `-1` (0xFFFFFFFF for int). `calloc` would zero the memory, making `psl = 0`, which is a valid PSL — the table would be corrupt.

**Solution**: Every slot array initialization explicitly sets `psl = RH_EMPTY` in a loop, rather than relying on zeroed memory.

### 5. Quoted Wildcard Protection

**Problem**: After `expand_str` runs, both `'*.c'` (quoted, literal) and `*.c` (unquoted, glob) expand to the same string `*.c`. `glob_expand_vec` cannot tell them apart.

**Solution**: `exp_squote` and `exp_dquote` replace every `*` with `\x01` (ASCII SOH — a byte that never appears in normal shell input). `glob_expand_vec` only triggers expansion when it finds a real `*` (ASCII 42). Before returning non-globbed tokens, `unescape_glob()` converts `\x01` back to `*`.

### 6. Heredoc Temp File Cleanup

**Problem**: Heredoc temp files needed cleanup even if the child exits abnormally.

**Solution**: The heredoc temp file is `unlink`'d while the read fd is still open (the "open then unlink" trick). This leaves the file data accessible through the fd but removes it from the filesystem — no cleanup code required even if the child exits abnormally.

### 7. Signal Handler Safety

**Problem**: `rl_on_new_line` / `rl_redisplay` are not async-signal-safe. Calling them from a signal handler causes undefined behaviour.

**Solution**: The signal handler only sets `g_signal` and calls `write`. The main loop calls readline functions only after `readline()` returns, in a safe context.

### 8. Redirect-Only Commands

**Problem**: `cmd >file` with no command name is a valid bash construct (it opens and truncates `file`).

**Solution**: `exec_cmd` detects this case (args vector is empty) and applies + immediately restores redirects rather than trying to execute a NULL command name.

### 9. execve Error Distinction

**Problem**: Different execve errors require different exit codes and messages.

**Solution**:
- `ENOEXEC` → exit 127 "command not found"
- `EACCES` + stat shows directory → exit 126 "is a directory"
- Other errors → exit 126 + `strerror(errno)`

### 10. exit Builtin Edge Cases

**Problem**: `exit a b` emits an error but should not exit the shell.

**Solution**: `exit` with too many arguments returns 1 without setting `sh->running = 0`. The shell keeps running. Only single-argument or no-argument `exit` actually exits.

### 11. Arena Pointer Lifetime

**Problem**: Pointers into the arena become invalid after `mem_reset()`.

**Solution**: Never hold a pointer into the arena across a `mem_reset()` call. Anything that must survive across commands (env strings, builtin table) is on the heap via `malloc`/`ft_strdup`.

### 12. Ctrl-C During Heredoc

**Problem**: SIGINT during heredoc input needed to abort cleanly.

**Solution**: The heredoc read loop checks `!g_signal` as a termination condition. When Ctrl-C fires, `g_signal` is set to `SIGINT`, the loop exits, and the heredoc is abandoned.

---

## Key Technical Choices

### Why Arena Allocation?

The arena allocator was chosen for several reasons:

1. **O(1) cleanup** — Single instruction to reset after each command
2. **Zero fragmentation** — All temporary data in contiguous memory
3. **Simplified lifetime management** — No tracking of individual allocations
4. **Performance** — No repeated `malloc`/`free` calls in the hot path

### Why Robin Hood Hashing?

Robin Hood hashing was selected for the environment table because:

1. **Bounded probe lengths** — The Robin Hood invariant keeps worst-case lookup predictable
2. **Cache-friendly** — Open addressing with good locality
3. **Simple deletion** — No tombstones needed, just backshift

### Why Lazy Expansion?

Variable expansion happens at execution time, not parse time, because:

1. **Correctness** — `export FOO=bar && echo $FOO` works as expected
2. **Environment changes** — Variables set during execution are visible to subsequent commands
3. **Pipeline semantics** — Each command in a pipeline sees the correct environment state

---

## Testing and Validation

### Test Suite Coverage

**146/146 Tests Passing**

Complete validation against official 42 tester suite.

| Category | Tests |
|----------|-------|
| Builtins | 25 |
| Pipes | 20 |
| Redirects | 30 |
| Extras | 25 |
| Wildcards | 17 |
| Syntax | 15 |
| OS-specific | 14 |

### Performance Characteristics

- **Arena size**: Maximum 2MB heap usage for any command
- **Leak-free**: No memory leaks detectable by valgrind
- **Startup**: Shell initialization < 100ms
- **Command dispatch**: O(1) builtin lookup and dispatch
- **Variable lookup**: Average O(1) environment variable access

---

## Future Improvements

### Potential Enhancements

- **Word splitting**: Full POSIX `$IFS` word splitting for variable values
- **Job control**: Background processes with `&`, job management with `jobs`/`fg`/`bg`
- **Command history**: Persistent history across sessions
- **Tab completion**: Path and command completion
- **Script execution**: Support for `#!/bin/sh` shebang and script files
- **Advanced builtins**: `alias`, `set`, `unset`, `readonly`, `trap`
- **Signal handling**: More robust signal management and job control signals

---

## License

This project is part of the 42 School curriculum. All code is original implementation following the project specifications.

---

## Acknowledgments

**Authors:** alaziz <alaziz@student.42luxembourg.lu>, xxxxxx

Special thanks to the **42 School** for the comprehensive curriculum and project specifications, **Lucas Kuhn** for the excellent [minishell_tester](https://github.com/LucasKuhn/minishell_tester) suite, **GNU Readline** for interactive line editing capabilities, and **Bash** developers for establishing the shell behavior standards.

Special thanks to the 42 community for sharing knowledge and debugging insights throughout the development process.
