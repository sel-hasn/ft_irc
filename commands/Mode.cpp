#include "../Server.hpp"

void Server::Mode(Client client, std::vector<std::string> input)
{
    if (input.size() < 2)
    {
        sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
        return ;
    }
    Channel *channel = getChannel(input[1]);
    if (!channel){
        sendReply(client.getClientSocketfd(), ERR_NOSUCHCHANNEL(input[1]));
        return ;
    }
    if (!channel->inChannel(client)){
        sendReply(client.getClientSocketfd(), ERR_NOTONCHANNEL(client.getName(), input[1]));
        return ;
    }
    if (input.size() == 2)
    {
        std::string mode = "+";
        std::string argc = "";
        if (channel->getInviteOnly())
            mode += "i";
        if (channel->getTopicProtected())
            mode += "t";
        if (channel->get_pass_flag())
        {
            argc = channel->getPass();
            mode += "t";
        }
        sendReply(client.getClientSocketfd(), RPL_CHANNELMODEIS(client.getName(), channel->getName(), mode, argc));
        return;
    }
    if (!channel->isOperator(client)){
        sendReply(client.getClientSocketfd(), ERR_CHANOPRIVSNEEDED(input[1]));
        return ;
    }
    std::string mode = input[2];
    if (mode.length() != 2 || (mode[0] != '+' && mode[0] != '-'))
    {
        sendReply(client.getClientSocketfd(), ERR_UNKNOWNMODE(client.getName(), input[1], mode));
        return;
    }

    bool add;
    
    if (mode[0] == '+')
        add = true;
    else if (mode[0] == '-')
        add = false;
    
    if (mode[1] == 'i')
        channel->setInviteOnly(add);
    else if (mode[1] == 't')
        channel->setTopicProtected(add);
    else if (mode[1] == 'k')
    {
        if (add == true){
            if (input.size() < 4){
                sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
                return;
            }
            channel->set_pass_flag(add);
            channel->setPass(input[3]);
        }
        else {
            channel->set_pass_flag(add);
            channel->setPass("");
        }
    }
    else if (mode[1] == 'o'){
        if (input.size() < 4){
            sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
            return;
        }
        Client *target = getClient(input[3]);
        if (!target || !channel->inChannel(*target)) {
            sendReply(client.getClientSocketfd(), ERR_USERNOTINCHANNEL(input[3], input[1]));
            return;
        }
        if (add){
            channel->admines.push_back(*target);
        }
        else {
            channel->RemoveOperator(*target);
        }
    }
    else if (mode[1] == 'l'){
        if (add){
            if (input.size() < 4){
                sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
                return;
            }
            int limit = 0;
            bool valid = true;
            std::string limitStr = input[3];
            if (limitStr.empty()){
                return ;
            }
            if (!limitStr.empty()){
                for (size_t i = 0; i < limitStr.size(); ++i){
                    if (!isdigit(limitStr[i])){
                        valid = false;
                        break ;
                    }
                }
            }
            if (valid)
            {
                limit = std::atoi(limitStr.c_str());
                if (limit >= 0)
                {
                    channel->setUserLimit(limit);
                    channel->set_UserLimitFlag(true);
                }
            }   
        }
        else {
            channel->setUserLimit(0);
            channel->set_UserLimitFlag(false);
        }
    }
    else 
        sendReply(client.getClientSocketfd(), ERR_UNKNOWNMODE(client.getName(), input[1], input[2]));
}