/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   tables_init.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/04/09 02:17:14 by alaziz            #+#    #+#             */
/*   Updated: 2026/04/09 02:17:32 by alaziz           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** Populates sh->dispatch[ND_*] with the execution visitor functions
** (xv_cmd, xv_pipe, xv_and, xv_or, xv_sub). Called once at startup.
*/
void	dispatch_init(t_shell *sh)
{
	sh->dispatch[ND_CMD] = xv_cmd;
	sh->dispatch[ND_PIPE] = xv_pipe;
	sh->dispatch[ND_AND] = xv_and;
	sh->dispatch[ND_OR] = xv_or;
	sh->dispatch[ND_SUB] = xv_sub;
}

/*
** Creates the builtin hash table and registers all 7 builtins
** (echo, cd, pwd, export, unset, env, exit).
*/
void	builtin_ht_init(t_shell *sh)
{
	sh->builtins = builtin_ht_new(16);
	builtin_ht_set(sh->builtins, "echo", bi_echo);
	builtin_ht_set(sh->builtins, "cd", bi_cd);
	builtin_ht_set(sh->builtins, "pwd", bi_pwd);
	builtin_ht_set(sh->builtins, "export", bi_export);
	builtin_ht_set(sh->builtins, "unset", bi_unset);
	builtin_ht_set(sh->builtins, "env", bi_env);
	builtin_ht_set(sh->builtins, "exit", bi_exit);
}
