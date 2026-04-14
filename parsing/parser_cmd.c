/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser_cmd.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** mk_cmd: allocates a command structure with an empty argv-style args vector.
** The t_cmd collects command words and a linked list of redirections.
*/
t_cmd	*mk_cmd(t_shell *sh)
{
	t_cmd	*cmd;

	cmd = (t_cmd *)mem_temp(&sh->mem, sizeof(t_cmd));
	cmd->args = vec_new(&sh->mem, 8);
	cmd->redirs = NULL;
	return (cmd);
}

/*
** push_redir: appends a redirect node to the command's redirection list.
** Order matters because shell redirections are applied in the sequence they
** appear in the command.
*/
void	push_redir(t_cmd *cmd, t_redir *r)
{
	t_redir	*tail;

	if (!cmd->redirs)
		cmd->redirs = r;
	else
	{
		tail = cmd->redirs;
		while (tail->next)
			tail = tail->next;
		tail->next = r;
	}
}

/*
** is_redir_tok: checks whether a token is any redirection operator.
** Includes input, output, append, and heredoc redirections.
*/
int	is_redir_tok(int type)
{
	return (type == TK_RIN || type == TK_ROUT
		|| type == TK_APPEND || type == TK_HEREDOC);
}

/*
** is_fd_prefix: detects file descriptor prefix syntax like `2>` or `3>>`.
** If the current word token is a single digit and the next token is a
** redirection operator, returns that digit; otherwise returns -1.
*/
int	is_fd_prefix(t_tok_iter *it, t_token *tok)
{
	t_token	*next;

	if (tok->type != TK_WORD || !tok->value)
		return (-1);
	if (tok->value[0] < '0' || tok->value[0] > '9' || tok->value[1])
		return (-1);
	if (it->pos + 1 >= it->toks->len)
		return (-1);
	next = (t_token *)it->toks->data[it->pos + 1];
	if (!next || !is_redir_tok(next->type))
		return (-1);
	return (tok->value[0] - '0');
}

/*
** parse_redir_fd: parses a redirection from tokens and attaches it to cmd.
** The operator token determines redirect type, the next WORD token is the
** target filename or heredoc delimiter, and fd is the optional explicit fd.
*/
void	parse_redir_fd(t_parser *p, t_cmd *cmd, int fd)
{
	t_token	*op;
	t_token	*file;
	t_redir	*r;

	op = tok_next(&p->it);
	file = tok_next(&p->it);
	if (!file || file->type != TK_WORD)
	{
		err_syntax(p->sh, "newline");
		return ;
	}
	r = (t_redir *)mem_temp(&p->sh->mem, sizeof(t_redir));
	r->type = op->type - TK_RIN;
	r->fd = fd;
	r->file = file->value;
	r->next = NULL;
	push_redir(cmd, r);
}
