
#include "Server.hpp"

int ParsePort(const std::string& av1){
    int port = -1;
    
    try {
        for (size_t i = 0; i < av1.length(); i++)
        {
            if (!std::isdigit(av1[i]))
                throw -2;
        }
        port = atol(av1.c_str());
        if (port < 1024 || port > 65535)
            throw -4;
    }
    catch(int n){
        if (n == -2)
            throw std::runtime_error("Invalid port: Not a number.");
        if (n == -4)
            throw std::runtime_error("Invalid port: Must be between 1024 and 65535.\n\t\t(Note : ports under 1024 needs a root previleges to use them sorry in advance but we do not have those access)");
    }
    return port;
}

void PasswordParse(std::string av2){
    if (av2.length() == 0)
        throw std::runtime_error("please enter a valid Server's password.");
    if (av2.length() > 128)
        throw std::runtime_error("please enter a valid Server's password. (Password too long (max 128 characters))");
    for (size_t i = 0; i < av2.length(); i++)
    {
        if (!std::isprint(av2[i]))
            throw std::runtime_error("please enter printable chars on password.");
        if (av2[i] == 32)
            throw std::runtime_error("space and whitespaces are not allowed");
    }
}
