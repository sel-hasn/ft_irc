
#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>

#include "CustomException.hpp"
#include "Client.hpp"

class Server{
    private:
        Server& operator=(const Server& other);
        Server(const Server& other);
        Server();

    public:
        Server(int port, std::string Password);
        ~Server();
        // parametrize constructor


        void                newClientHandler(int cSockerfd, sockaddr_in Client__Address);
        int                 ServerSocketFD;
        int                 Port;
        std::string         PassWord;
        std::vector<Client>    Clients;
        sockaddr_in         SAddress;

};


int ParsePort(const std::string& av1);
void PasswordParse(std::string av2);

#endif