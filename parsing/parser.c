/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parser.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** mk_node: allocates an AST node in the arena and zeroes its children.
** Nodes represent commands, pipes, logical operators, and subshell groups.
*/
t_node	*mk_node(int type, t_shell *sh)
{
	t_node	*nd;

	nd = (t_node *)mem_temp(&sh->mem, sizeof(t_node));
	nd->type = type;
	nd->cmd = NULL;
	nd->left = NULL;
	nd->right = NULL;
	return (nd);
}

/*
** parse_pipe: parses a pipeline production.
** A pipe node is right-associative here, so `a | b | c` becomes
** ND_PIPE(a, ND_PIPE(b, c)). This matches shell semantics for nested pipes.
*/
static t_node	*parse_pipe(t_parser *p)
{
	t_node	*left;
	t_node	*nd;
	t_token	*tok;

	left = parse_cmd(p);
	if (!left)
		return (NULL);
	tok = tok_peek(&p->it);
	if (!tok || tok->type != TK_PIPE)
		return (left);
	tok_next(&p->it);
	nd = mk_node(ND_PIPE, p->sh);
	nd->left = left;
	nd->right = parse_pipe(p);
	return (nd);
}

/*
** is_logical: returns 1 if token type is a shell logical operator.
** Logical operators include && and ||; semicolon is handled separately.
*/
static int	is_logical(int type)
{
	return (type == TK_AND || type == TK_OR);
}

/*
** parse_list: parses command lists with &&, || and ; separators.
** The AST encodes short-circuit operators and sequential execution.
** Semicolons are represented as ND_AND nodes for consistent traversal.
*/
t_node	*parse_list(t_parser *p)
{
	t_node	*left;
	t_node	*nd;
	t_token	*tok;
	int		nd_type;

	left = parse_pipe(p);
	if (!left)
		return (NULL);
	tok = tok_peek(&p->it);
	if (!tok || (!is_logical(tok->type) && tok->type != TK_SEMI))
		return (left);
	if (tok->type == TK_SEMI || tok->type == TK_AND)
		nd_type = ND_AND;
	else
		nd_type = ND_OR;
	tok_next(&p->it);
	nd = mk_node(nd_type, p->sh);
	nd->left = left;
	nd->right = parse_list(p);
	return (nd);
}

/*
** parse: top-level parser entry point. Converts a token vector into an AST.
** The parser uses tok_iter to navigate tokens and returns the root node.
*/
t_node	*parse(t_vec *toks, t_shell *sh)
{
	t_parser	p;
	t_node		*ast;

	tok_iter_mk(&p.it, toks);
	p.sh = sh;
	ast = parse_list(&p);
	return (ast);
}
