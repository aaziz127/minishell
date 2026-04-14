/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   redirections.c                                :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** mk_hd_path: builds a unique temp-file path for a heredoc:
** "/tmp/.msh_hd_<pid>_<n>" where n is sh->hd_count (incremented).
*/
static void	mk_hd_path(char *buf, int size, t_shell *sh)
{
	int		i;
	char	*pid_s;
	char	*cnt_s;

	pid_s = str_itoa(&sh->mem, (int)getpid());
	cnt_s = str_itoa(&sh->mem, sh->hd_count++);
	i = 0;
	while (i < size - 1 && "/tmp/.msh_hd_"[i])
	{
		buf[i] = "/tmp/.msh_hd_"[i];
		i++;
	}
	while (i < size - 1 && *pid_s)
		buf[i++] = *pid_s++;
	buf[i++] = '_';
	while (i < size - 1 && *cnt_s)
		buf[i++] = *cnt_s++;
	buf[i] = '\0';
}

/*
** handle_heredoc: implements <<. Creates a temp file, reads lines via
** readline("> ") until the delimiter is seen or SIGINT. Writes each line to
** the temp file. Opens it for reading, unlinks it (file stays alive via fd
** until closed). Returns the read-only fd, or -1 on open failure.
*/
int	handle_heredoc(const char *delim, t_shell *sh)
{
	char	path[128];
	int		fd_w;
	int		fd_r;
	char	*line;

	mk_hd_path(path, 128, sh);
	fd_w = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	if (fd_w < 0)
		return (-1);
	setup_heredoc_sigs();
	line = readline("> ");
	while (line && ft_strcmp(line, delim) != 0 && !g_signal)
	{
		write(fd_w, line, ft_strlen(line));
		write(fd_w, "\n", 1);
		free(line);
		line = readline("> ");
	}
	if (line)
		free(line);
	close(fd_w);
	setup_signals();
	fd_r = open(path, O_RDONLY);
	unlink(path);
	return (fd_r);
}

/*
** open_file: opens the file specified by redirect r for the appropriate mode:
** RDONLY for RD_IN, WRONLY|CREAT|TRUNC for RD_OUT, WRONLY|CREAT|APPEND for
** RD_APPEND. Calls err_set+err_emit on failure.
*/
static int	open_file(t_redir *r, t_shell *sh)
{
	int	fd;
	int	flags;

	if (r->type == RD_IN)
		flags = O_RDONLY;
	else if (r->type == RD_OUT)
		flags = O_WRONLY | O_CREAT | O_TRUNC;
	else
		flags = O_WRONLY | O_CREAT | O_APPEND;
	fd = open(r->file, flags, 0644);
	if (fd < 0)
	{
		err_set(sh, E_NOPERM, r->file, strerror(errno));
		err_emit(sh);
	}
	return (fd);
}

/*
** redir_target: returns the target fd for dup2. If r->fd >= 0, returns it
** (explicit fd like 2>). Otherwise returns STDIN for input-type redirects,
** STDOUT for output-type.
*/
static int	redir_target(t_redir *r)
{
	if (r->fd >= 0)
		return (r->fd);
	if (r->type == RD_IN || r->type == RD_HEREDOC)
		return (STDIN_FILENO);
	return (STDOUT_FILENO);
}

/*
** apply_redirs: walks the redirect list in order, opens each file (or heredoc),
** dup2s it to the target fd, closes the original fd. Returns 0 on any failure.
** Because the list is in source order, the last redirect for each fd wins —
** matching bash behaviour.
*/
int	apply_redirs(t_redir *r, t_shell *sh)
{
	int	fd;
	int	target;

	while (r)
	{
		if (r->type == RD_HEREDOC)
			fd = handle_heredoc(r->file, sh);
		else
			fd = open_file(r, sh);
		if (fd < 0)
			return (0);
		target = redir_target(r);
		if (dup2(fd, target) < 0)
		{
			close(fd);
			return (0);
		}
		close(fd);
		r = r->next;
	}
	return (1);
}
