/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   strbuf.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** sb_grow - Doubles the buffer capacity (from arena) until it fits needed bytes.
** @sb: Pointer to the t_strbuf structure.
** @needed: The number of bytes needed.
**
** Copies existing data into the new buffer. Returns void.
*/
static void	sb_grow(t_strbuf *sb, int needed)
{
	int		new_cap;
	char	*new_buf;
	int		i;

	new_cap = sb->cap * 2;
	while (new_cap < needed)
		new_cap *= 2;
	new_buf = (char *)mem_temp(sb->mem, (size_t)new_cap);
	i = 0;
	while (i < sb->len)
	{
		new_buf[i] = sb->buf[i];
		i++;
	}
	sb->buf = new_buf;
	sb->cap = new_cap;
}

/*
** sb_init - Initialises a string buffer with initial capacity, backed by arena.
** @sb: Pointer to the t_strbuf structure.
** @m: Pointer to the t_mem arena allocator.
** @cap: Initial capacity in bytes.
**
** Returns void.
*/
void	sb_init(t_strbuf *sb, t_mem *m, int cap)
{
	sb->mem = m;
	sb->cap = cap;
	sb->len = 0;
	sb->buf = (char *)mem_temp(m, (size_t)cap);
}

/*
** sb_push_c - Appends one character to the buffer, growing if needed.
** @sb: Pointer to the t_strbuf structure.
** @c: The character to append.
**
** Returns void.
*/
void	sb_push_c(t_strbuf *sb, char c)
{
	if (sb->len + 1 >= sb->cap)
		sb_grow(sb, sb->len + 2);
	sb->buf[sb->len++] = c;
}

/*
** sb_push_s - Appends a null-terminated string to the buffer.
** @sb: Pointer to the t_strbuf structure.
** @s: The string to append (NULL-terminated).
**
** No-op if s is NULL. Returns void.
*/
void	sb_push_s(t_strbuf *sb, const char *s)
{
	int	i;

	if (!s)
		return ;
	i = 0;
	while (s[i])
	{
		sb_push_c(sb, s[i]);
		i++;
	}
}

/*
** sb_finish - Null-terminates the buffer and returns the raw char* pointer.
** @sb: Pointer to the t_strbuf structure.
**
** Does NOT allocate. Pointer is valid for the lifetime of the arena.
** Returns the char* pointer.
*/
char	*sb_finish(t_strbuf *sb)
{
	sb_push_c(sb, '\0');
	sb->len--;
	return (sb->buf);
}
