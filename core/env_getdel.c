/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_getdel.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** djb2 - Dan Bernstein hash function. Computes a 32-bit hash of string s.
** @s: The string to hash.
**
** Uses h = h*33 + c for each character. Returns unsigned int hash value.
** Duplicated here so both compilation units are self-contained.
*/
static unsigned int	djb2(const char *s)
{
	unsigned int	h;
	int				i;

	h = 5381;
	i = 0;
	while (s[i])
		h = ((h << 5) + h) + (unsigned char)s[i++];
	return (h);
}

/*
** env_get - Looks up key in the Robin Hood hash table.
** @e: Pointer to the t_rh_env hash table structure.
** @key: The key to look up.
**
** Returns pointer to matching t_env_slot, or NULL if not found.
** Early-exit when PSL < current probe length (Robin Hood invariant).
*/
t_env_slot	*env_get(t_rh_env *e, const char *key)
{
	int	h;
	int	psl;

	h = (int)(djb2(key) % (unsigned int)e->cap);
	psl = 0;
	while (1)
	{
		if (e->slots[h].psl == RH_EMPTY || e->slots[h].psl < psl)
			return (NULL);
		if (ft_strcmp(e->slots[h].key, key) == 0)
			return (&e->slots[h]);
		h = (h + 1) % e->cap;
		psl++;
	}
}

/*
** env_del - Removes key from the hash table.
** @e: Pointer to the t_rh_env hash table structure.
** @key: The key to delete.
**
** Uses env_get to find the slot. Performs backward shift deletion to restore
** Robin Hood invariant: shifts subsequent slots left while their PSL > 0.
** Returns void.
*/
void	env_del(t_rh_env *e, const char *key)
{
	t_env_slot	*s;
	int			h;
	int			next;

	s = env_get(e, key);
	if (!s)
		return ;
	h = (int)(s - e->slots);
	free(e->slots[h].key);
	free(e->slots[h].value);
	e->slots[h].psl = RH_EMPTY;
	e->slots[h].key = NULL;
	e->slots[h].value = NULL;
	next = (h + 1) % e->cap;
	while (e->slots[next].psl > 0)
	{
		e->slots[h] = e->slots[next];
		e->slots[h].psl--;
		e->slots[next].psl = RH_EMPTY;
		h = next;
		next = (h + 1) % e->cap;
	}
	e->count--;
}
