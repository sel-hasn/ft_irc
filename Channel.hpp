#include "Server.hpp"

class Channel
{
    private:
        std::string chan_name;
        std::string password;
        bool pass_flag;
    public:

        std::vector<Client> members;
        std::vector<Client> admines;

        Channel() : chan_name(""), password(""), pass_flag(false){}

        Channel(const Channel &other)
        {
            *this = other;
        }

        Channel &operator=(const Channel &other)
        {
            if (this == &other)
                return (*this);
            this->chan_name = other.chan_name;
            this->password = other.password;
            this->pass_flag = other.pass_flag;
            return (*this);
        }
        ~Channel(){}

        void set_chan_name(std::string name)
        {
            chan_name = name;
        }
        void set_password(std::string pass)
        {
            password = pass;
        }
        void set_pass_flag(bool h)
        {
            pass_flag = h;
        }

        std::string get_chan_name()
        {
            return (chan_name);
        }
        std::string get_password()
        {
            return (password);
        }
        int get_pass_flag()
        {
            return (pass_flag);
        }
};