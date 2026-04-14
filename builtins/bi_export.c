/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bi_export.c                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** copy_key: copies the identifier portion of arg (up to '=' or end of string)
** into key, stopping at 255 chars.
*/
static void	copy_key(char *key, const char *arg, const char *eq)
{
	int	i;

	i = 0;
	while (arg[i] && i < 255 && (!eq || arg + i < eq))
	{
		key[i] = arg[i];
		i++;
	}
	key[i] = '\0';
}

/*
** export_one: processes one export argument "NAME" or "NAME=VALUE". Extracts
** key via copy_key, validates with ft_valid_id, calls env_set. Returns 1 on
** invalid identifier error, 0 otherwise.
*/
static int	export_one(const char *arg, t_shell *sh)
{
	char		key[256];
	const char	*eq;

	eq = ft_strchr(arg, '=');
	copy_key(key, arg, eq);
	if (!ft_valid_id(key))
	{
		err_set(sh, E_MISUSE, "export", "not a valid identifier");
		err_emit(sh);
		return (1);
	}
	if (eq)
		env_set(sh->env, key, eq + 1, 1);
	else
		env_set(sh->env, key, NULL, 1);
	return (0);
}

/*
** bi_export: implements `export`. No args -> calls env_print. With args ->
** calls export_one for each. Returns 1 if any argument was invalid.
*/
int	bi_export(char **args, void *ctx)
{
	t_shell	*sh;
	int		i;
	int		ret;

	sh = (t_shell *)ctx;
	if (!args[1])
	{
		env_print(sh->env);
		return (0);
	}
	ret = 0;
	i = 1;
	while (args[i])
	{
		if (export_one(args[i], sh) != 0)
			ret = 1;
		i++;
	}
	return (ret);
}
