CC = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98
RM = rm -f

# Headers
Header = webserv.hpp configFile/Tokenizer.hpp configFile/Config.hpp req/HTTPRequest.hpp

# Source files
SRCS = main.cpp Methodes.cpp responce.cpp \
		configFile/Tokenizer.cpp configFile/Config.cpp configFile/get.cpp configFile/set.cpp configFile/error.cpp \
		req/HTTPRequest.cpp req/parseBody.cpp req/parseHeaders.cpp req/parseRequestLine.cpp req/get.cpp

# Output executable
NAME = webserv
OBJS = $(SRCS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJS) $(Header)
	$(CC) $(CPPFLAGS) $(OBJS) -o $(NAME)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(NAME)

re: fclean all

.PHONY: all clean fclean re