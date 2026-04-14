/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bi_pwd.c                                      :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** bi_pwd: implements `pwd`. Calls getcwd and writes the result followed by
** newline.
*/
int	bi_pwd(char **args, void *ctx)
{
	t_shell	*sh;
	char	cwd[4096];

	sh = (t_shell *)ctx;
	(void)args;
	if (!getcwd(cwd, sizeof(cwd)))
	{
		err_set(sh, E_NOPERM, "pwd", strerror(errno));
		err_emit(sh);
		return (1);
	}
	write(STDOUT_FILENO, cwd, ft_strlen(cwd));
	write(STDOUT_FILENO, "\n", 1);
	return (0);
}
