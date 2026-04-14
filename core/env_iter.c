/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_iter.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** Creates and returns a t_env_iter positioned at the start of hash table e.
*/
t_env_iter	env_iter_mk(t_rh_env *e)
{
	t_env_iter	it;

	it.env = e;
	it.pos = 0;
	return (it);
}

/*
** Advances it->pos past empty slots. Returns 1 if a non-empty slot exists
** at the current position, 0 if iteration is complete.
*/
int	env_iter_has(t_env_iter *it)
{
	while (it->pos < it->env->cap)
	{
		if (it->env->slots[it->pos].psl != RH_EMPTY)
			return (1);
		it->pos++;
	}
	return (0);
}

/*
** Returns pointer to the current slot and increments pos.
** Must only be called after env_iter_has returned 1.
*/
t_env_slot	*env_iter_next(t_env_iter *it)
{
	t_env_slot	*slot;

	slot = &it->env->slots[it->pos];
	it->pos++;
	return (slot);
}
