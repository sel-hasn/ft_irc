CPPC= c++

NAME= ircserv
BONUS_NAME= bot

CPPFLAGS= -Wall -Wextra -Werror -g -std=c++98 -fsanitize=address

SRC= main.cpp Server.cpp CustomException.cpp Parse_PortPW.cpp commands/Invite.cpp  commands/Kick.cpp \
 commands/Mode.cpp   commands/Topic.cpp commands/Join.cpp  commands/Privmsg.cpp utils.cpp Authntication.cpp

BONUS_SRC= bonus/main.cpp bonus/bot.cpp

OBJS= $(SRC:%.cpp=%.o)
BONUS_OBJS= $(BONUS_SRC:%.cpp=%.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CPPC) $(CPPFLAGS) $(OBJS) -o $(NAME)

bonus: $(BONUS_NAME)

$(BONUS_NAME): $(BONUS_OBJS)
	$(CPPC) $(CPPFLAGS) $(BONUS_OBJS) -o $(BONUS_NAME)

%bonus/.o: %bonus/.cpp bonus/bot.hpp
	$(CPPC) $(CPPFLAGS) -c $< -o $@

%.o: %.cpp Server.hpp CustomException.hpp Client.hpp Channel.hpp Replies.hpp
	$(CPPC) $(CPPFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)
	rm -f $(BONUS_OBJS)

fclean: clean
	rm -f $(NAME)
	rm -f $(BONUS_NAME)

re: fclean all

.PHONY: clean
