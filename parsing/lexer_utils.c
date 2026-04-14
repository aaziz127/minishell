/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   lexer_utils.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** is_op_char: determines whether a character begins an operator token.
** Operators include pipeline, logical AND/OR, redirections, parentheses,
** and statement terminator (';'). This is used by the lexer to split
** words from operator tokens.
*/
int	is_op_char(char c)
{
	return (c == '|' || c == '&' || c == '<' || c == '>'
		|| c == '(' || c == ')' || c == ';');
}

/*
** skip_spaces: advances the scanner to the next non-whitespace char.
** This lexer helper skips spaces, tabs, and other whitespace between tokens.
*/
void	skip_spaces(char **s)
{
	while (**s && ft_isspace(**s))
		(*s)++;
}

/*
** classify_op: classifies an operator string into a TK_* token type.
** Supports two-character operators first (||, &&, <<, >>) and falls back
** to single-character operators. Returns TK_EOF if no operator matches.
*/
int	classify_op(const char *s)
{
	if (s[0] == '|' && s[1] == '|')
		return (TK_OR);
	if (s[0] == '&' && s[1] == '&')
		return (TK_AND);
	if (s[0] == '<' && s[1] == '<')
		return (TK_HEREDOC);
	if (s[0] == '>' && s[1] == '>')
		return (TK_APPEND);
	if (s[0] == '|')
		return (TK_PIPE);
	if (s[0] == '<')
		return (TK_RIN);
	if (s[0] == '>')
		return (TK_ROUT);
	if (s[0] == '(')
		return (TK_LPAREN);
	if (s[0] == ')')
		return (TK_RPAREN);
	if (s[0] == ';')
		return (TK_SEMI);
	return (TK_EOF);
}

/*
** op_len: returns 2 for two-char operators (|| && << >>), 1 for single-char
*/
int	op_len(int type)
{
	if (type == TK_OR || type == TK_AND
		|| type == TK_HEREDOC || type == TK_APPEND)
		return (2);
	return (1);
}
