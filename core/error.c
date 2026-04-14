/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   error.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** fill_buf - Copies src chars into buf starting at offset i, stopping at max-1.
** @buf: The destination buffer.
** @src: The source string to copy from.
** @i: Starting offset in buf.
** @max: Maximum position in buf (exclusive).
**
** Used to safely build error messages. Returns new offset i.
*/
static int	fill_buf(char *buf, const char *src, int i, int max)
{
	int	j;

	j = 0;
	while (i < max - 1 && src && src[j])
		buf[i++] = src[j++];
	return (i);
}

/*
** err_set - Stores an error code, context string, and message without printing.
** @sh: Pointer to the t_shell structure.
** @code: The error code (t_err_code).
** @ctx: Context string (command/context name, may be NULL).
** @msg: Human-readable error message.
**
** Returns void.
*/
void	err_set(t_shell *sh, t_err_code code,
		const char *ctx, const char *msg)
{
	int	i;

	sh->err.code = code;
	i = fill_buf(sh->err.ctx, ctx, 0, (int) sizeof(sh->err.ctx));
	sh->err.ctx[i] = '\0';
	i = fill_buf(sh->err.msg, msg, 0, (int) sizeof(sh->err.msg));
	sh->err.msg[i] = '\0';
}

/*
** err_emit - Writes the stored error to stderr as "minishell: [ctx: ]msg\n".
** @sh: Pointer to the t_shell structure.
**
** No-op if code is E_OK. Clears the error after emitting. Returns void.
*/
void	err_emit(t_shell *sh)
{
	if (sh->err.code == E_OK)
		return ;
	write(STDERR_FILENO, "minishell: ", 11);
	if (sh->err.ctx[0])
	{
		write(STDERR_FILENO, sh->err.ctx, ft_strlen(sh->err.ctx));
		write(STDERR_FILENO, ": ", 2);
	}
	write(STDERR_FILENO, sh->err.msg, ft_strlen(sh->err.msg));
	write(STDERR_FILENO, "\n", 1);
	err_clear(sh);
}

/*
** err_clear - Resets sh->err to E_OK with empty strings.
** @sh: Pointer to the t_shell structure.
**
** Returns void.
*/
void	err_clear(t_shell *sh)
{
	sh->err.code = E_OK;
	sh->err.ctx[0] = '\0';
	sh->err.msg[0] = '\0';
}

/*
** err_syntax - Formats and emits a bash-style syntax error for bad token.
** @sh: Pointer to the t_shell structure.
** @token: The unexpected token string.
**
** Sets exit_status=2 and formats "syntax error near unexpected token `TOKEN'".
** Returns void.
*/
void	err_syntax(t_shell *sh, const char *token)
{
	char	buf[256];
	int		i;

	i = fill_buf(buf, "syntax error near unexpected token `", 0, 256);
	i = fill_buf(buf, token, i, 256);
	i = fill_buf(buf, "'", i, 256);
	buf[i] = '\0';
	err_set(sh, E_SYNTAX, NULL, buf);
	err_emit(sh);
	sh->exit_status = 2;
}
