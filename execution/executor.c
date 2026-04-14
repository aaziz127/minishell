/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   executor.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 02:16:50 by alaziz            #+#    #+#             */
/*   Updated: 2026/04/09 03:09:51 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** execute: entry point for AST execution.
** Builds the execution visitor and dispatches the root node through visit().
** Returns the final exit status produced by the AST traversal.
*/
int	execute(t_node *nd, t_shell *sh)
{
	t_visitor	xv;

	if (!nd)
		return (0);
	xv = mk_exec_visitor(sh);
	return (visit(&xv, nd));
}
