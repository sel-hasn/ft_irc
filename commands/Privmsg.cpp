#include "../Server.hpp"

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";

    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

void Server::connectTobot(Client &client, std::vector<std::string> input)
{
	std::string message;

	if (input.size() > 2) {
		message = input[2];
		for (size_t i = 3; i < input.size(); ++i) {
			message += " " + input[i];
		}

		if (message[0] == ':')
			message = message.substr(1);
		else if (message[1] == ':')
			message = message.substr(2);
	}

	Client *bot = getClient(input[1]);
	if (!bot) {
		sendReply(client.getClientSocketfd(), ":IRCServer : PRIVMSG " + client.getName() + " :Bot not found.\r\n");
		std::cerr << "[ERROR] Tried to message non-existent bot: " << input[1] << std::endl;
		return;
	}

	message = trim(message);
	sendReply(bot->getClientSocketfd(), client.getName() + " : " + message);

	if (message == "game" || message == "GAME") {
		std::cout << "\n\nconnect with the bot\n\n";
		client.setconnectTobot(true);
	}
}

void Server::Privmsg(Client &client, std::vector<std::string> input)
{
	if (input.size() <= 2)
	{
		sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
		return ;
	}

	if (input[1] == "bot"){
		connectTobot(client, input);
		return ;
	}
	if (input[1][0] == '#')
	{
		std::vector<Channel>::iterator it = Channels.begin();
		for (; it != Channels.end(); it++)
		{
			if (it->getName() == input[1])
				break ;
		}
		if (it == Channels.end())
		{
			sendReply(client.getClientSocketfd(), ERR_NOSUCHCHANNEL(input[1]));
		}
		else if (it != Channels.end())
		{
			std::vector<Client>::iterator itt = it->members.begin();
			for (; itt != it->members.end(); itt++)
			{
				if (itt->getName() != client.getName())
				{
					if (input[2][0] == ':')
					{
						sendReply(itt->getClientSocketfd(), RPL_PRIVMSG(client.getHostname(), input[1], input[2].substr(1, input[2].size())));
					}
					else if (input[2][0] != ':')
					{
						sendReply(itt->getClientSocketfd(), RPL_PRIVMSG(client.getHostname(), input[1], input[2]));
					}
				}
			}
		}
	}
	else
	{
		std::vector<Client>::iterator it = Clients.begin();
		for (; it != Clients.end(); it++)
		{
			if (it->getName() == input[1])
				break ;
		}
		if (it == Clients.end() || !it->getisRegistered())
		{
			sendReply(client.getClientSocketfd(), ERR_NOSUCHNICK(input[1]));
		}
		else if (it != Clients.end())
		{
			if (input[2][0] == ':')
			{
				std::string fullMessage = input[2].substr(1);
        		for (size_t i = 3; i < input.size(); ++i)
        		    fullMessage += " " + input[i];

        		if (client.getName() == "bot" && fullMessage == " You have completed the quiz!")
        		{
					std::cout<<"disconnect with the bot\n\n";
        		    client.setconnectTobot(false);
        		}
				sendReply(it->getClientSocketfd(), RPL_PRIVMSG(client.getHostname(), input[1], input[2].substr(1, input[2].size())));
			}
			else if (input[2][0] != ':')
			{
				sendReply(it->getClientSocketfd(), RPL_PRIVMSG(client.getHostname(), input[1], input[2]));
			}
		}
	}
}