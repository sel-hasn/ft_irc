#include "Server.hpp"

void    Server::sendReply(int cSocketfd, std::string message){
    if (send(cSocketfd, message.c_str(), message.length(), 0) < 0){
        std::cout << "send() failed !\n";
        return ;
    }
}

Channel* Server::getChannel(std::string name)
{
    for (size_t i = 0; i < Channels.size(); i++) {
        if (Channels[i].getName() == name) {
            return &Channels[i];
        }
    }
    return NULL;
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

Client* Server::getClient_byfd(int fd){
    if (Clients.size() == 0)
        return NULL;
    for(size_t i = 0; i < Clients.size(); i++){
        if (Clients[i].getClientSocketfd() == fd){
            return &Clients[i];
        }
    }
    return NULL;
}

void eraser_samenewlines(std::string& receivedData){
    if (receivedData.length() == 0)
        return ;
    for (size_t i = 0; i < receivedData.length(); ) {
        if (receivedData[i] == '\n' || receivedData[i] == '\r')
            receivedData.erase(i, 1);
        else
            i++;
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


void Server:: erasing_fd_from_poll_vecteurs(int fd){
    for (size_t i = 1; i < PollFDs.size(); i++)
    {
        if (PollFDs[i].fd == fd){
            PollFDs.erase(PollFDs.begin() + i);
            break ;
        }
    }
}
void Server:: erasing_fd_from_client_vecteurs(int fd){
    for (size_t i = 0; i < Clients.size(); i++)
    {
        if (Clients[i].getClientSocketfd() == fd){
            Clients.erase(Clients.begin() + i);
            break ;
        }
    }
}

void Server:: server_ends(){
    for (;PollFDs.size() > 0;)
    {
        PollFDs.erase(PollFDs.begin());
    }
    for (;Clients.size() > 0;)
    {
        close(Clients[0].getClientSocketfd());
        Clients.erase(Clients.begin());
    }
    close(ServerSocketFD);
}

void  Server::treating_commands(Client *client){
    if (client->getBUFFER().length() == 0)
        return ;
    std::string buffer = client->getBUFFER();
    eraser_samenewlines(buffer);
    if (buffer.length() == 0)
        return ;
    std::vector<std::string> input = split(buffer);
    // std::cout <<buffer<<std::endl<<std::endl;
    if ((client->gethasPass() || !client->gethasPass()) && !input.size())
        return ;
    if (!client->gethasPass() && input[0] != "PASS"){
        sendReply(client->getClientSocketfd(), ERR_NOTREGISTERED);
        return ;
    }
    if (input[0] == "PASS")
        PASS_cmd(client, buffer);
    else if (input[0] == "NICK")
        NICK_cmd(client, buffer);
    else if (input[0] == "USER")
        USER_cmd(client, buffer);
    else if (!client->getisRegistered()){
        sendReply(client->getClientSocketfd(), ERR_NOTREGISTERED);
        return ;
    }
    else if (input[0] == "JOIN")
        Join(*client, input);
    else if (input[0] == "PRIVMSG")
        Privmsg(*client, input);
    else if (input[0] == "KICK")
        Kick(*client, input, buffer);
    else if (input[0] == "INVITE")
        Invite(*client, input);
    else if (input[0] == "TOPIC")
        Topic(*client, input, buffer);
    else if (input[0] == "MODE")
        Mode(*client, input);
    else {
        sendReply(client->getClientSocketfd(), ERR_UNKNOWNCOMMAND(client->getName(), input[0]));
        return ;
    }
}