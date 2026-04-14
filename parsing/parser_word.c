/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_word.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

int		is_redir_tok(int type);
int		is_fd_prefix(t_tok_iter *it, t_token *tok);
void	parse_redir_fd(t_parser *p, t_cmd *cmd, int fd);

/*
** handle_word_tok: processes a WORD token inside a simple command.
** Detects fd-prefix syntax like `2>` and rewrites it as an explicit
** file descriptor redirection. Otherwise the WORD is appended to argv.
*/
static void	handle_word_tok(t_parser *p, t_cmd *cmd, t_token *tok)
{
	int	fd;

	fd = is_fd_prefix(&p->it, tok);
	if (fd >= 0)
	{
		tok_next(&p->it);
		parse_redir_fd(p, cmd, fd);
	}
	else
		vec_push(cmd->args, tok_next(&p->it)->value);
}

/*
** parse_sub: parses a subshell expression enclosed in parentheses.
** It consumes '(' then parses the nested command list and expects ')'.
** The returned ND_SUB node contains the contained command tree.
*/
static t_node	*parse_sub(t_parser *p)
{
	t_node	*nd;
	t_node	*body;

	tok_next(&p->it);
	body = parse_list(p);
	if (!tok_expect(&p->it, TK_RPAREN, p->sh))
		return (NULL);
	nd = mk_node(ND_SUB, p->sh);
	nd->left = body;
	return (nd);
}

/*
** parse_cmd: parses a simple command or subshell form.
** It accepts command words and redirections, and returns NULL for empty
** input or parenthesized subshell forms handled by parse_sub.
*/
t_node	*parse_cmd(t_parser *p)
{
	t_cmd	*cmd;
	t_node	*nd;
	t_token	*tok;

	tok = tok_peek(&p->it);
	if (!tok || tok->type == TK_EOF)
		return (NULL);
	if (tok->type == TK_LPAREN)
		return (parse_sub(p));
	cmd = mk_cmd(p->sh);
	nd = mk_node(ND_CMD, p->sh);
	nd->cmd = cmd;
	tok = tok_peek(&p->it);
	while (tok && (tok->type == TK_WORD || is_redir_tok(tok->type)))
	{
		if (tok->type == TK_WORD)
			handle_word_tok(p, cmd, tok);
		else
			parse_redir_fd(p, cmd, -1);
		tok = tok_peek(&p->it);
	}
	if (vec_len(cmd->args) == 0 && cmd->redirs == NULL)
		return (NULL);
	return (nd);
}
