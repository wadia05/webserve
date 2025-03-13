NAME = webserv
SRCS = main.cpp Methodes.cpp responce.cpp
OBJS = $(SRCS:.cpp=.o)
GXX = c++
FLAGS = -Wall -Wextra -Werror -std=c++98 -g -fsanitize=address

all: $(NAME)

$(NAME): $(OBJS)
	$(GXX) $(FLAGS) -o $@ $^

%.o: %.cpp
	$(GXX) $(FLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

# .PHONY: all clean fclean re