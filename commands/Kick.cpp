#include "../Server.hpp"

void Server::Kick(Client client, std::vector<std::string> input, std::string buffer)
{
    if (input.size() < 3)
    {
        sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
        return ;
    }
    Channel *channel = getChannel(input[1]);
    if (!channel){
        sendReply(client.getClientSocketfd(), ERR_NOSUCHCHANNEL(input[1]));
        return ;
    }
    Client *toKickclient = getClient(input[2]);
    if (!toKickclient){
        sendReply(client.getClientSocketfd(), ERR_USERNOTINCHANNEL(input[2], input[1]));
        return ;
    }
    if (!channel->inChannel(client)){
        sendReply(client.getClientSocketfd(), ERR_NOTONCHANNEL(client.getName(), input[1]));
        return ;
    }
    std::string couse;
    if (input.size() > 3)
        couse = buffer.substr(buffer.find(":") + 1);
    else 
        couse = "";
    if (channel->isOperator(client)) {
        std::string reply;
        if (couse.empty())
            reply = ":" + client.getHostname() + " KICK " + channel->getName() + " " + toKickclient->getName() + "\r\n";
        else 
            reply = ":" + client.getHostname() + " KICK " + channel->getName() + " " + toKickclient->getName() + " :" + couse + "\r\n";
        for (size_t i = 0; i < channel->members.size(); ++i)
        {
            if (channel->members[i].getName() != toKickclient->getName())
                sendReply(channel->members[i].getClientSocketfd(), reply);
        }
        sendReply(toKickclient->getClientSocketfd(), reply);
        if (channel->isOperator(*toKickclient)){
            std::cout << "here \n";
            channel->RemoveMember(*toKickclient);
            channel->RemoveOperator(*toKickclient);
        }
        else 
            channel->RemoveMember(*toKickclient);
    }
    else 
        sendReply(client.getClientSocketfd(), ERR_CHANOPRIVSNEEDED(input[1]));
}