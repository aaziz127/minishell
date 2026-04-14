/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   shell_init.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** Allocates and fully initialises the t_shell struct: arena, error state,
** env hash table (populated from envp), saved stdin/stdout fds, builtin table,
** dispatch table, signals. Calls err_fatal on any malloc failure.
*/
t_shell	*shell_init(char **envp)
{
	t_shell	*sh;

	sh = (t_shell *)malloc(sizeof(t_shell));
	if (!sh)
		err_fatal("shell_init", "malloc failed");
	mem_init(&sh->mem);
	sh->err.code = E_OK;
	sh->err.ctx[0] = '\0';
	sh->err.msg[0] = '\0';
	sh->exit_status = 0;
	sh->running = 1;
	sh->hd_count = 0;
	sh->interactive = isatty(STDIN_FILENO);
	sh->env = env_new(RHE_INIT);
	if (!sh->env)
		err_fatal("shell_init", "env_new failed");
	env_init(sh->env, envp);
	sh->fd_in = dup(STDIN_FILENO);
	sh->fd_out = dup(STDOUT_FILENO);
	builtin_ht_init(sh);
	dispatch_init(sh);
	setup_signals();
	return (sh);
}

/*
** Frees all heap-allocated shell resources: env, builtins, saved fds, arena.
** No-op if sh is NULL.
*/
void	shell_free(t_shell *sh)
{
	if (!sh)
		return ;
	env_free(sh->env);
	builtin_ht_free(sh->builtins);
	if (sh->fd_in >= 0)
		close(sh->fd_in);
	if (sh->fd_out >= 0)
		close(sh->fd_out);
	mem_destroy(&sh->mem);
	free(sh);
}
