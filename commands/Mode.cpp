#include "../Server.hpp"
void Server::Mode(Client client, std::vector<std::string> input)
{
    if (input.size() < 2)
    {
        sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
        return;
    }
    Channel *channel = getChannel(input[1]);
    if (!channel)
    {
        sendReply(client.getClientSocketfd(), ERR_NOSUCHCHANNEL(input[1]));
        return;
    }
    if (!channel->inChannel(client))
    {
        sendReply(client.getClientSocketfd(), ERR_NOTONCHANNEL(client.getName(), input[1]));
        return;
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
            mode += "k";
            argc += channel->getPass() + " ";
        }
        if (channel->get_UserLimitFlag())
        {
            mode += "l";
            std::stringstream ss;
            ss << channel->getUserLimit();
            argc += ss.str();
        }

        sendReply(client.getClientSocketfd(), RPL_CHANNELMODEIS(client.getName(), channel->getName(), mode, argc));
        return;
    }

    if (!channel->isOperator(client))
    {
        sendReply(client.getClientSocketfd(), ERR_CHANOPRIVSNEEDED(input[1]));
        return;
    }
    
    std::string modes = input[2];
    if (modes.empty() || (modes[0] != '+' && modes[0] != '-'))
    {
        sendReply(client.getClientSocketfd(), ERR_UNKNOWNMODE(client.getName(), input[1], modes));
        return;
    }

    bool add = true;  // Tracks whether we're adding or removing modes
    size_t paramIndex = 3; // Start looking for parameters from input[3]
    std::string modeChanges; // For reply, store all applied modes (+i-t+k etc.)

    for (size_t i = 0; i < modes.size(); ++i)
    {
        char c = modes[i];
        if (c == '+')
        {
            add = true;
            modeChanges += c;
            continue;
        }
        else if (c == '-')
        {
            add = false;
            modeChanges += c;
            continue;
        }

        // Process mode character c:
        bool needsParam = (c == 'k' || c == 'o' || c == 'l');
        std::string param;

        if (needsParam)
        {
            if (paramIndex >= input.size())
            {
                sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
                return;
            }
            param = input[paramIndex++];
        }

        switch (c)
        {
            case 'i':
                channel->setInviteOnly(add);
                modeChanges += c;
                break;
            case 't':
                channel->setTopicProtected(add);
                modeChanges += c;
                break;
            case 'k':
                if (add)
                {
                    channel->set_pass_flag(true);
                    channel->setPass(param);
                }
                else
                {
                    channel->set_pass_flag(false);
                    channel->setPass("");
                }
                modeChanges += c;
                break;
            case 'o':
            {
                Client *target = getClient(param);
                if (!target || !channel->inChannel(*target))
                {
                    sendReply(client.getClientSocketfd(), ERR_USERNOTINCHANNEL(param, input[1]));
                    return;
                }
                if (add)
                {
                    channel->admines.push_back(*target);
                }
                else
                {
                    channel->RemoveOperator(*target);
                }
                modeChanges += c;
                break;
            }
            case 'l':
                if (add)
                {
                    int limit = 0;
                    bool valid = true;
                    std::string limitStr = param;
                    if (limitStr.empty())
                    {
                        sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
                        return;
                    }
                    for (size_t j = 0; j < limitStr.size(); ++j)
                    {
                        if (!isdigit(limitStr[j]))
                        {
                            valid = false;
                            break;
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
                    modeChanges += c;
                }
                else
                {
                    channel->setUserLimit(0);
                    channel->set_UserLimitFlag(false);
                    modeChanges += c;
                }
                break;
            default:
                // Unknown mode char
                sendReply(client.getClientSocketfd(), ERR_UNKNOWNMODE(client.getName(), input[1], std::string(1, c)));
                return;
        }
    }

    sendReply(client.getClientSocketfd(), RPL_CHANGEMODE(client.getHostname(), channel->getName(), modeChanges));
}
