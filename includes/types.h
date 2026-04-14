/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   types.h                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2026/04/08 23:24:49 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TYPES_H
# define TYPES_H

# include <stddef.h>
# include <signal.h>
/*
** Memory management configuration.
** MEM_CAP   — Total arena capacity (2 MB per shell).
** MEM_ALIGN — Alignment boundary for allocations.
** RHE_INIT  — Initial capacity for environment hash table.
** RHE_LN    — Load factor numerator (7/10 = 0.7).
** RHE_LD    — Load factor denominator.
** RH_EMPTY  — Sentinel value for empty hash slot.
*/
# define MEM_CAP     2097152
# define MEM_GROW_FACTOR 2
# define MEM_ALIGN   8
# define RHE_INIT    64
# define RHE_LN      7
# define RHE_LD      10
# define RH_EMPTY    -1

/*
** AST node types for command syntax tree.
** ND_CMD   — Leaf node: a simple command.
** ND_PIPE  — Operator: pipe | between commands.
** ND_AND   — Operator: && logical AND.
** ND_OR    — Operator: || logical OR.
** ND_SUB   — Subshell: ( ... ).
** ND_COUNT — Total number of node types.
*/
# define ND_CMD      0
# define ND_PIPE     1
# define ND_AND      2
# define ND_OR       3
# define ND_SUB      4
# define ND_COUNT    5

/*
** Token types from lexer.
** TK_WORD   — Alphanumeric word or quoted string.
** TK_PIPE   — | operator.
** TK_OR     — || operator.
** TK_AND    — && operator.
** TK_RIN    — < input redirection.
** TK_ROUT   — > output redirection.
** TK_APPEND — >> append redirection.
** TK_HEREDOC — << here-document.
** TK_LPAREN — ( left parenthesis.
** TK_RPAREN — ) right parenthesis.
** TK_SEMI   — ; semicolon.
** TK_EOF    — End of input.
*/
# define TK_WORD     0
# define TK_PIPE     1
# define TK_OR       2
# define TK_AND      3
# define TK_RIN      4
# define TK_ROUT     5
# define TK_APPEND   6
# define TK_HEREDOC  7
# define TK_LPAREN   8
# define TK_RPAREN   9
# define TK_SEMI     10
# define TK_EOF      11

/*
** Redirection types.
** RD_IN     — Standard input (< file).
** RD_OUT    — Standard output (> file).
** RD_APPEND — Append to file (>> file).
** RD_HEREDOC — Here-document (<< delim).
*/
# define RD_IN       0
# define RD_OUT      1
# define RD_APPEND   2
# define RD_HEREDOC  3

/*
** t_err_code: Exit codes and error conditions.
** E_OK     — Success (0).
** E_MALLOC — Memory allocation failure (1).
** E_FORK   — Process fork failure (2).
** E_PIPE   — Pipe creation failure (3).
** E_SYNTAX — Command syntax error (4).
** E_MISUSE — Command misuse (2).
** E_NOTFND — Command not found (127).
** E_NOPERM — Permission denied (126).
*/
typedef enum e_err_code
{
	E_OK = 0,
	E_MALLOC = 1,
	E_FORK = 2,
	E_PIPE = 3,
	E_SYNTAX = 4,
	E_MISUSE = 2,
	E_NOTFND = 127,
	E_NOPERM = 126,
}	t_err_code;

/*
** t_mem: Bump-pointer arena allocator.
** buf  — Base pointer of the 2 MB heap block.
** used — Current offset; bytes consumed so far.
** cap  — Total capacity (always MEM_CAP).
*/
typedef struct s_mem
{
	unsigned char	*buf;
	size_t			used;
	size_t			cap;
}	t_mem;

/*
** t_error: Error state and context.
** code — Error code (t_err_code).
** ctx  — Context where error occurred (64 chars max).
** msg  — Detailed error message (192 chars max).
*/
typedef struct s_error
{
	t_err_code	code;
	char		ctx[64];
	char		msg[192];
}	t_error;

/*
** t_env_slot: Single environment variable entry.
** key      — Variable name (e.g., "PATH").
** value    — Variable value (e.g., "/bin:/usr/bin").
** exported — Whether to include in child process envp.
** psl      — Probe sequence length (hash table internal).
*/
typedef struct s_env_slot
{
	char	*key;
	char	*value;
	int		exported;
	int		psl;
}	t_env_slot;

/*
** t_rh_env: Environment variable hash table with open addressing.
** slots — Array of environment slots.
** cap   — Current capacity of slots array.
** count — Number of occupied slots.
*/
typedef struct s_rh_env
{
	t_env_slot	*slots;
	int			cap;
	int			count;
}	t_rh_env;

/*
** t_builtin_slot: Hash table entry for builtin command.
** key — Command name (e.g., "echo", "cd").
** fn  — Function pointer to execute the builtin.
** psl — Probe sequence length (hash table internal).
*/
typedef struct s_builtin_slot
{
	char	*key;
	int		(*fn)(char **, void *);
	int		psl;
}	t_builtin_slot;

/*
** t_builtin_ht: Hash table for builtin command lookup.
** slots — Array of builtin slots.
** cap   — Current capacity of slots array.
** count — Number of occupied slots.
*/
typedef struct s_builtin_ht
{
	t_builtin_slot	*slots;
	int				cap;
	int				count;
}	t_builtin_ht;

/*
** t_vec: Dynamic array / vector.
** data — Array of void pointers to elements.
** len  — Current number of elements.
** cap  — Allocated capacity.
*/
typedef struct s_vec
{
	void	**data;
	int		len;
	int		cap;
}	t_vec;

/*
** t_strbuf: String buffer backed by arena allocator.
** buf — Character buffer.
** len — Current length.
** cap — Allocated capacity.
** mem — Pointer to parent arena for allocation.
*/
typedef struct s_strbuf
{
	char	*buf;
	int		len;
	int		cap;
	t_mem	*mem;
}	t_strbuf;

/*
** t_redir: Linked list node for input/output redirection.
** type — Redirection type (RD_IN, RD_OUT, RD_APPEND, RD_HEREDOC).
** fd   — Original file descriptor (for restoration).
** file — Target file path or here-document delimiter.
** next — Pointer to next redirection in chain.
*/
typedef struct s_redir
{
	int				type;
	int				fd;
	char			*file;
	struct s_redir	*next;
}	t_redir;

/*
** t_cmd: A parsed command with arguments and redirections.
** args   — Vector of argument strings.
** redirs — Linked list of redirections to apply.
*/
typedef struct s_cmd
{
	t_vec	*args;
	t_redir	*redirs;
}	t_cmd;

/*
** t_node: AST node for command syntax tree.
** type  — Node type (ND_CMD, ND_PIPE, ND_AND, ND_OR, ND_SUB).
** cmd   — Command data (only for ND_CMD nodes).
** left  — Left child (for binary operators and ND_SUB).
** right — Right child (for binary operators).
*/
typedef struct s_node
{
	int				type;
	t_cmd			*cmd;
	struct s_node	*left;
	struct s_node	*right;
}	t_node;

/*
** t_token: Lexer output token.
** type  — Token type (TK_WORD, TK_PIPE, etc.).
** value — Token string value.
*/
typedef struct s_token
{
	int		type;
	char	*value;
}	t_token;

/*
** t_tok_iter: Iterator over token vector.
** toks — Pointer to token vector.
** pos  — Current position in token stream.
*/
typedef struct s_tok_iter
{
	t_vec	*toks;
	int		pos;
}	t_tok_iter;

/*
** t_env_iter: Iterator over environment variables.
** env — Pointer to environment hash table.
** pos — Current slot position.
*/
typedef struct s_env_iter
{
	t_rh_env	*env;
	int			pos;
}	t_env_iter;

struct		s_shell;

/*
** t_exec_fn: Function pointer type for AST node execution.
** Takes an AST node and a context pointer, returns exit status.
*/
typedef int	(*t_exec_fn)(struct s_node *, void *);

/*
** t_visitor: Visitor pattern for AST traversal.
** fn[ND_COUNT] — Function pointers indexed by node type.
** ctx          — Context pointer passed to visitor functions.
*/
typedef struct s_visitor
{
	t_exec_fn	fn[ND_COUNT];
	void		*ctx;
}	t_visitor;

/*
** t_shell: Main shell state structure.
** dispatch[ND_COUNT] — AST node execution dispatch table.
** builtins          — Hash table of builtin commands.
** env               — Environment variables.
** mem               — Arena allocator.
** err               — Current error state.
** exit_status       — Last command exit code.
** running           — Loop control flag.
** fd_in, fd_out     — Saved stdin/stdout for restoration.
** hd_count          — Counter for unique here-document file names.
** interactive       — Flag: running in interactive mode.
*/
typedef struct s_shell
{
	t_exec_fn		dispatch[ND_COUNT];
	t_builtin_ht	*builtins;
	t_rh_env		*env;
	t_mem			mem;
	t_error			err;
	int				exit_status;
	int				running;
	int				fd_in;
	int				fd_out;
	int				hd_count;
	int				interactive;
}	t_shell;

/*
** t_parser: Parser state and context.
** it — Token iterator.
** sh — Pointer to parent shell state.
*/
typedef struct s_parser
{
	t_tok_iter	it;
	t_shell		*sh;
}	t_parser;

/*
** t_pipe_ctx: Pipe execution context.
** fds — Array of pipe file descriptors.
** pids — Array of child process IDs.
** n    — Number of pipes/processes.
*/
typedef struct s_pipe_ctx
{
	int		*fds;
	pid_t	*pids;
	int		n;
}	t_pipe_ctx;

#endif
