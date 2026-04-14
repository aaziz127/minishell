/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vector.c                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 02:17:54 by alaziz            #+#    #+#             */
/*   Updated: 2026/04/09 02:19:52 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** Allocates a t_vec and its data array from arena m with initial capacity cap.
*/
t_vec	*vec_new(t_mem *m, int cap)
{
	t_vec	*v;

	v = (t_vec *)mem_temp(m, sizeof(t_vec));
	v->cap = cap;
	v->len = 0;
	v->data = (void **)mem_temp(m, (size_t)cap * sizeof(void *));
	return (v);
}

/*
** Doubles capacity by allocating a new heap array (malloc, NOT arena),
** copies existing items. Called when arena-backed vec overflows.
*/
static void	vec_grow_heap(t_vec *v)
{
	void	**nd;
	int		i;
	int		nc;

	nc = v->cap * 2;
	nd = (void **)malloc((size_t)nc * sizeof(void *));
	if (!nd)
		err_fatal("vec_push", "allocation failed");
	i = 0;
	while (i < v->len)
	{
		nd[i] = v->data[i];
		i++;
	}
	v->data = nd;
	v->cap = nc;
}

/*
** Appends item to v. Grows via vec_grow_heap if full.
*/
void	vec_push(t_vec *v, void *item)
{
	if (v->len >= v->cap)
		vec_grow_heap(v);
	v->data[v->len++] = item;
}

/*
** Returns v->len, or 0 if v is NULL.
*/
int	vec_len(const t_vec *v)
{
	if (!v)
		return (0);
	return (v->len);
}

/*
** Converts vec to NULL-terminated char** array allocated from arena m.
*/
char	**vec_to_arr(t_vec *v, t_mem *m)
{
	char	**arr;
	int		i;

	arr = (char **)mem_temp(m, (size_t)(v->len + 1) * sizeof(char *));
	i = 0;
	while (i < v->len)
	{
		arr[i] = (char *)v->data[i];
		i++;
	}
	arr[v->len] = NULL;
	return (arr);
}
