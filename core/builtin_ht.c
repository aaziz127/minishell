/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   builtin_ht.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** DJB2 hash function for the builtin hash table (same algorithm as env djb2,
** separate copy for isolation).
*/
static unsigned int	djb2_bt(const char *s)
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
** Allocates a t_builtin_ht with cap Robin Hood slots. All slots initialised
** to key=NULL, fn=NULL, psl=RH_EMPTY. Calls err_fatal on malloc failure.
*/
t_builtin_ht	*builtin_ht_new(int cap)
{
	t_builtin_ht	*ht;
	int				i;

	ht = (t_builtin_ht *)malloc(sizeof(t_builtin_ht));
	if (!ht)
		err_fatal("builtin_ht_new", "malloc failed");
	ht->cap = cap;
	ht->count = 0;
	ht->slots = (t_builtin_slot *)malloc(
			(size_t)cap * sizeof(t_builtin_slot));
	if (!ht->slots)
		err_fatal("builtin_ht_new", "malloc slots failed");
	i = -1;
	while (++i < cap)
	{
		ht->slots[i].key = NULL;
		ht->slots[i].fn = NULL;
		ht->slots[i].psl = RH_EMPTY;
	}
	return (ht);
}

/*
** Looks up key in the builtin hash table. Returns t_builtin_slot* or NULL
** if not found.
*/
t_builtin_slot	*builtin_ht_get(t_builtin_ht *ht, const char *key)
{
	int	h;
	int	psl;

	h = (int)(djb2_bt(key) % (unsigned int)ht->cap);
	psl = 0;
	while (1)
	{
		if (ht->slots[h].psl == RH_EMPTY || ht->slots[h].psl < psl)
			return (NULL);
		if (ft_strcmp(ht->slots[h].key, key) == 0)
			return (&ht->slots[h]);
		h = (h + 1) % ht->cap;
		psl++;
	}
}

/*
** Swaps two t_builtin_slot values. Used during Robin Hood insertion.
*/
static void	bht_swap(t_builtin_slot *a, t_builtin_slot *b)
{
	t_builtin_slot	tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;
}

/*
** Inserts a builtin (key→fn) into the hash table using Robin Hood probing.
*/
void	builtin_ht_set(t_builtin_ht *ht, const char *key,
			int (*fn)(char **, void *))
{
	t_builtin_slot	slot;
	int				h;

	slot.key = ft_strdup(key);
	slot.fn = fn;
	slot.psl = 0;
	h = (int)(djb2_bt(key) % (unsigned int)ht->cap);
	while (1)
	{
		if (ht->slots[h].psl == RH_EMPTY)
		{
			ht->slots[h] = slot;
			ht->count++;
			return ;
		}
		if (ht->slots[h].psl < slot.psl)
			bht_swap(&ht->slots[h], &slot);
		h = (h + 1) % ht->cap;
		slot.psl++;
	}
}
