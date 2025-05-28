#include "Server.hpp"

void    Server::sendReply(int cSocketfd, std::string message){
    send(cSocketfd, message.c_str(), message.length(), 0);
}

Server::Server(const Server& other){
    (void) other;
}
Server& Server::operator=(const Server& other){
    if (this != &other){
        this->setPassword(other.getPassword());
        this->setPort(other.getPort());
        this->setServerSocketFD(other.getServerSocketFD());
    }
    return *this;
}

void Server::Signals_handler(int signum){
    (void)signum;
	std::cout << std::endl << "Signal Received!" << std::endl;
	Server::Signal = true;
}

Client* Server::getClient(int fd){
    for(size_t i = 0; i < Clients.size(); i++){
        if (Clients[i].getClientSocketfd() == fd){
            return &Clients[i];
        }
    }
    return NULL;
}

void eraser_samenewlines(std::string& receivedData){
    for (size_t i = 0; i < receivedData.length(); ) {
        if (receivedData[i] == '\n' || receivedData[i] == '\r')
            receivedData.erase(i, 1);
        else
            ++i;
    }
}

std::vector<std::string> split(const std::string& str) {
    std::vector<std::string> tokens;
    std::istringstream iss(str);
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
        if (!token.empty() && token[0] == ':') {
            std::string rest;
            std::getline(iss, rest);
            tokens.back() += rest;
            break;
        }
    }
    return tokens;
}


void Server:: erasing_fd_from_vecteurs(int fd){
    for (size_t i = 0; i < Clients.size(); i++)
    {
        if (Clients[i].getClientSocketfd() == fd)
            Clients.erase(Clients.begin() + i);
    }
    for (size_t i = 1; i < PollFDs.size(); i++)
    {
        if (PollFDs[i].fd == fd)
            PollFDs.erase(PollFDs.begin() + i);
    }
}