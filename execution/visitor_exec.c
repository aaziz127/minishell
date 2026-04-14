/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   visitor_exec.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** xv_cmd: visitor handler for command nodes.
** Expands the command node, performs wildcard expansion, executes the
** command, and updates shell exit status. This ensures expansion occurs
** immediately before execution, preserving runtime environment semantics.
*/
int	xv_cmd(t_node *nd, void *ctx)
{
	t_shell	*sh;
	int		ret;

	sh = (t_shell *)ctx;
	expand_node(nd, sh);
	nd->cmd->args = glob_expand_vec(nd->cmd->args, sh);
	ret = exec_cmd(nd, sh);
	sh->exit_status = ret;
	return (ret);
}

/*
** xv_pipe: visitor handler for pipeline nodes.
** Executes all stages of the pipe chain and returns the exit status of the
** final command, matching shell semantics where the pipe's return value is
** the last stage's exit code.
*/
int	xv_pipe(t_node *nd, void *ctx)
{
	t_shell	*sh;
	int		ret;

	sh = (t_shell *)ctx;
	ret = exec_pipe(nd, sh);
	sh->exit_status = ret;
	return (ret);
}

/*
** xv_and: visitor handler for logical AND nodes.
** Evaluates the left child first and only executes the right child if the
** left child returns success (0), implementing shell short-circuit logic.
*/
int	xv_and(t_node *nd, void *ctx)
{
	t_shell		*sh;
	t_visitor	xv;
	int			ret;

	sh = (t_shell *)ctx;
	xv = mk_exec_visitor(sh);
	ret = visit(&xv, nd->left);
	sh->exit_status = ret;
	if (ret == 0)
	{
		xv = mk_exec_visitor(sh);
		ret = visit(&xv, nd->right);
		sh->exit_status = ret;
	}
	return (sh->exit_status);
}

/*
** xv_or: visitor handler for logical OR nodes.
** Evaluates the left child first and only executes the right child if the
** left child fails (non-zero), implementing shell short-circuit logic.
*/
int	xv_or(t_node *nd, void *ctx)
{
	t_shell		*sh;
	t_visitor	xv;
	int			ret;

	sh = (t_shell *)ctx;
	xv = mk_exec_visitor(sh);
	ret = visit(&xv, nd->left);
	sh->exit_status = ret;
	if (ret != 0)
	{
		xv = mk_exec_visitor(sh);
		ret = visit(&xv, nd->right);
		sh->exit_status = ret;
	}
	return (sh->exit_status);
}

/*
** xv_sub: visitor handler for subshell nodes.
** Executes the nested command tree in a forked child process and returns
** the child's exit status, preserving subshell isolation.
*/
int	xv_sub(t_node *nd, void *ctx)
{
	t_shell	*sh;
	int		ret;

	sh = (t_shell *)ctx;
	ret = exec_sub(nd, sh);
	sh->exit_status = ret;
	return (ret);
}
