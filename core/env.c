/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env.c                                         :+:      :+:    :+:   */
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
** Used to compute initial slot index for Robin Hood hash table.
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
** rh_ins - Inserts `slot` into the Robin Hood hash table.
** @e: Pointer to the t_rh_env hash table structure.
** @slot: The slot to insert (by value).
**
** Displaces any existing slot with lower PSL (probe sequence length).
** Ensures worst-case probe lengths are minimised. Returns void.
*/
static void	rh_ins(t_rh_env *e, t_env_slot slot)
{
	t_env_slot	tmp;
	int			h;

	h = (int)(djb2(slot.key) % (unsigned int)e->cap);
	while (1)
	{
		if (e->slots[h].psl == RH_EMPTY)
		{
			e->slots[h] = slot;
			return ;
		}
		if (e->slots[h].psl < slot.psl)
		{
			tmp = e->slots[h];
			e->slots[h] = slot;
			slot = tmp;
		}
		h = (h + 1) % e->cap;
		slot.psl++;
	}
}

/*
** update_slot - Updates an existing slot: frees old value, assigns new.
** @s: Pointer to the t_env_slot to update.
** @val: New value string (may be NULL). Will be duplicated.
** @exported: New exported flag value.
**
** Returns void.
*/
static void	update_slot(t_env_slot *s, const char *val, int exported)
{
	free(s->value);
	if (val)
		s->value = ft_strdup(val);
	else
		s->value = NULL;
	s->exported = exported;
}

/*
** env_resize - Doubles the hash table capacity and re-inserts all slots.
** @e: Pointer to the t_rh_env hash table structure.
**
** Allocates new slot array, initialises PSLs to RH_EMPTY, then re-inserts.
** Calls err_fatal on malloc failure. Returns void.
*/
static void	env_resize(t_rh_env *e)
{
	t_env_slot	*old;
	int			old_cap;
	int			i;

	old = e->slots;
	old_cap = e->cap;
	e->cap *= 2;
	e->slots = (t_env_slot *)malloc((size_t)e->cap * sizeof(t_env_slot));
	if (!e->slots)
		err_fatal("env_resize", "malloc failed");
	i = -1;
	while (++i < e->cap)
		e->slots[i].psl = RH_EMPTY;
	i = -1;
	while (++i < old_cap)
	{
		if (old[i].psl != RH_EMPTY)
		{
			old[i].psl = 0;
			rh_ins(e, old[i]);
		}
	}
	free(old);
}

/*
** env_set - Inserts or updates key in the hash table.
** @e: Pointer to the t_rh_env hash table structure.
** @key: The key to set.
** @val: The value to set (may be NULL).
** @exported: Whether the variable is exported.
**
** Triggers resize if load factor exceeds RHE_LD/RHE_LN. If key exists,
** calls update_slot. Otherwise inserts a new slot. Returns void.
*/
void	env_set(t_rh_env *e, const char *key,
			const char *val, int exported)
{
	t_env_slot	*s;
	t_env_slot	slot;

	if ((e->count + 1) * RHE_LD > e->cap * RHE_LN)
		env_resize(e);
	s = env_get(e, key);
	if (s)
	{
		update_slot(s, val, exported);
		return ;
	}
	slot.key = ft_strdup(key);
	if (val)
		slot.value = ft_strdup(val);
	else
		slot.value = NULL;
	slot.exported = exported;
	slot.psl = 0;
	rh_ins(e, slot);
	e->count++;
}
