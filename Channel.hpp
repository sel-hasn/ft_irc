#pragma once

#include "Server.hpp"
#include "Client.hpp"
#include <vector>

class Client;

class Channel
{
    private:
        std::string Name;
        std::string Pass;
        std::string topic;
        bool pass_flag;
        bool InviteOnly;
        bool TopicProtected;
        bool UserLimitFlag;
        int UserLimit;
        time_t TopicsetAtime;
        std::string howsetTopic;
    public:

        std::vector<Client> members;
        std::vector<Client> admines;

        Channel() : Name(""), Pass(""), topic(""), pass_flag(false), InviteOnly(false), TopicProtected(false)
        , UserLimitFlag(false), UserLimit(0){}

        Channel(const Channel &other)
        {
            *this = other;
        }

        Channel &operator=(const Channel &other)
        {
            if (this == &other)
                return (*this);
            this->Name = other.Name;
            this->Pass = other.Pass;
            this->pass_flag = other.pass_flag;
            for (size_t i = 0; i < other.admines.size(); i++)
            {
                this->admines.push_back(other.admines[i]);
            }
            for (size_t i = 0; i < other.members.size(); i++)
            {
                this->members.push_back(other.members[i]);
            }
            this->TopicProtected = other.TopicProtected;
            return (*this);
        }
        ~Channel(){}
        void set_UserLimitFlag(bool _UserLimitFlag)
        {
            UserLimitFlag = _UserLimitFlag;
        }
        bool get_UserLimitFlag()
        {
            return UserLimitFlag;
        }
        void setUserLimit(int _UserLimit)
        {
            UserLimit = _UserLimit;
        }
        int getUserLimit()
        {
            return UserLimit;
        }
        void setInviteOnly(bool _InviteOnly)
        {
            InviteOnly = _InviteOnly;
        }
        bool getInviteOnly()
        {
            return InviteOnly;
        }
        void sethowsetTopic(std::string _howsetTopic)
        {
            howsetTopic = _howsetTopic;
        }
        std::string gethowsetTopic()
        {
            return howsetTopic;
        }
        void setTopicsetAtime(time_t _TopicsetAtime)
        {
            TopicsetAtime = _TopicsetAtime;
        }
        time_t getTopicsetAtime()
        {
            return TopicsetAtime;
        }
        void setTopicProtected(bool Protected)
        {
            TopicProtected = Protected;
        }
        bool getTopicProtected()
        {
            return TopicProtected;
        }
        void setTopic(std::string _topic)
        {
            topic = _topic;
        }
        std::string getTopic()
        {
            return topic;
        }
        void setName(std::string name)
        {
            Name = name;
        }
        void setPass(std::string pass)
        {
            Pass = pass;
        }
        void set_pass_flag(bool h)
        {
            pass_flag = h;
        }

        std::string getName()
        {
            return (Name);
        }
        std::string getPass()
        {
            return (Pass);
        }
        int get_pass_flag()
        {
            return (pass_flag);
        }
        bool inChannel(Client user)
        {
            for (size_t i = 0; i < members.size(); ++i){
                if (members[i].getName() == user.getName()){
                    return true;
                }
            }
            return false;
        }
        bool isOperator(Client user)
        {
            for (size_t i = 0; i < admines.size(); ++i){
                if (admines[i].getName() == user.getName()){
                    return true;
                }
            }
            return false;
        }
        void RemoveMember(Client user)
        {
            for (size_t i = 0; i < members.size(); ++i){
                if (members[i].getName() == user.getName()){
                    members.erase(members.begin() + i);
                    break ;
                }
            }
        }
        void RemoveOperator(Client user)
        {
            for (size_t i = 0; i < admines.size(); ++i){
                if (admines[i].getName() == user.getName()){
                    admines.erase(admines.begin() + i);
                    break ;
                }
            }
        }
};