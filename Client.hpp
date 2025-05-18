#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "Server.hpp" 

class Client{
    private:
        Client& operator=(const Client& other);
        // client() //parameterize
    
    public:
        Client(const Client& other){
            (void) other;
        };    
        std::string Name;
        std::string Pass;
        std::string realName;
        int         isRegistered;
        int         ClientSocketfd;
        bool        hasPass;
        bool        hasUserName;
        bool        hasName;
        bool        hasrealName;
        sockaddr_in ClientAddress;
        
        Client(){

        };
        ~Client(){

        };
};

#endif