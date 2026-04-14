/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mem.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** align_up - Rounds size up to next MEM_ALIGN (8-byte) boundary using bitmask.
** @size: The size to align.
**
** Returns the aligned size_t value.
*/
static size_t	align_up(size_t size)
{
	return ((size + MEM_ALIGN - 1) & ~(size_t)(MEM_ALIGN - 1));
}

/*
** mem_init - Allocates the 2 MB arena heap block and initialises used=0, cap.
** @m: Pointer to the t_mem structure to initialise.
**
** Calls err_fatal on malloc failure. Returns void.
*/
void	mem_init(t_mem *m)
{
	m->buf = (unsigned char *)malloc(MEM_CAP);
	if (!m->buf)
		err_fatal("mem_init", "arena allocation failed");
	m->used = 0;
	m->cap = MEM_CAP;
}

/*
** mem_temp - Bump-allocates `size` bytes from the arena, aligned to 8 bytes.
** @m: Pointer to the t_mem structure.
** @size: Number of bytes to allocate.
**
** If the arena lacks space, mem_grow() is called to double capacity.
** On allocation failure after growth, calls err_fatal.
** Returns a void* pointer into the arena.
*/
void	*mem_temp(t_mem *m, size_t size)
{
	size_t	aligned;
	void	*ptr;

	aligned = align_up(size);
	if (m->used + aligned > m->cap)
	{
		if (mem_grow(m) < 0)
			err_fatal("mem_temp", "arena exhausted");
	}
	ptr = m->buf + m->used;
	m->used += aligned;
	return (ptr);
}

/*
** mem_reset - Resets used to 0, reclaiming all arena memory in O(1).
** @m: Pointer to the t_mem structure.
**
** Does NOT free the underlying buffer. Returns void.
*/
void	mem_reset(t_mem *m)
{
	m->used = 0;
}

/*
** mem_destroy - Frees the underlying heap block and zeroes all fields.
** @m: Pointer to the t_mem structure.
**
** Returns void.
*/
void	mem_destroy(t_mem *m)
{
	free(m->buf);
	m->buf = NULL;
	m->used = 0;
	m->cap = 0;
}
