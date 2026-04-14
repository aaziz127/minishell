/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_ht_free.c                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** Frees all key strings in occupied slots, then frees the slots array
** and the struct itself. No-op if ht is NULL.
*/
void	builtin_ht_free(t_builtin_ht *ht)
{
	int	i;

	if (!ht)
		return ;
	i = -1;
	while (++i < ht->cap)
	{
		if (ht->slots[i].psl != RH_EMPTY)
			free(ht->slots[i].key);
	}
	free(ht->slots);
	free(ht);
}
