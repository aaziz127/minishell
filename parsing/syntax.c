/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   syntax.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** is_binop: returns 1 if type is a binary operator token
*/
static int	is_binop(int type)
{
	return (type == TK_PIPE || type == TK_OR
		|| type == TK_AND || type == TK_SEMI);
}

/*
** is_redir: returns 1 if type is a redirect token
*/
static int	is_redir(int type)
{
	return (type == TK_RIN || type == TK_ROUT
		|| type == TK_APPEND || type == TK_HEREDOC);
}

/*
** chk_redir: validates that a redirect operator is followed by a target word.
** This catches syntax like `>` at end of input or `> |` and reports an error.
*/
static int	chk_redir(t_vec *toks, int i, t_shell *sh)
{
	t_token	*next;

	if (i + 1 >= toks->len)
	{
		err_syntax(sh, "newline");
		return (0);
	}
	next = (t_token *)toks->data[i + 1];
	if (next->type != TK_WORD)
	{
		err_syntax(sh, next->value);
		return (0);
	}
	return (1);
}

/*
** chk_token: validates the token at position i for syntax rules.
** It rejects misplaced operators, consecutive binary operators, and
** an orphan closing parenthesis at the start of input.
*/
static int	chk_token(t_vec *toks, int i, t_shell *sh)
{
	t_token	*tok;

	tok = (t_token *)toks->data[i];
	if (is_redir(tok->type))
		return (chk_redir(toks, i, sh));
	if (is_binop(tok->type))
	{
		if (i == 0 || is_binop(((t_token *)toks->data[i - 1])->type))
		{
			err_syntax(sh, tok->value);
			return (0);
		}
	}
	if (tok->type == TK_RPAREN && i == 0)
	{
		err_syntax(sh, ")");
		return (0);
	}
	return (1);
}

/*
** check_syntax: performs a full pass over token stream to validate command
** structure. It checks redirect targets, operator placement, and rejects a
** trailing operator or redirection without a target.
*/
int	check_syntax(t_vec *toks, t_shell *sh)
{
	int		i;
	t_token	*last;

	i = 0;
	while (i < toks->len - 1)
	{
		if (!chk_token(toks, i, sh))
			return (0);
		i++;
	}
	if (toks->len > 1)
	{
		last = (t_token *)toks->data[toks->len - 2];
		if (is_binop(last->type) || is_redir(last->type))
		{
			err_syntax(sh, "newline");
			return (0);
		}
	}
	return (1);
}
