/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** path_next: extracts the next directory segment from PATH-style string.
** It replaces ':' with '\0' in place and advances *p to the following
** segment. This is a helper for PATH search in find_exec.
*/
static char	*path_next(char **p)
{
	char	*start;

	start = *p;
	while (**p && **p != ':')
		(*p)++;
	if (**p == ':')
		*(*p)++ = '\0';
	return (start);
}

/*
** find_exec: resolves a command name to an executable path.
** If cmd contains '/', it is treated as a literal path and tested directly.
** Otherwise the function searches each PATH directory and returns the first
** accessible executable path, or NULL if none is found.
*/
char	*find_exec(const char *cmd, t_shell *sh)
{
	t_env_slot	*ps;
	char		*path_copy;
	char		*dir;
	char		*full;

	if (ft_strchr(cmd, '/'))
	{
		if (access(cmd, F_OK) == 0)
			return (str_dup_arena(&sh->mem, cmd));
		return (NULL);
	}
	ps = env_get(sh->env, "PATH");
	if (!ps || !ps->value)
		return (NULL);
	path_copy = str_dup_arena(&sh->mem, ps->value);
	while (*path_copy)
	{
		dir = path_next(&path_copy);
		full = str_cat3(&sh->mem, dir, "/", cmd);
		if (access(full, X_OK) == 0)
			return (full);
	}
	return (NULL);
}

/*
** wait_child: waits for a child process and interprets its termination.
** On signal termination returns 128+signal, printing shell-compatible
** diagnostics for SIGQUIT/SIGINT. On normal exit returns the child code.
*/
int	wait_child(pid_t pid)
{
	int	status;

	if (waitpid(pid, &status, 0) < 0)
		return (1);
	if (WIFSIGNALED(status))
	{
		if (WTERMSIG(status) == SIGQUIT)
			write(STDERR_FILENO, "Quit (core dumped)\n", 19);
		else if (WTERMSIG(status) == SIGINT)
			write(STDERR_FILENO, "\n", 1);
		return (128 + WTERMSIG(status));
	}
	if (WIFEXITED(status))
		return (WEXITSTATUS(status));
	return (1);
}

/*
** restore_fds: restores the shell's saved original stdin/stdout file
** descriptors after a command modifies them. This prevents redirections
** from leaking into subsequent commands.
*/
void	restore_fds(t_shell *sh)
{
	if (sh->fd_in >= 0)
		dup2(sh->fd_in, STDIN_FILENO);
	if (sh->fd_out >= 0)
		dup2(sh->fd_out, STDOUT_FILENO);
}
