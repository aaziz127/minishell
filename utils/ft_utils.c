/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   ft_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** ft_isspace: returns non-zero if c is a whitespace character (space, tab,
** newline, carriage return, vertical tab, form feed).
*/
int	ft_isspace(int c)
{
	return (c == ' ' || c == '\t' || c == '\n'
		|| c == '\r' || c == '\v' || c == '\f');
}

/*
** ft_isalpha: returns non-zero if c is an ASCII letter (a-z or A-Z).
*/
int	ft_isalpha(int c)
{
	return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'));
}

/*
** ft_isdigit: returns non-zero if c is an ASCII digit (0-9).
*/
int	ft_isdigit(int c)
{
	return (c >= '0' && c <= '9');
}

/*
** ft_isalnum: returns non-zero if c is alphanumeric.
*/
int	ft_isalnum(int c)
{
	return (ft_isalpha(c) || ft_isdigit(c));
}

/*
** ft_valid_id: returns 1 if s is a valid shell identifier: starts with a
** letter or underscore, continues with alphanumerics or underscores. Returns
** 0 for empty or invalid strings.
*/
int	ft_valid_id(const char *s)
{
	int	i;

	if (!s || !s[0])
		return (0);
	if (!ft_isalpha(s[0]) && s[0] != '_')
		return (0);
	i = 1;
	while (s[i])
	{
		if (!ft_isalnum(s[i]) && s[i] != '_')
			return (0);
		i++;
	}
	return (1);
}
