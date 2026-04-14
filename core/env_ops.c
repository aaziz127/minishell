/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   env_ops.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/01 00:00:00 by alaziz            #+#    #+#             */
/*   Updated: 2025/01/01 00:00:00 by alaziz       ###   42LUXEMBOURG.lu       */
/*                                                                            */
/* ************************************************************************** */

#include "../includes/minishell.h"

/*
** Writes "key=value\n" to stdout for one env slot.
*/
static void	print_slot(const t_env_slot *s)
{
	write(STDOUT_FILENO, s->key, ft_strlen(s->key));
	write(STDOUT_FILENO, "=", 1);
	if (s->value)
		write(STDOUT_FILENO, s->value, ft_strlen(s->value));
	write(STDOUT_FILENO, "\n", 1);
}

/*
** Prints all exported slots that have a value in `declare -x KEY=value` format
** (used by `export` with no args).
*/
void	env_print(const t_rh_env *e)
{
	int	i;

	i = 0;
	while (i < e->cap)
	{
		if (e->slots[i].psl != RH_EMPTY && e->slots[i].exported
			&& e->slots[i].value)
		{
			write(STDOUT_FILENO, "declare -x ", 11);
			print_slot(&e->slots[i]);
		}
		i++;
	}
}

/*
** Converts exported env slots to a NULL-terminated "KEY=value" char** array
** in arena m, suitable for execve.
*/
char	**env_to_envp(const t_rh_env *e, t_mem *m)
{
	t_vec		*v;
	t_env_iter	it;
	t_env_slot	*slot;

	v = vec_new(m, 32);
	it = env_iter_mk((t_rh_env *)e);
	while (env_iter_has(&it))
	{
		slot = env_iter_next(&it);
		if (slot->exported && slot->value)
			vec_push(v, str_cat3(m, slot->key, "=", slot->value));
	}
	return (vec_to_arr(v, m));
}
