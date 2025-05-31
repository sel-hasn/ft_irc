#include "../Server.hpp"

std::string toString(time_t val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
}

void Server::Topic(Client client, std::vector<std::string> input, std::string buffer)
{
    if (input.size() < 2){
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
    if (input.size() > 2){
        if (channel->getTopicProtected() && !channel->isOperator(client)){
            sendReply(client.getClientSocketfd(), ERR_CHANOPRIVSNEEDED(input[1]));
            return ;
        }
        std::string topic = buffer.substr(buffer.find(":") + 1);
        std::string reply;
        if (topic.empty()){
            channel->setTopic("");
            reply = ":" + client.getHostname() + " TOPIC " + channel->getName() + " :\r\n";
        }
        else {
            channel->setTopic(topic);
            reply = ":" + client.getHostname() + " TOPIC " + channel->getName() + " :" + topic + "\r\n";
            channel->setTopicsetAtime(std::time(0));
            channel->sethowsetTopic(client.getName());
        }
        for (size_t i = 0; i < channel->members.size(); ++i)
        {
            sendReply(channel->members[i].getClientSocketfd(), reply);
        }
    }
    else {
        std::string currentTopic = channel->getTopic();
        if (currentTopic.empty())
            sendReply(client.getClientSocketfd(), RPL_NOTOPIC(client.getName(), channel->getName()));
        else {
            sendReply(client.getClientSocketfd(), RPL_TOPIC(client.getName(), channel->getName(), currentTopic));
            sendReply(client.getClientSocketfd(), RPL_TOPICWHOTIME(client.getName(), channel->getName(), channel->gethowsetTopic(), toString(channel->getTopicsetAtime())));
        }
    }
}