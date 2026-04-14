/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bi_env.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** bi_env: implements `env`. Iterates over all env slots via env_iter and
** prints "key=value\n" for each exported slot that has a value.
*/
int	bi_env(char **args, void *ctx)
{
	t_shell		*sh;
	t_env_iter	it;
	t_env_slot	*slot;

	sh = (t_shell *)ctx;
	(void)args;
	it = env_iter_mk(sh->env);
	while (env_iter_has(&it))
	{
		slot = env_iter_next(&it);
		if (slot->exported && slot->value)
		{
			write(STDOUT_FILENO, slot->key, ft_strlen(slot->key));
			write(STDOUT_FILENO, "=", 1);
			write(STDOUT_FILENO, slot->value, ft_strlen(slot->value));
			write(STDOUT_FILENO, "\n", 1);
		}
	}
	return (0);
}
