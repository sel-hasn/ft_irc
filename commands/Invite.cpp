#include "../Server.hpp"

void Server::Invite(Client client, std::vector<std::string> input)
{
    if (input.size() < 3)
    {
        sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
        return ;
    }
    Channel *channel = getChannel(input[2]);
    if (!channel){
        sendReply(client.getClientSocketfd(), ERR_NOSUCHCHANNEL(input[2]));
        return ;
    }
    if (!channel->inChannel(client)){
        sendReply(client.getClientSocketfd(), ERR_NOTONCHANNEL(client.getName(), input[2]));
        return ;
    }
    Client *toInviteClient = getClient(input[1]);
    if (!toInviteClient){
        sendReply(client.getClientSocketfd(), ERR_NOSUCHNICK(input[1]));
        return ;
    }
    if (channel->inChannel(*toInviteClient))
    {
        sendReply(client.getClientSocketfd(), ERR_USERONCHANNEL(input[1], input[2]));
        return ;
    }
    if (channel->getInviteOnly() && channel->isOperator(client))
    {
        sendReply(client.getClientSocketfd(), ERR_CHANOPRIVSNEEDED(input[2]));
        return ;
    }
    sendReply(client.getClientSocketfd(), RPL_INVITING(client.getName(), toInviteClient->getName(), channel->getName()));
    std::string reply = ":" + client.getHostname() + " INVITE " + toInviteClient->getName() + " " + channel->getName() + "\r\n";
    channel->members.push_back(*toInviteClient);
    for (size_t i = 0; i < channel->members.size(); ++i)
    {
        sendReply(channel->members[i].getClientSocketfd(), reply);
    }
}