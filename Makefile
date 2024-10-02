GREEN = \033[0;32m
RESET = \033[0m

NAME = webserv

CC = c++
CFLAGS = -Wall -Wextra -Werror -std=c++98 -Iincludes

SRCS = $(wildcard sources/*.cpp) #main.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS)
	@echo "Linking the executable..."
	@$(CC) $(CFLAGS) -o $(NAME) $(OBJS)
	@echo "$(GREEN)./$(NAME) is ready!$(RESET)"


sources/%.o: sources/%.cpp includes/%.hpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

%.o: %.cpp includes/%.hpp
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Cleaning object files and executable..."
	@rm -f $(OBJS)

fclean: clean
	@echo "Cleaning all temporary files..."
	@rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
