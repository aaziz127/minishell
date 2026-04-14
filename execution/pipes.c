/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   pipes.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** collect_cmds: recursively counts or collects the leaf command nodes of a
** pipe chain. If out is NULL, only increments *i (counting pass). If out is
** non-NULL, stores nodes (collecting pass).
*/
static void	collect_cmds(t_node *nd, t_node **out, int *i)
{
	int	n;

	if (!nd)
		return ;
	if (nd->type != ND_PIPE)
	{
		if (out)
			out[(*i)++] = nd;
		else
			(*i)++;
		return ;
	}
	n = *i;
	collect_cmds(nd->left, out, i);
	collect_cmds(nd->right, out, i);
	(void)n;
}

/*
** pipe_exec_cmd: inside a child process: looks up args[0] in the builtin table
** (calls exit(fn())) or finds the executable and calls run_child.
*/
static void	pipe_exec_cmd(char **args, t_shell *sh)
{
	t_builtin_slot	*slot;
	char			**envp;
	char			*path;

	slot = builtin_ht_get(sh->builtins, args[0]);
	if (slot)
		exit(slot->fn(args, (void *)sh));
	path = find_exec(args[0], sh);
	if (!path)
	{
		write(STDERR_FILENO, "minishell: ", 11);
		write(STDERR_FILENO, args[0], ft_strlen(args[0]));
		write(STDERR_FILENO, ": command not found\n", 20);
		exit(127);
	}
	envp = env_to_envp(sh->env, &sh->mem);
	run_child(args, envp, path);
}

/*
** fork_one: forks one stage of a pipeline. Child: sets up pipe dup2s for
** stage i (read from fds[2*(i-1)], write to fds[2*i+1]), closes all raw fds,
** expands node, glob-expands args, applies redirects, executes. Parent: returns
** child pid.
*/
static pid_t	fork_one(t_node *cmd, t_pipe_ctx ctx, int i, t_shell *sh)
{
	pid_t	pid;
	char	**args;

	pid = fork();
	if (pid != 0)
		return (pid);
	setup_child_sigs();
	if (i > 0)
		dup2(ctx.fds[2 * (i - 1)], STDIN_FILENO);
	if (i < ctx.n - 1)
		dup2(ctx.fds[2 * i + 1], STDOUT_FILENO);
	i = 0;
	while (i < 2 * (ctx.n - 1))
		close(ctx.fds[i++]);
	expand_node(cmd, sh);
	if (!cmd->cmd)
		exit(0);
	cmd->cmd->args = glob_expand_vec(cmd->cmd->args, sh);
	if (cmd->cmd->redirs && !apply_redirs(cmd->cmd->redirs, sh))
		exit(1);
	args = vec_to_arr(cmd->cmd->args, &sh->mem);
	if (!args[0] || !args[0][0])
		exit(0);
	pipe_exec_cmd(args, sh);
	exit(127);
}

/*
** pipe_run_forks: forks all n pipeline stages by calling fork_one for each,
** then closes all parent copies of pipe fds.
*/
static void	pipe_run_forks(t_node **cmds, t_pipe_ctx ctx, t_shell *sh)
{
	int	i;

	i = 0;
	while (i < ctx.n)
	{
		ctx.pids[i] = fork_one(cmds[i], ctx, i, sh);
		i++;
	}
	i = 0;
	while (i < 2 * (ctx.n - 1))
		close(ctx.fds[i++]);
}

/*
** exec_pipe: orchestrates a pipeline. Counts commands, allocates arrays for
** nodes/fds/pids from arena. Creates n-1 pipe pairs. Calls pipe_run_forks.
** Waits for all children; returns exit status of the LAST command.
*/
int	exec_pipe(t_node *nd, t_shell *sh)
{
	t_node		**cmds;
	t_pipe_ctx	ctx;
	int			i;

	ctx.n = 0;
	collect_cmds(nd, NULL, &ctx.n);
	cmds = (t_node **)mem_temp(&sh->mem, (size_t)ctx.n * sizeof(t_node *));
	ctx.fds = (int *)mem_temp(&sh->mem,
			(size_t)(2 * (ctx.n - 1)) * sizeof(int));
	ctx.pids = (pid_t *)mem_temp(&sh->mem, (size_t)ctx.n * sizeof(pid_t));
	i = 0;
	collect_cmds(nd, cmds, &i);
	i = 1;
	while (i < ctx.n)
	{
		if (pipe(ctx.fds + 2 * (i - 1)) < 0)
			return (1);
		i++;
	}
	pipe_run_forks(cmds, ctx, sh);
	i = 0;
	while (i < ctx.n - 1)
		waitpid(ctx.pids[i++], NULL, 0);
	return (wait_child(ctx.pids[ctx.n - 1]));
}
