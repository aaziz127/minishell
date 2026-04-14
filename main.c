/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "includes/minishell.h"
#include <string.h>
#include <unistd.h>

/*
** handle_sigint_prompt: called from shell_loop when g_signal==SIGINT after
** readline returns. Sets exit_status=130, clears g_signal, calls rl_on_new_line
** so readline redraws cleanly.
*/
static void	handle_sigint_prompt(t_shell *sh)
{
	sh->exit_status = 130;
	g_signal = 0;
	rl_on_new_line();
}

/*
** process_input: processes one non-empty line of user input: adds to readline
** history, tokenises, syntax-checks, parses, executes, restores fds, resets
** arena.
*/
static void	process_input(char *input, t_shell *sh)
{
	t_vec	*toks;
	t_node	*ast;

	add_history(input);
	toks = tokenize(input, sh);
	if (!check_syntax(toks, sh))
	{
		mem_reset(&sh->mem);
		return ;
	}
	ast = parse(toks, sh);
	if (ast)
		sh->exit_status = execute(ast, sh);
	restore_fds(sh);
	mem_reset(&sh->mem);
}

/*
** shell_loop: main REPL loop. Calls readline for a prompt, checks g_signal,
** processes input or handles EOF (Ctrl-D). Clears history on exit.
*/
void	shell_loop(t_shell *sh)
{
	char	*input;

	while (sh->running)
	{
		input = readline("🚀 minishell$ ");
		if (g_signal == SIGINT)
			handle_sigint_prompt(sh);
		if (!input)
		{
			write(STDOUT_FILENO, "exit\n", 5);
			break ;
		}
		if (*input)
			process_input(input, sh);
		free(input);
	}
	rl_clear_history();
}

/*
** display_startup_banner: shows a fancy ASCII art banner
** when minishell starts up. Uses ANSI escape codes for colors.
*/
static void	display_startup_banner(void)
{
	write(STDOUT_FILENO, "\033[2J\033[H\033[1;36m", 15);
	write(STDOUT_FILENO, "========================================\n", 41);
	write(STDOUT_FILENO, "                                        \n", 41);
	write(STDOUT_FILENO, "           🚀  M I N I S H E L L       \n", 41);
	write(STDOUT_FILENO, "                                        \n", 41);
	write(STDOUT_FILENO, "       \"As beautiful as a shell.\"      \n", 41);
	write(STDOUT_FILENO, "                                        \n", 41);
	write(STDOUT_FILENO, "   POSIX-compliant • 42 School Project  \n", 41);
	write(STDOUT_FILENO, "                                        \n", 41);
	write(STDOUT_FILENO, "========================================\n", 41);
	write(STDOUT_FILENO, "\033[0m\n\033[1;33m✨ ✨ ✨ ✨ ✨\n\033[0m\n\n", 32);
	usleep(500000);
}

/*
** main: entry point. Silences unused argc/argv. Initialises shell, runs
** shell_loop, captures exit status, frees shell, returns status.
*/
int	main(int argc, char **argv, char **envp)
{
	t_shell	*sh;
	int		ret;

	(void)argc;
	(void)argv;
	sh = shell_init(envp);
	if (sh->interactive)
		display_startup_banner();
	shell_loop(sh);
	ret = sh->exit_status;
	shell_free(sh);
	return (ret);
}
