/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   visitor_exec_mk.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** Builds a t_visitor whose handlers are copied from sh->dispatch[].
** Used by execute() to dispatch the AST.
*/
t_visitor	mk_exec_visitor(t_shell *sh)
{
	t_visitor	v;
	int			i;

	v.ctx = sh;
	i = -1;
	while (++i < ND_COUNT)
		v.fn[i] = sh->dispatch[i];
	return (v);
}
