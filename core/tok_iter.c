/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tok_iter.c                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 02:17:39 by alaziz            #+#    #+#             */
/*   Updated: 2026/04/09 03:00:36 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** tok_iter_mk: initializes a token iterator for sequential parsing.
** The iterator wraps the token vector and maintains the current index.
*/
void	tok_iter_mk(t_tok_iter *it, t_vec *toks)
{
	it->toks = toks;
	it->pos = 0;
}

/*
** tok_peek: returns the current token without advancing the iterator.
** Useful for lookahead decisions in the parser.
*/
t_token	*tok_peek(t_tok_iter *it)
{
	if (it->pos >= it->toks->len)
		return (NULL);
	return ((t_token *)it->toks->data[it->pos]);
}

/*
** tok_next: consumes the current token and advances to the next one.
** Returns NULL when the iterator reaches the end of the token stream.
*/
t_token	*tok_next(t_tok_iter *it)
{
	t_token	*tok;

	if (it->pos >= it->toks->len)
		return (NULL);
	tok = (t_token *)it->toks->data[it->pos];
	it->pos++;
	return (tok);
}

/*
** tok_expect: consumes the next token only if it matches the expected type.
** If the token does not match, it reports a syntax error for the parser.
*/
t_token	*tok_expect(t_tok_iter *it, int type, t_shell *sh)
{
	t_token	*tok;

	tok = tok_peek(it);
	if (!tok || tok->type != type)
	{
		if (tok)
			err_syntax(sh, tok->value);
		else
			err_syntax(sh, "newline");
		return (NULL);
	}
	return (tok_next(it));
}
