#include "../Server.hpp"
#include <algorithm>

void Server::Join(Client client, std::vector<std::string> input)
{
	if (input.size() == 1)
	{
		sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
		return ;
	}
	if (input[1] == "0")
    {
        for (size_t i = 0; i < Channels.size(); )
        {
            bool erasedChannel = false;

            for (size_t j = 0; j < Channels[i].members.size(); j++)
            {
                if (Channels[i].members[j].getName() == client.getName())
                {
                    std::string prefix = client.getHostname();
                    std::string channelName = Channels[i].getName();

                    std::string partMsg = ":" + prefix + " PART " + channelName + "\r\n";

                    for (size_t k = 0; k < Channels[i].members.size(); k++)
                    {
                        sendReply(Channels[i].members[k].getClientSocketfd(), partMsg);
                    }

                    if (Channels[i].members.size() == 1)
                    {
                        Channels.erase(Channels.begin() + i);
                        erasedChannel = true;
                        break;
                    }
                    else
                    {
                        if (j + 1 < Channels[i].members.size())
                        {
                            Client newAdmin = Channels[i].members[j + 1];
                            for (size_t idx = 0; idx < Channels[i].admines.size(); idx++)
                            {
                                if (Channels[i].admines[idx].getName() == client.getName())
                                {
                                    Channels[i].admines.erase(Channels[i].admines.begin() + idx);
									size_t c = 0;
									for (; c < Channels[i].admines.size(); c++)
									{
										if (Channels[i].admines[c].getName() == newAdmin.getName())
											break ;
									}
									if (c == Channels[i].admines.size())
									{
                                    	Channels[i].admines.push_back(newAdmin);
										Mode(Channels[i].admines[idx], split("MODE " + Channels[i].getName() + " +o " + newAdmin.getName() + "\r\n"));
									}
                                    break;
                                }
                            }
                        }
                        Channels[i].members.erase(Channels[i].members.begin() + j);
                        break;
                    }
                }
            }
            if (!erasedChannel)
                ++i;
        }
        return;
    }
	std::vector<std::string> str_names;
	std::vector<std::string> str_keys;
	for (size_t i = 0; i < input[1].size(); i++)
	{
		std::string chan_name;
		for (; input[1][i] && input[1][i] != ','; i++)
			chan_name.push_back(input[1][i]);
		str_names.push_back(chan_name);
	}


	if (input.size() != 2)
	{
		std::string buffer;
		for (size_t i = 0; input[2][i]; i++)
			buffer.push_back(input[2][i]);
		
		for (size_t i = 0; buffer[i]; i++)
		{
			std::string key;
			for (; buffer[i] && buffer[i] != ','; i++)
				key.push_back(buffer[i]);
			str_keys.push_back(key);
		}
	}

	if (str_names.size() > str_keys.size())
	{
		size_t deff = str_names.size() - str_keys.size();
		size_t i = 0;
		for(; i < str_keys.size(); i++){}
		size_t j = 0;
		for (; j < deff; j++)
		{
			str_keys.push_back("");
		}
	}

	std::vector<std::string>::iterator it_name;
	std::vector<std::string>::iterator it_key;

	it_name = str_names.begin();
	it_key = str_keys.begin();
	std::vector<Channel>::iterator it = Channels.begin();
	for (; it_name != str_names.end(); it_name++, it_key++)
	{
		if (it_name->size() == 0)
		{
			sendReply(client.getClientSocketfd(), ERR_BADCHANMASK(*it_name));
			continue ;
		}
		if (it_name->size() == 1 && *it_name->begin() == '#')
		{
			sendReply(client.getClientSocketfd(), ERR_BADCHANMASK(*it_name));
			continue ;
		}
		if (it_name->size() != 0 && *it_name->begin() != '#')
		{
			sendReply(client.getClientSocketfd(), ERR_BADCHANMASK(*it_name));
			continue ;
		}
		else
		{
			it = Channels.begin();
			for (; it != Channels.end(); it++)
			{	
				if (it->getName() == *it_name)
				{
					break ;
				}
			}
			if (it != Channels.end())
			{
				if (it->inChannel(client))
				{
					sendReply(client.getClientSocketfd(), ERR_USERONCHANNEL(client.getName(), it->getName()));
					continue ;
				}
				if (it->get_pass_flag())
				{
					if (it_key->size() != 0)
					{
						if (it->getPass() != *it_key)
						{
							sendReply(client.getClientSocketfd(), ERR_PASSWDMISMATCH(client.getName()));
							continue ;
						}
					}
					else if (it_key->size() == 0)
					{
						sendReply(client.getClientSocketfd(), ERR_PASSWDMISMATCH(client.getName()));
						continue ;
					}
				}
				if (it->get_UserLimitFlag())
				{
					if (it->getUserLimit() <= (int)it->members.size())
					{
						sendReply(client.getClientSocketfd(), ERR_CHANNELISFULL(client.getName(), *it_name));
						continue ;
					}
				}
                
                if (it->getInviteOnly() && !it->isOperator(client) && !it->isInvited(client))
                {
                    sendReply(client.getClientSocketfd(), ERR_INVITEONLYCHAN(client.getName(), *it_name));
                    continue ;
                }
				if (it->isInvited(client))
				{
					it->RemoveFromInvited(client);
				}
				it->members.push_back(client);
				std::vector<Client>::iterator it_member = it->members.begin();
				for (; it_member != it->members.end(); it_member++)
				{
					sendReply(it_member->getClientSocketfd(), RPL_JOIN(client.getName(), it->getName()));
				}
				if (it->getTopic() == "")
					sendReply(client.getClientSocketfd(), RPL_NOTOPIC(client.getName(), *it_name));
				else 
				{
					sendReply(client.getClientSocketfd(), RPL_TOPIC(client.getName(), it->getName(), it->getTopic()));
            		sendReply(client.getClientSocketfd(), RPL_TOPICWHOTIME(client.getName(), it->getName(), it->gethowsetTopic(), toString(it->getTopicsetAtime())));
				}
				std::string users;
				for (size_t j = 0; j < it->members.size(); j++)
				{
					int flag = 0;
					for (size_t i = 0; i < it->admines.size(); i++)
					{
						flag = 0;
						if (it->admines[i].getName() == it->members[j].getName())
						{
							users.append("@" + it->admines[i].getName());
							flag = 1;
							break ;
						}
					}
					if (flag != 1)
						users.append(it->members[j].getName());
					users.append(" ");
				}
				sendReply(client.getClientSocketfd(), RPL_NAMREPLY(client.getName(), it->getName(), users));
				sendReply(client.getClientSocketfd(), RPL_ENDOFNAMES(client.getName(), it->getName()));
			}
			else
			{
				Channel last_chan;
				last_chan.setName(*it_name);
				if (it_key->size() != 0)
				{
					last_chan.setPass(*it_key);
					last_chan.set_pass_flag(false);
				}
				else if (it_key->size() == 0)
				{
					last_chan.setPass("");
					last_chan.set_pass_flag(false);
				}
				last_chan.set_UserLimitFlag(false);
				last_chan.setInviteOnly(false);
				last_chan.setTopicProtected(false);
				last_chan.members.push_back(client);
				last_chan.admines.push_back(client);
				Channels.push_back(last_chan);
				sendReply(client.getClientSocketfd(), ":" + client.getHostname() + " JOIN :" + *it_name + "\r\n");
				sendReply(client.getClientSocketfd(), RPL_NOTOPIC(client.getName(), *it_name));
				sendReply(client.getClientSocketfd(), RPL_NAMREPLY(client.getName(), *it_name, "@" + client.getName()));
				sendReply(client.getClientSocketfd(), RPL_ENDOFNAMES(client.getName(), *it_name));
			}
		}
	}
}