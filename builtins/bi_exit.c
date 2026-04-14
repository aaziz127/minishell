/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bi_exit.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** str_is_numeric: returns 1 if s is a valid signed integer literal (optional
** +/- followed by at least one digit).
*/
static int	str_is_numeric(const char *s)
{
	int	i;

	i = 0;
	if (s[i] == '-' || s[i] == '+')
		i++;
	if (!s[i])
		return (0);
	while (s[i])
	{
		if (!ft_isdigit(s[i]))
			return (0);
		i++;
	}
	return (1);
}

/*
** str_to_code: converts s to an exit code in range 0-255 using `value & 0xFF`
** (matches bash modulo-256 behaviour).
*/
static int	str_to_code(const char *s)
{
	long	n;
	int		sign;
	int		i;

	n = 0;
	sign = 1;
	i = 0;
	if (s[i] == '-')
	{
		sign = -1;
		i++;
	}
	else if (s[i] == '+')
		i++;
	while (s[i] && ft_isdigit(s[i]))
	{
		n = n * 10 + (s[i] - '0');
		i++;
	}
	return ((int)((n * sign) & 0xFF));
}

/*
** bi_exit_err: helper that sets an E_MISUSE error, emits it, sets
** sh->running=running, and returns code. Used to deduplicate the two error
** paths in bi_exit.
*/
static int	bi_exit_err(t_shell *sh, const char *msg,
				int running, int code)
{
	err_set(sh, E_MISUSE, "exit", msg);
	err_emit(sh);
	sh->running = running;
	return (code);
}

/*
** bi_exit: implements `exit`. Prints "exit". No args: sets running=0, returns
** exit_status. Too many args: error, keep running. Non-numeric arg: error,
** exit 255. Numeric: parse code, set running=0, return code.
*/
int	bi_exit(char **args, void *ctx)
{
	t_shell	*sh;
	int		code;

	sh = (t_shell *)ctx;
	write(STDOUT_FILENO, "exit\n", 5);
	if (!args[1])
	{
		sh->running = 0;
		return (sh->exit_status);
	}
	if (args[2])
		return (bi_exit_err(sh, "too many arguments", 1, 1));
	if (!str_is_numeric(args[1]))
		return (bi_exit_err(sh, "numeric argument required", 1, 255));
	code = str_to_code(args[1]);
	sh->exit_status = code;
	sh->running = 0;
	return (code);
}
