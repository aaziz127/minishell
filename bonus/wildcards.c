/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   wildcards.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** match_pat: recursive glob pattern matcher for '*' only.
** @pat: pattern string, may contain '*' wildcards.
** @str: filename string to test.
**
** Returns 1 if the filename matches the pattern, 0 otherwise.
** This matcher treats '*' as zero or more arbitrary characters.
*/
int	match_pat(const char *pat, const char *str)
{
	if (!*pat && !*str)
		return (1);
	if (*pat == '*')
	{
		while (*pat == '*')
			pat++;
		if (!*pat)
			return (1);
		while (*str)
		{
			if (match_pat(pat, str))
				return (1);
			str++;
		}
		return (0);
	}
	if (!*str || *pat != *str)
		return (0);
	return (match_pat(pat + 1, str + 1));
}

/*
** sort_matches: simple insertion sort on a vec of (char *) by ft_strcmp.
** Bash returns glob matches in alphabetical order; readdir order is
** non-deterministic, so we must sort before appending to the result.
*/
static void	sort_matches(t_vec *v)
{
	int		i;
	int		j;
	void	*tmp;

	i = 1;
	while (i < vec_len(v))
	{
		j = i;
		while (j > 0
			&& ft_strcmp((char *)v->data[j - 1], (char *)v->data[j]) > 0)
		{
			tmp = v->data[j - 1];
			v->data[j - 1] = v->data[j];
			v->data[j] = tmp;
			j--;
		}
		i++;
	}
}

/*
** unescape_glob: replace the sentinel \x01 (used to mark a literal '*'
** that was inside single or double quotes) back to a real '*'.
** Called on any token that does NOT get glob-expanded.
*/
static char	*unescape_glob(const char *s, t_shell *sh)
{
	t_strbuf	sb;

	sb_init(&sb, &sh->mem, 64);
	while (*s)
	{
		if (*s == '\x01')
			sb_push_c(&sb, '*');
		else
			sb_push_c(&sb, *s);
		s++;
	}
	return (sb_finish(&sb));
}

/*
** collect_matches: reads the current directory and collects filenames
** matching the glob pattern pat.
** Hidden entries (names starting with '.') are excluded.
** Results are returned in a sorted vector to preserve bash-like ordering.
*/
static t_vec	*collect_matches(const char *pat, t_shell *sh)
{
	DIR				*dir;
	struct dirent	*entry;
	t_vec			*matches;

	matches = vec_new(&sh->mem, 8);
	dir = opendir(".");
	if (!dir)
		return (matches);
	entry = readdir(dir);
	while (entry)
	{
		if (entry->d_name[0] != '.' && match_pat(pat, entry->d_name))
			vec_push(matches, str_dup_arena(&sh->mem, entry->d_name));
		entry = readdir(dir);
	}
	closedir(dir);
	sort_matches(matches);
	return (matches);
}

/*
** glob_expand_vec: expands '*' patterns in argument vector args.
** For each arg:
**   - if it contains no '*', the value is unescaped and kept verbatim.
**   - if it contains '*', the current directory is searched for matches.
**   - if no matches are found, the original pattern is preserved.
**
** Returns a new argument vector allocated from the arena.
*/
t_vec	*glob_expand_vec(t_vec *args, t_shell *sh)
{
	t_vec	*result;
	t_vec	*matches;
	char	*arg;
	int		i;
	int		j;

	result = vec_new(&sh->mem, vec_len(args));
	i = 0;
	while (i < vec_len(args))
	{
		arg = (char *)args->data[i++];
		if (!ft_strchr(arg, '*'))
		{
			vec_push(result, unescape_glob(arg, sh));
			continue ;
		}
		matches = collect_matches(arg, sh);
		if (vec_len(matches) == 0)
			vec_push(result, unescape_glob(arg, sh));
		j = 0;
		while (j < vec_len(matches))
			vec_push(result, matches->data[j++]);
	}
	return (result);
}
