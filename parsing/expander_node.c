/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   expander_node.c                              :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** glob_redir_file: handles glob expansion for redirection filenames
** Unlike command args, redirections need exactly one target file.
** Expands only if pattern matches exactly one file, otherwise keeps pattern.
** Prevents ambiguity in redirections like "> *.txt" when multiple files exist.
*/
static char	*glob_redir_file(const char *file, t_shell *sh)
{
	t_vec	*single_args;
	t_vec	*expanded;

	if (!ft_strchr(file, '*'))
		return (str_dup_arena(&sh->mem, file));
	single_args = vec_new(&sh->mem, 1);
	vec_push(single_args, (void *)file);
	expanded = glob_expand_vec(single_args, sh);
	if (vec_len(expanded) == 1)
		return ((char *)expanded->data[0]);
	return (str_dup_arena(&sh->mem, file));
}

/*
** expand_node_args: processes command arguments for expansion
** Performs variable expansion, quote removal, and word splitting.
** Removes empty unquoted args (bash word-splitting behavior).
** Preserves quoted empty strings as intentional empty arguments.
*/
static void	expand_node_args(t_node *nd, t_shell *sh)
{
	t_vec	*new_args;
	int		i;
	char	first;

	new_args = vec_new(&sh->mem, vec_len(nd->cmd->args) + 1);
	i = 0;
	while (i < vec_len(nd->cmd->args))
	{
		first = ((char *)nd->cmd->args->data[i])[0];
		nd->cmd->args->data[i] = expand_str(nd->cmd->args->data[i], sh);
		if (*(char *)nd->cmd->args->data[i] || first == '\'' || first == '"')
			vec_push(new_args, nd->cmd->args->data[i]);
		i++;
	}
	nd->cmd->args = new_args;
}

/*
** expand_node_redirs: processes redirection filenames for expansion
** Expands variables in filenames and applies conservative glob expansion.
** Skips heredoc redirections (handled separately during execution).
** Processes redirections in declaration order.
*/
static void	expand_node_redirs(t_node *nd, t_shell *sh)
{
	t_redir	*r;

	r = nd->cmd->redirs;
	while (r)
	{
		if (r->type != RD_HEREDOC)
		{
			r->file = expand_str(r->file, sh);
			r->file = glob_redir_file(r->file, sh);
		}
		r = r->next;
	}
}

/*
** expand_node: orchestrates expansion of command arguments and redirections
** Guards against NULL nodes/commands, then delegates to specialized functions.
** Performs two-phase expansion: arguments first, then redirections.
** All expansions use arena memory that resets between commands.
*/
void	expand_node(t_node *nd, t_shell *sh)
{
	if (!nd || !nd->cmd)
		return ;
	expand_node_args(nd, sh);
	expand_node_redirs(nd, sh);
}
