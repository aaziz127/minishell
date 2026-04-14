/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** mk_tok: allocates a token in the arena and stores its type/value.
** The returned token is owned by arena memory, not malloc, so it lives
** until the shell frees the arena at command completion.
*/
static t_token	*mk_tok(int type, char *val, t_mem *m)
{
	t_token	*tok;

	tok = (t_token *)mem_temp(m, sizeof(t_token));
	tok->type = type;
	tok->value = val;
	return (tok);
}

/*
** lex_quote: consumes one quoted span and preserves quote characters.
** For single quotes, content is copied literally. For double quotes, backslash
** escapes are honored only for ", \\ and $. The lexer retains the quote
** delimiters so the parser/expander can handle them later.
*/
static void	lex_quote(char **s, t_strbuf *sb, char delim)
{
	sb_push_c(sb, **s);
	(*s)++;
	while (**s && **s != delim)
	{
		if (delim == '"' && **s == '\\' && (*s)[1]
			&& ft_strchr("\"\\$", (*s)[1]))
			(*s)++;
		sb_push_c(sb, **s);
		(*s)++;
	}
	if (**s == delim)
	{
		sb_push_c(sb, **s);
		(*s)++;
	}
}

/*
** lex_word: reads a TK_WORD token from input.
** Embedded quoted spans are preserved as part of the word token, so later
** expansion can process quoted content correctly.
*/
static t_token	*lex_word(char **s, t_mem *m)
{
	t_strbuf	sb;

	sb_init(&sb, m, 64);
	while (**s && !ft_isspace(**s) && !is_op_char(**s))
	{
		if (**s == '\'')
			lex_quote(s, &sb, '\'');
		else if (**s == '"')
			lex_quote(s, &sb, '"');
		else
		{
			sb_push_c(&sb, **s);
			(*s)++;
		}
	}
	return (mk_tok(TK_WORD, sb_finish(&sb), m));
}

/*
** lex_op: reads operator token characters based on type
*/
static t_token	*lex_op(char **s, t_mem *m)
{
	t_strbuf	sb;
	int			type;
	int			len;
	int			i;

	type = classify_op(*s);
	len = op_len(type);
	sb_init(&sb, m, 4);
	i = -1;
	while (++i < len)
		sb_push_c(&sb, (*s)[i]);
	*s += len;
	return (mk_tok(type, sb_finish(&sb), m));
}

/*
** tokenize: converts raw input into vector of tokens, appends TK_EOF sentinel
*/
t_vec	*tokenize(char *input, t_shell *sh)
{
	t_vec	*v;
	char	*s;

	v = vec_new(&sh->mem, 16);
	s = input;
	while (*s)
	{
		skip_spaces(&s);
		if (!*s)
			break ;
		if (is_op_char(*s))
			vec_push(v, lex_op(&s, &sh->mem));
		else
			vec_push(v, lex_word(&s, &sh->mem));
	}
	vec_push(v, mk_tok(TK_EOF, "", &sh->mem));
	return (v);
}
