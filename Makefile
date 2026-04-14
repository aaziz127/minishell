# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: alaziz <alaziz@student.42luxembourg.lu>    +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/04/09 02:22:30 by alaziz            #+#    #+#              #
#    Updated: 2026/04/09 02:42:24 by alaziz           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

NAME    = minishell
NAME_BONUS = minishell_bonus
CC      = cc
CFLAGS  = -Wall -Wextra -Werror -g3
IFLAGS  = -I includes

# ── Readline : Linux système ou macOS Homebrew ───────────────
UNAME := $(shell uname -s)
ifeq ($(UNAME), Darwin)
  RL_PREFIX := $(shell brew --prefix readline 2>/dev/null \
                || echo /usr/local/opt/readline)
  IFLAGS   += -I$(RL_PREFIX)/include
  LDFLAGS   = -L$(RL_PREFIX)/lib -lreadline
else
  LDFLAGS   = -lreadline
endif

# ── Sources ─────────────────────────────────────────────────

CORE_SRC =	core/mem.c				\
			core/mem1.c				\
			core/error.c			\
			core/error_fatal.c		\
			core/strbuf.c			\
			core/str.c				\
			core/env.c				\
			core/env_getdel.c		\
			core/env_ops.c			\
			core/env_init.c			\
			core/env_iter.c			\
			core/vector.c			\
			core/tok_iter.c			\
			core/visitor.c			\
			core/visitor_exec_mk.c	\
			core/builtin_ht.c		\
			core/builtin_ht_free.c	\
			core/shell_init.c		\
			core/tables_init.c

PARSE_SRC =	parsing/lexer.c			\
			parsing/lexer_utils.c	\
			parsing/syntax.c		\
			parsing/parser.c		\
			parsing/parser_cmd.c	\
			parsing/parser_word.c	\
			parsing/expander.c		\
			parsing/expander_node.c

EXEC_SRC =	execution/signals.c		\
			execution/redirections.c\
			execution/exec_utils.c	\
			execution/exec_ops.c	\
			execution/pipes.c		\
			execution/visitor_exec.c\
			execution/executor.c

BUILTIN_SRC =	builtins/bi_echo.c	\
				builtins/bi_cd.c	\
				builtins/bi_pwd.c	\
				builtins/bi_export.c\
				builtins/bi_unset.c	\
				builtins/bi_env.c	\
				builtins/bi_exit.c

UTILS_SRC =	utils/ft_str.c		\
			utils/ft_utils.c

BONUS_SRC =	bonus/wildcards.c

MAIN_SRC = main.c

SRCS       = $(CORE_SRC) $(PARSE_SRC) $(EXEC_SRC) \
             $(BUILTIN_SRC) $(UTILS_SRC) $(BONUS_SRC) $(MAIN_SRC)
SRCS_BONUS = $(SRCS)

OBJS       = $(SRCS:.c=.o)
OBJS_BONUS = $(SRCS_BONUS:.c=.o)

# ── Rules ────────────────────────────────────────────────────

.PHONY: all clean fclean re bonus test

all: $(NAME)

test: all
	@chmod +x tests/*.sh
	@./tests/test_all.sh ./$(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) $(LDFLAGS) -o $(NAME)
	@echo "✓  $(NAME) built"

bonus: $(OBJS_BONUS)
	$(CC) $(CFLAGS) $(OBJS_BONUS) $(LDFLAGS) -o $(NAME)
	@ln -sf $(NAME) $(NAME_BONUS)
	@echo "✓  $(NAME) [bonus] built"
	@echo "✓  $(NAME_BONUS) symlink created"

%.o: %.c includes/minishell.h includes/types.h
	$(CC) $(CFLAGS) $(IFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(OBJS_BONUS)
	@echo "✓  objects removed"

fclean: clean
	rm -f $(NAME) $(NAME_BONUS)
	@echo "✓  $(NAME) and symlink removed"

re: fclean all
