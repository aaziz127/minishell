/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   minishell.h                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MINISHELL_H
# define MINISHELL_H

# include "types.h"
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <string.h>
# include <errno.h>
# include <fcntl.h>
# include <signal.h>
# include <sys/wait.h>
# include <sys/stat.h>
# include <dirent.h>
# include <termios.h>
# include <readline/readline.h>
# include <readline/history.h>

/*
** NOTE: rl_replace_line is NOT called in this project.
** The signal handler only uses write(2) -- async-signal-safe.
** rl_on_new_line / rl_redisplay are called from shell_loop only,
** never from inside a signal handler.
*/

extern sig_atomic_t	g_signal;

/*
** error.c: Error handling and reporting.
** err_set()    — Set error state with code, context, and message.
** err_emit()   — Output error message to stderr.
** err_clear()  — Reset error state to E_OK.
** err_syntax() — Create syntax error for malformed token.
** err_fatal()  — Print fatal error and exit.
*/
void				err_set(t_shell *sh, t_err_code code,
						const char *ctx, const char *msg);
void				err_emit(t_shell *sh);
void				err_clear(t_shell *sh);
void				err_syntax(t_shell *sh, const char *token);
void				err_fatal(const char *ctx, const char *msg);

/*
** mem.c: Arena allocator (bump-pointer) for command memory.
** mem_init()   — Initialize empty arena (allocate 2 MB heap).
** mem_grow()   — Expand arena capacity.
** mem_temp()   — Allocate size bytes from arena (auto-aligned).
** mem_reset()  — Zero out used bytes, keep heap allocated.
** mem_destroy() — Free the entire heap block.
*/
void				mem_init(t_mem *m);
int					mem_grow(t_mem *m);
void				*mem_temp(t_mem *m, size_t size);
void				mem_reset(t_mem *m);
void				mem_destroy(t_mem *m);

/*
** strbuf.c: Dynamic string buffer backed by arena.
** sb_init()    — Initialize string buffer with initial capacity.
** sb_push_c()  — Append single character.
** sb_push_s()  — Append string (null-terminated).
** sb_finish()  — Finalize and return null-terminated string.
*/
void				sb_init(t_strbuf *sb, t_mem *m, int cap);
void				sb_push_c(t_strbuf *sb, char c);
void				sb_push_s(t_strbuf *sb, const char *s);
char				*sb_finish(t_strbuf *sb);

/*
** str.c: String manipulation utilities.
** str_cat3()      — Concatenate three strings into arena allocation.
** str_itoa()      — Convert integer to string in arena.
** str_dup_arena() — Duplicate string using arena allocator.
*/
char				*str_cat3(t_mem *m, const char *a,
						const char *b, const char *c);
char				*str_itoa(t_mem *m, int n);
char				*str_dup_arena(t_mem *m, const char *s);

/*
** ft_str.c: Standard C string functions (custom implementations).
** ft_strlen()  — Return string length.
** ft_strcmp()  — Compare two strings (returns <0, 0, or >0).
** ft_strchr()  — Find character in string.
** ft_strdup()  — Duplicate string (malloc).
*/
int					ft_strlen(const char *s);
int					ft_strcmp(const char *a, const char *b);
char				*ft_strchr(const char *s, int c);
char				*ft_strdup(const char *s);
char				*ft_memcpy(void *dest, const void *src, size_t n);

/*
** ft_utils.c: Character classification and validation.
** ft_isspace() — True if character is whitespace.
** ft_isalnum() — True if character is alphanumeric.
** ft_isdigit() — True if character is a digit.
** ft_valid_id() — True if string is valid variable identifier.
** ft_isalpha() — True if character is alphabetic.
*/
int					ft_isspace(int c);
int					ft_isalnum(int c);
int					ft_isdigit(int c);
int					ft_valid_id(const char *s);
int					ft_isalpha(int c);

/*
** env.c: Environment variable lookup and modification.
** env_get()  — Look up value by key (returns NULL if not found).
** env_set()  — Add or update variable (exported flag for inheritance).
** env_del()  — Remove variable by key.
*/
t_env_slot			*env_get(t_rh_env *e, const char *key);
void				env_set(t_rh_env *e, const char *key,
						const char *val, int exported);
void				env_del(t_rh_env *e, const char *key);

/*
** env_ops.c: Environment introspection and export.
** env_count()   — Return number of exported variables.
** env_to_envp() — Convert to char** for execve (malloc'd).
** env_print()   — Print all variables to stdout (for 'env' builtin).
*/
int					env_count(const t_rh_env *e);
char				**env_to_envp(const t_rh_env *e, t_mem *m);
void				env_print(const t_rh_env *e);

/*
** env_init.c: Environment initialization from parent process.
** env_new()   — Allocate empty environment hash table.
** env_free()  — Free all slots and table.
** env_init()  — Populate environment from envp array (parent's environment).
*/
t_rh_env			*env_new(int cap);
void				env_free(t_rh_env *e);
void				env_init(t_rh_env *e, char **envp);

/*
** vector.c: Dynamic array implementation.
** vec_new()      — Create new vector with initial capacity.
** vec_push()     — Append item pointer to vector.
** vec_len()      — Return current number of items.
** vec_to_arr()   — Convert vector to malloc'd char** array.
*/
t_vec				*vec_new(t_mem *m, int cap);
void				vec_push(t_vec *v, void *item);
int					vec_len(const t_vec *v);
char				**vec_to_arr(t_vec *v, t_mem *m);

/*
** tok_iter.c: Token stream iterator.
** tok_iter_mk() — Initialize iterator over token vector.
** tok_peek()    — Return current token without advancing.
** tok_next()    — Return current token and advance position.
** tok_expect()  — Consume token of expected type or set syntax error.
*/
void				tok_iter_mk(t_tok_iter *it, t_vec *toks);
t_token				*tok_peek(t_tok_iter *it);
t_token				*tok_next(t_tok_iter *it);
t_token				*tok_expect(t_tok_iter *it, int type, t_shell *sh);

/*
** env_iter.c: Environment variable iterator.
** env_iter_mk()   — Initialize iterator over environment.
** env_iter_has()  — Return true if more variables remain.
** env_iter_next() — Return next variable and advance.
*/
t_env_iter			env_iter_mk(t_rh_env *e);
int					env_iter_has(t_env_iter *it);
t_env_slot			*env_iter_next(t_env_iter *it);

/*
** visitor.c: Visitor pattern for AST traversal.
** visit()                 — Execute visitor on node.
** mk_expand_visitor()     — Create expansion visitor.
** mk_exec_visitor()       — Create execution visitor.
*/
int					visit(t_visitor *v, t_node *nd);
t_visitor			mk_expand_visitor(t_shell *sh);
t_visitor			mk_exec_visitor(t_shell *sh);

/*
** tables_init.c: Initialize dispatcher and builtin tables.
** dispatch_init()    — Set up ND_CMD/ND_PIPE/ND_AND/ND_OR/ND_SUB handlers.
** builtin_ht_init()  — Populate echo, cd, pwd, export, unset, env, exit.
*/
void				dispatch_init(t_shell *sh);
void				builtin_ht_init(t_shell *sh);

/*
** builtin_ht.c: Builtin command hash table.
** builtin_ht_new()  — Create new hash table with capacity.
** builtin_ht_get()  — Look up builtin by name.
** builtin_ht_set()  — Register builtin command handler.
** builtin_ht_free() — Free all slots and table.
*/
t_builtin_ht		*builtin_ht_new(int cap);
t_builtin_slot		*builtin_ht_get(t_builtin_ht *ht, const char *key);
void				builtin_ht_set(t_builtin_ht *ht, const char *key,
						int (*fn)(char **, void *));
void				builtin_ht_free(t_builtin_ht *ht);

/*
** lexer.c: Tokenize input string.
** tokenize() — Split input into token vector (WORD, PIPE, AND, OR, etc.).
*/
t_vec				*tokenize(char *input, t_shell *sh);

/*
** lexer_utils.c: Character classification for lexer.
** is_op_char()  — True if character is operator (| & < > ; ( )).
** skip_spaces() — Advance pointer past whitespace.
** classify_op() — Determine operator type from string.
** op_len()      — Return length of operator (1 or 2).
*/
int					is_op_char(char c);
void				skip_spaces(char **s);
int					classify_op(const char *s);
int					op_len(int type);

/*
** syntax.c: Parse-time syntax validation.
** check_syntax() — Verify token sequence for grammatical errors.
*/
int					check_syntax(t_vec *toks, t_shell *sh);

/*
** parser.c: Parse tokens into AST.
** parse()      — Build AST from token vector.
** parse_list() — Parse top-level expression (pipes, &&, ||).
** mk_node()    — Allocate new AST node.
*/
t_node				*parse(t_vec *toks, t_shell *sh);
t_node				*parse_list(t_parser *p);
t_node				*mk_node(int type, t_shell *sh);

/*
** parser_cmd.c: Parse simple command (ND_CMD node).
** parse_cmd() — Parse command with args and redirections.
** mk_cmd()    — Allocate and initialize command structure.
*/
t_node				*parse_cmd(t_parser *p);
t_cmd				*mk_cmd(t_shell *sh);

/*
** expander.c: Variable and glob expansion.
** expand_str()  — Expand $VAR and wildcards in string.
** expand_node() — Expand all args in command node.
*/
char				*expand_str(const char *s, t_shell *sh);
void				expand_node(t_node *nd, t_shell *sh);

/*
** executor.c: Main execution dispatcher.
** execute() — Execute AST using dispatch table.
*/
int					execute(t_node *nd, t_shell *sh);

/*
** exec_ops.c: Command and subshell execution.
** exec_cmd()     — Execute simple command (fork + execve or builtin).
** exec_sub()     — Execute subshell command.
** run_child()    — Child process execution (execve wrapper).
*/
int					exec_cmd(t_node *nd, t_shell *sh);
int					exec_sub(t_node *nd, t_shell *sh);
int					run_child(char **args, char **envp, const char *path);

/*
** exec_utils.c: Execution utilities.
** find_exec()    — Search PATH for executable.
** wait_child()   — Wait for child process and return exit status.
** restore_fds()  — Restore original stdin/stdout from saved copies.
*/
char				*find_exec(const char *cmd, t_shell *sh);
int					wait_child(pid_t pid);
void				restore_fds(t_shell *sh);

/*
** pipes.c: Pipe operator execution (|).
** exec_pipe() — Execute left | right (create pipes and fork children).
*/
int					exec_pipe(t_node *nd, t_shell *sh);

/*
** redirections.c: Input/output redirection.
** apply_redirs()  — Apply all redirections in chain (dup2 or file ops).
** handle_heredoc() — Create here-document and return file descriptor.
*/
int					apply_redirs(t_redir *r, t_shell *sh);
int					handle_heredoc(const char *delim, t_shell *sh);

/*
** signals.c: Signal handling for interactive shell.
** setup_signals()        — Configure signals for interactive mode.
** setup_child_sigs()     — Reset signals for child process (default behavior).
** setup_heredoc_sigs()   — Configure signals during here-document input.
** sig_handler()          — Handle SIGINT (Ctrl+C) and SIGQUIT.
*/
void				setup_signals(void);
void				setup_child_sigs(void);
void				setup_heredoc_sigs(void);
void				sig_handler(int sig);

/*
** visitor_exec.c: Execution visitor functions (dispatch implementations).
** xv_cmd()  — Execute ND_CMD node (simple command).
** xv_pipe() — Execute ND_PIPE node (pipe operator).
** xv_and()  — Execute ND_AND node (&& conditional).
** xv_or()   — Execute ND_OR node (|| conditional).
** xv_sub()  — Execute ND_SUB node (subshell).
*/
int					xv_cmd(t_node *nd, void *ctx);
int					xv_pipe(t_node *nd, void *ctx);
int					xv_and(t_node *nd, void *ctx);
int					xv_or(t_node *nd, void *ctx);
int					xv_sub(t_node *nd, void *ctx);

/*
** Builtin commands: Shell built-in functions.
** bi_echo()   — Print arguments (with newline).
** bi_cd()     — Change working directory.
** bi_pwd()    — Print working directory.
** bi_export() — Set and export environment variable.
** bi_unset()  — Remove environment variable.
** bi_env()    — Print all environment variables.
** bi_exit()   — Exit shell with optional status.
*/
int					bi_echo(char **args, void *sh);
int					bi_cd(char **args, void *sh);
int					bi_pwd(char **args, void *sh);
int					bi_export(char **args, void *sh);
int					bi_unset(char **args, void *sh);
int					bi_env(char **args, void *sh);
int					bi_exit(char **args, void *sh);

/*
** wildcards.c: Glob pattern expansion.
** glob_expand_vec() — Expand * and ? wildcards in argument vector.
** match_pat()       — Check if string matches glob pattern.
*/
t_vec				*glob_expand_vec(t_vec *args, t_shell *sh);
int					match_pat(const char *pat, const char *str);

/*
** main.c: Shell initialization and main loop.
** shell_init() — Initialize shell state from environment and dispatch table.
** shell_free() — Clean up all allocated memory.
** shell_loop() — Main REPL: readline, parse, execute, repeat.
*/
t_shell				*shell_init(char **envp);
void				shell_free(t_shell *sh);
void				shell_loop(t_shell *sh);

#endif
