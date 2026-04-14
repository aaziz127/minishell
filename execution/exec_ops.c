/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   exec_ops.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** run_child: replaces the current process image with the executable.
** If execve fails, the child prints a shell-style error message and exits.
** ENOEXEC is treated as command-not-found. EACCES on a directory is reported
** as "is a directory"; all other failures use strerror(errno).
*/
int	run_child(char **args, char **envp, const char *path)
{
	const char	*msg;
	struct stat	st;

	setup_child_sigs();
	execve(path, args, envp);
	if (errno == ENOEXEC)
	{
		write(STDERR_FILENO, "minishell: ", 11);
		write(STDERR_FILENO, args[0], ft_strlen(args[0]));
		write(STDERR_FILENO, ": command not found\n", 20);
		exit(127);
	}
	if (errno == EACCES && stat(path, &st) == 0 && S_ISDIR(st.st_mode))
		msg = "is a directory";
	else
		msg = strerror(errno);
	write(STDERR_FILENO, "minishell: ", 11);
	write(STDERR_FILENO, args[0], ft_strlen(args[0]));
	write(STDERR_FILENO, ": ", 2);
	write(STDERR_FILENO, msg, ft_strlen(msg));
	write(STDERR_FILENO, "\n", 1);
	exit(126);
}

/*
** fork_exec: forks a child process to run an external command.
** In the child, it applies any redirections before calling run_child.
** The parent waits and returns the child's exit status or signal-based code.
*/
static int	fork_exec(t_node *nd, const char *path, char **envp, t_shell *sh)
{
	pid_t	pid;

	pid = fork();
	if (pid < 0)
	{
		err_set(sh, E_FORK, "fork", strerror(errno));
		err_emit(sh);
		return (1);
	}
	if (pid == 0)
	{
		if (!apply_redirs(nd->cmd->redirs, sh))
			exit(1);
		run_child(vec_to_arr(nd->cmd->args, &sh->mem), envp, path);
	}
	return (wait_child(pid));
}

/*
** exec_external: executes a command node that is not a shell operator.
** It first checks builtin commands and executes them directly with redirs.
** If not built-in, it searches PATH for an executable and uses fork_exec.
** If resolution fails, it reports a shell-style "command not found" error.
*/
static int	exec_external(t_node *nd, t_shell *sh)
{
	char			**args;
	char			*path;
	t_builtin_slot	*slot;

	args = vec_to_arr(nd->cmd->args, &sh->mem);
	if (!args[0] || !args[0][0])
		return (0);
	slot = builtin_ht_get(sh->builtins, args[0]);
	if (slot)
	{
		if (!apply_redirs(nd->cmd->redirs, sh))
			return (1);
		return (slot->fn(args, (void *)sh));
	}
	path = find_exec(args[0], sh);
	if (!path)
	{
		if (ft_strchr(args[0], '/') && errno == ENOENT)
			err_set(sh, E_NOTFND, args[0], "No such file or directory");
		else
			err_set(sh, E_NOTFND, args[0], "command not found");
		err_emit(sh);
		return (127);
	}
	return (fork_exec(nd, path, env_to_envp(sh->env, &sh->mem), sh));
}

/*
** exec_cmd: executes a command AST node.
** If the node has only redirections and no arguments, it applies them in the
** parent shell and returns success or failure. Otherwise it delegates to
** exec_external and restores stdin/stdout afterward.
*/
int	exec_cmd(t_node *nd, t_shell *sh)
{
	int	ret;

	if (!nd || !nd->cmd)
		return (0);
	if (vec_len(nd->cmd->args) == 0 && nd->cmd->redirs)
	{
		if (!apply_redirs(nd->cmd->redirs, sh))
			return (1);
		return (0);
	}
	ret = exec_external(nd, sh);
	restore_fds(sh);
	return (ret);
}

/*
** exec_sub: executes a subshell AST node in a separate process.
** The child runs the nested subtree and exits with its result.
** The parent waits and stores the subshell status in sh->exit_status.
*/
int	exec_sub(t_node *nd, t_shell *sh)
{
	t_visitor	xv;
	pid_t		pid;
	int			ret;

	pid = fork();
	if (pid < 0)
		return (1);
	if (pid == 0)
	{
		setup_child_sigs();
		xv = mk_exec_visitor(sh);
		exit(visit(&xv, nd->left));
	}
	ret = wait_child(pid);
	sh->exit_status = ret;
	return (ret);
}
