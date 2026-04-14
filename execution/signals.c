/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   signals.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*	 Global variable to store the signal number when a signal is received. 
**	This allows the main loop to check for signals and respond appropriately.
**	It is of type sig_atomic_t to ensure safe access across signal handlers
**	and the main program. When a signal is caught, the signal handler
**	sets g_signal to the signal number (e.g., SIGINT).The main loop can then
**	check g_signal to determine if a signal was received and take action,
such as breaking out of a readline loop or cleaning up before exiting.
*/
sig_atomic_t	g_signal = 0;

/*
** sig_handler: async-signal-safe handler. Sets g_signal=sig and writes "\n"
** for SIGINT. Only write(2) is used — no readline functions (not
** async-signal-safe).
*/
void	sig_handler(int sig)
{
	g_signal = sig;
	if (sig == SIGINT)
		write(STDOUT_FILENO, "\n", 1);
}

/*
** setup_signals: interactive mode signal setup. SIGINT -> sig_handler with
** SA_RESTART. SIGQUIT -> SIG_IGN.
*/
void	setup_signals(void)
{
	struct sigaction	sa;

	sa.sa_handler = sig_handler;
	sa.sa_flags = SA_RESTART;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sa.sa_handler = SIG_IGN;
	sa.sa_flags = 0;
	sigaction(SIGQUIT, &sa, NULL);
}

/*
** setup_child_sigs: restores SIGINT and SIGQUIT to SIG_DFL in child processes
** so they respond normally to keyboard signals.
*/
void	setup_child_sigs(void)
{
	struct sigaction	sa;

	sa.sa_handler = SIG_DFL;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGQUIT, &sa, NULL);
}

/*
** setup_heredoc_sigs: heredoc mode: SIGINT -> sig_handler (so Ctrl-C breaks
** the readline loop via g_signal check). SIGQUIT -> SIG_IGN.
*/
void	setup_heredoc_sigs(void)
{
	struct sigaction	sa;

	sa.sa_handler = sig_handler;
	sa.sa_flags = 0;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sa.sa_handler = SIG_IGN;
	sigaction(SIGQUIT, &sa, NULL);
}
