
CPPC= c++

NAME= ircserv

CPPFLAGS= -Wall -Wextra -Werror -g -std=c++98 -fsanitize=address

SRC= main.cpp Server.cpp CustomException.cpp Parse_PortPW.cpp

OBJS= $(SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJS)
		$(CPPC) $(CPPFLAGS) $(OBJS) -o $(NAME)

%.o: %.cpp Server.hpp CustomException.hpp Client.hpp
	$(CPPC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: clean
