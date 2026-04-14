/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_str.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** ft_strlen: returns the length of string s, or 0 if s is NULL.
*/
int	ft_strlen(const char *s)
{
	int	i;

	if (!s)
		return (0);
	i = 0;
	while (s[i])
		i++;
	return (i);
}

/*
** ft_strcmp: compares strings a and b. Returns negative/zero/positive like
** strcmp.
*/
int	ft_strcmp(const char *a, const char *b)
{
	int	i;

	i = 0;
	while (a[i] && b[i] && a[i] == b[i])
		i++;
	return ((unsigned char)a[i] - (unsigned char)b[i]);
}

/*
** ft_strchr: returns pointer to first occurrence of c in s (including the null
** terminator). Returns NULL if not found.
*/
char	*ft_strchr(const char *s, int c)
{
	int	i;

	i = 0;
	while (s[i])
	{
		if ((unsigned char)s[i] == (unsigned char)c)
			return ((char *)s + i);
		i++;
	}
	if (c == '\0')
		return ((char *)s + i);
	return (NULL);
}

/*
** ft_strdup: heap-allocates a copy of s using malloc. Returns NULL if s is
** NULL or malloc fails.
*/
char	*ft_strdup(const char *s)
{
	char	*dst;
	int		len;
	int		i;

	if (!s)
		return (NULL);
	len = ft_strlen(s);
	dst = (char *)malloc((size_t)(len + 1));
	if (!dst)
		return (NULL);
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
** ft_memcpy: copies n bytes from src to dest.
** @dest: Pointer to the destination buffer.
** @src: Pointer to the source buffer.
** @n: Number of bytes to copy.
**
** Returns a pointer to dest. Returns NULL if either dest or src is NULL.
*/
char	*ft_memcpy(void *dest, const void *src, size_t n)
{
	size_t	i;

	if (!dest || !src)
		return (NULL);
	i = 0;
	while (i < n)
	{
		((unsigned char *)dest)[i] = ((const unsigned char *)src)[i];
		i++;
	}
	return (dest);
}
