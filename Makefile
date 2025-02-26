GREEN = \033[0;32m
RESET = \033[0m

DEBUG = 0
PARSER = 0

NAME = webserv

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes

ifeq ($(DEBUG),1)
	CFLAGS += -DDEBUG_MODE
endif

ifeq ($(PARSER),1)
	CFLAGS += -DPARSER_MODE
endif

ifeq ($(CGI), 1)
	CFLAGS += -DCGI
endif


SRCS = $(wildcard sources/*.cpp)
OBJS = $(patsubst sources/%.cpp, objects/%.o, $(SRCS))

all: $(NAME)

$(NAME): $(OBJS)
	@echo "Linking the executable..."
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@echo "$(GREEN)./$(NAME) is ready!$(RESET)"

objects/%.o: sources/%.cpp | objects
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

objects:
	@mkdir -p objects

clean:
	@echo "Cleaning object files and executable..."
	@rm -f $(OBJS)
	@echo "Removing upload directories..."
	@find . -type d -name "uploads" -exec rm -rf {} +

fclean: clean
	@echo "Cleaning all temporary files..."
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
