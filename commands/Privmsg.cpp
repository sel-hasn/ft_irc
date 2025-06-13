#include "../Server.hpp"

void Server::Privmsg(Client client, std::vector<std::string> input)
{
	(void)client;
	if (input.size() <= 2)
	{
		sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
		return ;
	}
	if (input[2][0] != ':')
	{
		sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
		return ;
	}
	if (input[1][0] == '#')
	{
		std::vector<Channel>::iterator it = Channels.begin();
		std::string holder(input[1].substr(1, input[1].size()));
		for (; it != Channels.end(); it++)
		{
			if (it->getName() == holder)
				break ;
		}
		if (it == Channels.end())
		{
			sendReply(client.getClientSocketfd(), ERR_NOSUCHCHANNEL(holder));
		}
		else if (it != Channels.end())
		{
			std::vector<Client>::iterator itt = it->members.begin();
			for (; itt != it->members.end(); itt++)
			{
				if (itt->getName() != client.getName())
				{
					sendReply(itt->getClientSocketfd(), RPL_PRIVMSG(client.getHostname(), input[1], input[2].substr(1, input[2].size())));
				}
			}
		}
	}
	else if (input[1][0] == '$')
	{
		std::string holder = input[1].substr(1, input[1].size());
		
	}
	else
	{
		std::vector<Client>::iterator it = Clients.begin();
		for (; it != Clients.end(); it++)
		{
			if (it->getName() == input[1])
				break ;
		}
		if (it == Clients.end())
		{
			sendReply(client.getClientSocketfd(), ERR_NOSUCHNICK(input[1]));
		}
		else if (it != Clients.end())
		{
			sendReply(it->getClientSocketfd(), RPL_PRIVMSG(client.getHostname(), input[1], input[2].substr(1, input[2].size())));
		}
	}
}