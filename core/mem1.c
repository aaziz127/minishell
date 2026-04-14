/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mem1.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/08 11:32:01 by alaziz            #+#    #+#             */
/*   Updated: 2026/04/09 00:19:48 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** mem_grow - Expands the arena heap by MEM_GROW_FACTOR (×2).
** @m: Pointer to the t_mem structure to grow.
**
** Internally:
**   - malloc(new_cap) where new_cap = m->cap * MEM_GROW_FACTOR
**   - memcpy(new_buf, m->buf, m->used)
**   - free(m->buf), then m->buf = new_buf, m->cap = new_cap
**
** The used counter is unchanged; existing allocations remain valid.
** On failure, the original arena is untouched and -1 is returned.
** Returns 0 on success.
*/
int	mem_grow(t_mem *m)
{
	size_t	new_cap;
	void	*new_buf;

	new_cap = m->cap * MEM_GROW_FACTOR;
	new_buf = malloc(new_cap);
	if (!new_buf)
		return (-1);
	ft_memcpy(new_buf, m->buf, m->used);
	free(m->buf);
	m->buf = new_buf;
	m->cap = new_cap;
	return (0);
}
