#pragma once

#include "../Server.hpp"
#include "../CustomException.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

class CustomException;

struct player
{
    std::string Nick;
    size_t      quizIndix;
};

struct Quiz {
    std::string question;
    std::string choices[3];
    char answer;
};


class bot {
private:
    int  serverport;
    char *serverHostname;
    char *password;
    int  sockfd;
public:
    bot(char *hostname, int port, char *password);
    ~bot();
    std::vector<player> players;
    std::vector<Quiz> quizDatabase;

    void startbot();
    void connectToServer();
    void authenticate();
    void sendMessage(const std::string &msg);
    player *getplayer(std::string sender);
    void initQuizDatabase();
    void sendrespon(std::string &message, std::string &sender);
    void playGame(std::string &sender, std::string &message);
    void checkanswer(player &p, const std::string &message);
    void sendQuizToPlayer(player &p);
};