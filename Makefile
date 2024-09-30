NAME = webserv

CXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98
INCLUDES = -I incl/

SRCS = main.cpp \
		Server.cpp

OBJS = $(SRCS:.cpp=.o)

SRCDIR = src
OBJDIR = obj

all: $(NAME)

$(NAME): $(addprefix $(OBJDIR)/, $(OBJS))
	$(CXX) $(CXXFLAGS) $(addprefix $(OBJDIR)/, $(OBJS)) -o $(NAME)

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf $(OBJDIR)

fclean: clean
	rm -f $(NAME)

re: fclean all