/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bi_unset.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** bi_unset: implements `unset`. For each argument: validates identifier, calls
** env_del. Emits error for invalid identifiers but continues processing
** remaining args.
*/
int	bi_unset(char **args, void *ctx)
{
	t_shell	*sh;
	int		i;

	sh = (t_shell *)ctx;
	i = 1;
	while (args[i])
	{
		if (!ft_valid_id(args[i]))
		{
			err_set(sh, E_MISUSE, "unset", "not a valid identifier");
			err_emit(sh);
		}
		else
			env_del(sh->env, args[i]);
		i++;
	}
	return (0);
}
