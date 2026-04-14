/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   str.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** str_dup_arena - Duplicates string s into the arena allocator m.
** @m: Pointer to the t_mem arena allocator.
** @s: The string to duplicate (may be NULL).
**
** Returns char* pointer into the arena, or NULL if s is NULL.
*/
char	*str_dup_arena(t_mem *m, const char *s)
{
	int		len;
	char	*dst;
	int		i;

	if (!s)
		return (NULL);
	len = ft_strlen(s);
	dst = (char *)mem_temp(m, (size_t)(len + 1));
	i = 0;
	while (s[i])
	{
		dst[i] = s[i];
		i++;
	}
	dst[i] = '\0';
	return (dst);
}

/*
** str_cat3 - Concatenates three strings a+b+c into arena-allocated string.
** @m: Pointer to the t_mem arena allocator.
** @a: First string (may be NULL).
** @b: Second string (may be NULL).
** @c: Third string (may be NULL).
**
** Returns the arena-allocated char*.
*/
char	*str_cat3(t_mem *m, const char *a, const char *b, const char *c)
{
	t_strbuf	sb;

	sb_init(&sb, m, 64);
	sb_push_s(&sb, a);
	sb_push_s(&sb, b);
	sb_push_s(&sb, c);
	return (sb_finish(&sb));
}

/*
** reverse_buf - In-place reversal of buf[0..len-1]. Used by str_itoa.
** @buf: The buffer to reverse.
** @len: The length of the buffer.
**
** Returns void.
*/
static void	reverse_buf(char *buf, int len)
{
	int		j;
	int		k;
	char	tmp;

	j = 0;
	k = len - 1;
	while (j < k)
	{
		tmp = buf[j];
		buf[j] = buf[k];
		buf[k] = tmp;
		j++;
		k--;
	}
}

/*
** str_itoa - Converts integer n to its decimal string representation.
** @m: Pointer to the t_mem arena allocator.
** @n: The integer to convert (handles negative values).
**
** Returns arena-allocated char*.
*/
char	*str_itoa(t_mem *m, int n)
{
	char	buf[24];
	int		i;
	int		neg;
	long	num;

	i = 0;
	neg = 0;
	num = (long)n;
	if (num < 0)
	{
		neg = 1;
		num = -num;
	}
	if (num == 0)
		buf[i++] = '0';
	while (num > 0)
	{
		buf[i++] = (char)('0' + (num % 10));
		num /= 10;
	}
	if (neg)
		buf[i++] = '-';
	buf[i] = '\0';
	reverse_buf(buf, i);
	return (str_dup_arena(m, buf));
}
