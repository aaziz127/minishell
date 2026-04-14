/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_init.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** env_new - Allocates a new t_rh_env with `cap` slots.
** @cap: The initial capacity (number of slots).
**
** Initialises all slots: key=NULL, value=NULL, psl=RH_EMPTY.
** Returns t_rh_env* or NULL on malloc failure.
*/
t_rh_env	*env_new(int cap)
{
	t_rh_env	*e;
	int			i;

	e = (t_rh_env *)malloc(sizeof(t_rh_env));
	if (!e)
		return (NULL);
	e->cap = cap;
	e->count = 0;
	e->slots = (t_env_slot *)malloc((size_t)cap * sizeof(t_env_slot));
	if (!e->slots)
	{
		free(e);
		return (NULL);
	}
	i = 0;
	while (i < cap)
	{
		e->slots[i].key = NULL;
		e->slots[i].value = NULL;
		e->slots[i].psl = RH_EMPTY;
		i++;
	}
	return (e);
}

/*
** env_free - Frees all key/value strings and the slots array, then the struct.
** @e: Pointer to the t_rh_env to free.
**
** No-op if e is NULL. Returns void.
*/
void	env_free(t_rh_env *e)
{
	int	i;

	if (!e)
		return ;
	i = 0;
	while (i < e->cap)
	{
		if (e->slots[i].psl != RH_EMPTY)
		{
			free(e->slots[i].key);
			free(e->slots[i].value);
		}
		i++;
	}
	free(e->slots);
	free(e);
}

/*
** parse_entry - Parses one "KEY=VALUE" or "KEY" string and calls env_set.
** @e: Pointer to the t_rh_env to populate.
** @entry: The "KEY=VALUE" or "KEY" entry string from envp.
**
** Splits on the first '='. Returns void.
*/
static void	parse_entry(t_rh_env *e, char *entry)
{
	char	*eq;
	char	key[256];
	int		i;

	eq = ft_strchr(entry, '=');
	if (!eq)
	{
		env_set(e, entry, NULL, 1);
		return ;
	}
	i = 0;
	while (entry + i < eq && i < 255)
	{
		key[i] = entry[i];
		i++;
	}
	key[i] = '\0';
	env_set(e, key, eq + 1, 1);
}

/*
** env_init - Populates hash table e from a NULL-terminated envp array.
** @e: Pointer to the t_rh_env to populate.
** @envp: The environment array as received by main (NULL-terminated).
**
** Calls parse_entry for each entry. Returns void.
*/
void	env_init(t_rh_env *e, char **envp)
{
	int	i;

	if (!envp)
		return ;
	i = 0;
	while (envp[i])
	{
		parse_entry(e, envp[i]);
		i++;
	}
}
