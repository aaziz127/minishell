/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   visitor.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** Dispatches execution of node nd through visitor v.
** Calls v->fn[nd->type](nd, v->ctx). Returns 0 for NULL or unknown nodes.
*/
int	visit(t_visitor *v, t_node *nd)
{
	if (!nd)
		return (0);
	if (nd->type < 0 || nd->type >= ND_COUNT)
		return (0);
	if (!v->fn[nd->type])
		return (0);
	return (v->fn[nd->type](nd, v->ctx));
}

/*
** Expand visitor handler for ND_CMD nodes — calls expand_node.
*/
static int	ev_cmd(t_node *nd, void *ctx)
{
	expand_node(nd, (t_shell *)ctx);
	return (0);
}

/*
** Expand visitor handler for ND_PIPE/ND_AND/ND_OR — recursively visits
** both children with a new expand visitor.
*/
static int	ev_pass(t_node *nd, void *ctx)
{
	t_visitor	v;

	v = mk_expand_visitor((t_shell *)ctx);
	visit(&v, nd->left);
	visit(&v, nd->right);
	return (0);
}

/*
** Expand visitor handler for ND_SUB — visits only the left child
** (the subshell body).
*/
static int	ev_sub(t_node *nd, void *ctx)
{
	t_visitor	v;

	v = mk_expand_visitor((t_shell *)ctx);
	visit(&v, nd->left);
	return (0);
}

/*
** Builds and returns a t_visitor whose handlers call expand_node on every
** ND_CMD. Used to pre-expand nodes before pipe forking.
*/
t_visitor	mk_expand_visitor(t_shell *sh)
{
	t_visitor	v;

	v.ctx = sh;
	v.fn[ND_CMD] = ev_cmd;
	v.fn[ND_PIPE] = ev_pass;
	v.fn[ND_AND] = ev_pass;
	v.fn[ND_OR] = ev_pass;
	v.fn[ND_SUB] = ev_sub;
	return (v);
}
