/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bi_cd.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** save_cwd: copies the value string from slot into dst (up to 4094 chars).
** Used to snapshot PWD before the chdir call.
*/
static void	save_cwd(char *dst, t_env_slot *slot)
{
	int	i;

	i = 0;
	if (!slot || !slot->value)
	{
		dst[0] = '\0';
		return ;
	}
	while (slot->value[i] && i < 4094)
	{
		dst[i] = slot->value[i];
		i++;
	}
	dst[i] = '\0';
}

/*
** update_pwd: sets OLDPWD=old_pwd and PWD=getcwd() in the environment after
** a successful chdir.
*/
static void	update_pwd(t_shell *sh, const char *old_pwd)
{
	char	cwd[4096];

	env_set(sh->env, "OLDPWD", old_pwd, 1);
	if (getcwd(cwd, sizeof(cwd)))
		env_set(sh->env, "PWD", cwd, 1);
}

/*
** get_env_or_err: looks up key in the environment. If not found or value is
** NULL, sets E_MISUSE error with "cd" context and msg, emits it, returns NULL.
** Otherwise returns the value string.
*/
static const char	*get_env_or_err(t_shell *sh,
					const char *key, const char *msg)
{
	t_env_slot	*slot;

	slot = env_get(sh->env, key);
	if (!slot || !slot->value)
	{
		err_set(sh, E_MISUSE, "cd", msg);
		err_emit(sh);
		return (NULL);
	}
	return (slot->value);
}

/*
** resolve_path: resolves the target path for cd. No args or "~" -> HOME.
** "-" -> OLDPWD (also prints the path). Otherwise uses args[1] directly.
** Uses get_env_or_err for HOME and OLDPWD lookups.
*/
static const char	*resolve_path(char **args, t_shell *sh)
{
	const char	*val;

	if (!args[1] || ft_strcmp(args[1], "~") == 0)
		return (get_env_or_err(sh, "HOME", "HOME not set"));
	if (ft_strcmp(args[1], "-") == 0)
	{
		val = get_env_or_err(sh, "OLDPWD", "OLDPWD not set");
		if (val)
		{
			write(STDOUT_FILENO, val, ft_strlen(val));
			write(STDOUT_FILENO, "\n", 1);
		}
		return (val);
	}
	return (args[1]);
}

/*
** bi_cd: implements `cd`. Snapshots PWD, resolves target, calls chdir, updates
** PWD/OLDPWD.
*/
int	bi_cd(char **args, void *ctx)
{
	t_shell		*sh;
	t_env_slot	*pwd_slot;
	char		old_pwd[4096];
	const char	*target;

	sh = (t_shell *)ctx;
	pwd_slot = env_get(sh->env, "PWD");
	save_cwd(old_pwd, pwd_slot);
	target = resolve_path(args, sh);
	if (!target)
		return (1);
	if (chdir(target) < 0)
	{
		err_set(sh, E_NOPERM, args[1], strerror(errno));
		err_emit(sh);
		return (1);
	}
	update_pwd(sh, old_pwd);
	return (0);
}
