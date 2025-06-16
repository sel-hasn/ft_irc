#pragma once

#include "../Server.hpp"
#include "../CustomException.hpp"

class CustomException;

class bot {
private:
    int  serverport;
    int  sockfd;
    char *serverHostname;
    char *password;
public:
    bot(char *hostname, int port, char *password);
    ~bot();

    void startbot();
    void connectToServer();
    void authenticate();
    void sendReply(const std::string &msg);

};