#include "../Server.hpp"

std::vector<std::string> handlinput(const std::string& s, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delimiter)) {
        tokens.push_back(item);
    }
    return tokens;
}

void Server::Kick(Client client, std::vector<std::string> input)
{
    if (input.size() < 3)
    {
        sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
        return ;
    }

    std::vector<std::string> channels = handlinput(input[1], ',');
    std::vector<std::string> users = handlinput(input[2], ',');

    std::string comment = (input.size() >= 4) ? input[3] : "";

    if (channels.size() != users.size()) {
        sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
        return;
    }

    for (size_t i = 0; i < channels.size(); ++i) {
        std::string chanName = channels[i];
        std::string targetNick = users[i];

        Channel *channel = getChannel(chanName);
        if (!channel) {
            sendReply(client.getClientSocketfd(), ERR_NOSUCHCHANNEL(chanName));
            continue;
        }

        if (!channel->isOperator(client)) {
            sendReply(client.getClientSocketfd(), ERR_CHANOPRIVSNEEDED(chanName));
            continue;
        }

        Client *targetClient = getClient(targetNick);
        if (!targetClient || !channel->inChannel(*targetClient)) {
            sendReply(client.getClientSocketfd(), ERR_USERNOTINCHANNEL(targetNick, chanName));
            continue;
        }
        std::string reply;
        if (comment.empty())
            reply = ":" + client.getHostname() + " KICK " + chanName + " " + targetNick + "\r\n";
        else 
            reply = ":" + client.getHostname() + " KICK " + chanName + " " + targetNick + " " + comment + "\r\n";
        for (size_t i = 0; i < channel->members.size(); ++i)
        {
            if (channel->members[i].getName() != targetNick)
                sendReply(channel->members[i].getClientSocketfd(), reply);
        }
        sendReply(targetClient->getClientSocketfd(), reply);
        if (channel->isOperator(*targetClient)){
            channel->RemoveMember(*targetClient);
            channel->RemoveOperator(*targetClient);
            if (channel->admines.size() == 0 && channel->members.size() > 0){
                for (size_t i = 0; i < channel->members.size(); i++)
                {
                    if (targetNick != channel->members[i].getName()){
                        channel->admines.push_back(channel->members[i]);
						Mode(*targetClient, split("MODE " + chanName + " +o " + channel->members[i].getName() + "\r\n"));
                        break ;
                    }
                }
            }
            if (channel->members.size() == 0)
            {
                for (size_t i = 0; i < Channels.size(); i++)
                {
                    if (chanName == Channels[i].getName())
                    {
                        Channels.erase(Channels.begin() + i);
                        break ;
                    }
                }
            }
        }
        else 
            channel->RemoveMember(*targetClient);
    }
}
