/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error_fatal.c                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** err_fatal - Writes "minishell: fatal: [ctx: ]msg\n" to stderr and exits.
** @ctx: Context string (may be NULL). Used for unrecoverable startup errors.
** @msg: Human-readable error message.
**
** Uses write(2) only (async-signal-safe). Does not return (calls exit(1)).
*/
void	err_fatal(const char *ctx, const char *msg)
{
	write(STDERR_FILENO, "minishell: fatal: ", 18);
	if (ctx)
	{
		write(STDERR_FILENO, ctx, ft_strlen(ctx));
		write(STDERR_FILENO, ": ", 2);
	}
	write(STDERR_FILENO, msg, ft_strlen(msg));
	write(STDERR_FILENO, "\n", 1);
	exit(1);
}
