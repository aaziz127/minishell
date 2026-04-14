/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** exp_var: handles variable expansion ($VAR or $?)
** Expands $? to exit status, $VAR to environment value.
** Handles malformed variables (lone $, empty name) by preserving literal $.
** Advances input pointer past the expanded variable.
*/
static void	exp_var(const char **s, t_strbuf *sb, t_shell *sh)
{
	char		name[256];
	int			i;
	t_env_slot	*slot;

	(*s)++;
	if (**s == '?')
	{
		sb_push_s(sb, str_itoa(&sh->mem, sh->exit_status));
		(*s)++;
		return ;
	}
	i = 0;
	while (**s && (ft_isalnum(**s) || **s == '_') && i < 255)
		name[i++] = *(*s)++;
	name[i] = '\0';
	if (i == 0)
	{
		sb_push_c(sb, '$');
		return ;
	}
	slot = env_get(sh->env, name);
	if (slot && slot->value)
		sb_push_s(sb, slot->value);
}

/*
** exp_squote: processes single-quoted strings
** Copies content literally, replaces * with sentinel \x01 to protect from glob.
** Preserves all characters including spaces and special chars.
** Advances past closing quote.
*/
static void	exp_squote(const char **s, t_strbuf *sb)
{
	(*s)++;
	while (**s && **s != '\'')
	{
		if (**s == '*')
			sb_push_c(sb, '\x01');
		else
			sb_push_c(sb, **s);
		(*s)++;
	}
	if (**s == '\'')
		(*s)++;
}

/*
** exp_dquote: processes double-quoted strings with expansion
** Expands variables, handles escape sequences (\" \\ \$), protects * from glob.
** Allows variable expansion and limited escaping within quotes.
** Advances past closing quote.
*/
static void	exp_dquote(const char **s, t_strbuf *sb, t_shell *sh)
{
	(*s)++;
	while (**s && **s != '"')
	{
		if (**s == '$')
			exp_var(s, sb, sh);
		else if (**s == '\\' && (*s)[1] && ft_strchr("\"\\$", (*s)[1]))
		{
			(*s)++;
			sb_push_c(sb, *(*s)++);
		}
		else if (**s == '*')
		{
			sb_push_c(sb, '\x01');
			(*s)++;
		}
		else
			sb_push_c(sb, *(*s)++);
	}
	if (**s == '"')
		(*s)++;
}

/*
** expand_str: main expansion dispatcher for strings
** Processes input character by character, dispatching to specialized handlers.
** Handles unquoted text, single quotes, double quotes, and variables.
** Returns arena-allocated expanded string.
*/
char	*expand_str(const char *s, t_shell *sh)
{
	t_strbuf	sb;
	const char	*p;

	sb_init(&sb, &sh->mem, 64);
	p = s;
	while (*p)
	{
		if (*p == '\'')
			exp_squote(&p, &sb);
		else if (*p == '"')
			exp_dquote(&p, &sb, sh);
		else if (*p == '$')
			exp_var(&p, &sb, sh);
		else
			sb_push_c(&sb, *p++);
	}
	return (sb_finish(&sb));
}
