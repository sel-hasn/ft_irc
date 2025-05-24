
#include "Server.hpp"

bool Server::Signal = false;

Server::Server(int port, std::string Pass): ServerSocketFD(-1), Port(port), PassWord(Pass){
}

void    Server::sendReply(int cSocketfd, std::string message){
    send(cSocketfd, message.c_str(), message.length(), 0);
    close(cSocketfd);
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

static void eraser_samenewlines(std::string& receivedData){
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
            // Everything else is part of realname
            std::string rest;
            std::getline(iss, rest);
            tokens.back() += rest; // append the rest
            break;
        }
    }
    return tokens;
}


void Server::erasing_fd_from_vecteurs(int fd){
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

/////////////////////////////////////////////////////////////////////////
////////////////////////utils////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////

void    Server::ServerPrepa(){
    std::memset(&SAddress, 0, sizeof(SAddress));
    SAddress.sin_family = AF_INET;
    SAddress.sin_port = htons(Port);
    SAddress.sin_addr.s_addr = INADDR_ANY;

    ServerSocketFD = socket(PF_INET, SOCK_STREAM, 0);
    if (ServerSocketFD < 0)
        throw CustomException("Failed to create socket\n");
    /**/
    int en = 1;
    if(setsockopt(ServerSocketFD, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) //-> set the socket option (SO_REUSEADDR) to reuse the address
        throw(CustomException("faild to set option (SO_REUSEADDR) on socket"));
    if (fcntl(ServerSocketFD, F_SETFL, O_NONBLOCK) == -1) //-> set the socket option (O_NONBLOCK) for non-blocking socket
        throw(CustomException("faild to set option (O_NONBLOCK) on socket"));
    /**/ 
    std:: cout << Port << ' ' << PassWord << std::endl;
    if (bind(ServerSocketFD, (struct sockaddr*)&SAddress, sizeof(SAddress)) < 0)
        throw CustomException("Bind failed\n");

    if (listen(ServerSocketFD, 128) < 0)
        throw CustomException("Listen failed\n");

    pollfd server_pollfd;
    std::memset(&server_pollfd, 0, sizeof(server_pollfd));
    server_pollfd.fd = ServerSocketFD;
    server_pollfd.events = POLLIN;
    server_pollfd.revents = 0;
    PollFDs.push_back(server_pollfd);
}

void    Server::ServerStarts(){
    try{
        ServerPrepa();
    }
    catch(const CustomException& e){
        close(ServerSocketFD);
        std::cout << "Exception Cought in ServerPrepa :" << e.msg();
        throw CustomException("ServerStarts() -> ServerPrepa()");
    }
    while (!Signal){
        if ((poll(PollFDs.data(), PollFDs.size(), -1) == -1) && !Signal){
            throw CustomException("ServerStarts() : Poll( ) failed.");
        }
        for (size_t i = 0; i < PollFDs.size(); ++i) {
            if (PollFDs[i].revents & POLLIN) {
                if (PollFDs[i].fd == ServerSocketFD)
                    handleNewClient(); // accepting clients
                else
                {
                    try{
                        handleClientData(getClient(PollFDs[i].fd));
                    }
                    catch(...){}
                }
            }
        }
    }
    //close all sockets before leaving the door;
}

void Server::handleNewClient(){
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept(ServerSocketFD, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        std::cerr << "accept () failed .\n";
        return;
    }
    // if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1)
	// {
    //     std::cerr << "fcntl() failed" << std::endl;
    //     return ;
    // }
    pollfd client_pollfd;
    client_pollfd.fd = client_fd;
    client_pollfd.events = POLLIN;
    client_pollfd.revents = 0;
    PollFDs.push_back(client_pollfd);
    
    Client newClient;
    newClient.setClientsock(client_fd);
    newClient.setClientAddress(client_addr);
    newClient.setRegister(false);
    Clients.push_back(newClient);


    std::cout << "New client connected: " << client_fd << std::endl;
}

void Server::PASS_cmd(Client *clint, std::string &buffer){
    if (buffer.length() > 128){
        send(clint->getClientSocketfd(), "ERROR : Hey Password by rules is not contains alot of chars 128 max\n", 69, 0);
        std::cerr << "Client did write alot of chars.\n";
        erasing_fd_from_vecteurs(clint->getClientSocketfd());
    }
    std::vector<std::string> splited = split(buffer);
    if (splited.size() != 2){
        send(clint->getClientSocketfd(), "Please enter a Valid pass: PASS <password> (2 args, PASS counted)\n", 67, 0);
        erasing_fd_from_vecteurs(clint->getClientSocketfd());
        return ;
    }
    else{
        if (splited[1] != getPassword()){
            send(clint->getClientSocketfd(), "incorrect Password\n", 67, 0);
            erasing_fd_from_vecteurs(clint->getClientSocketfd());
            return ;
        }
        else{
            clint->setPass(splited[1]);
            clint->sethasPass(true);
        }
    }
}

void  Server::treating_commands(Client *clint){
        std::string buffer = clint->getBUFFER();
        eraser_samenewlines(buffer);
        std::cout << buffer << std::endl;
        std::vector<std::string> input = split(buffer);
        if (input[0] == "PASS")
            //do PASS
            ;
        else if (input[0] == "NICK")
            //do NICK
            ;
        else if (input[0] == "USER")
            //do USER
            ;
        else if (input[0] == "JOIN")
            //do JOIN
            ;
        else if (input[0] == "KICK")
            //do KICK
            ;
        else if (input[0] == "INVITE")
            //do INVITE
            ;
        else if (input[0] == "TOPIC")
            //do TOPIC
            ;
        else if (input[0] == "MODE")
            //do MODE
            ;
        else if (input[0] == "PRIVMSG")
            //do PRIVMSG
            ;
        // if (!clint->getisRegistered()){
        //     if (clint->gethasPass() && std::strncmp(buffer.c_str(), "PASS ", 5) == 0)
        //         PASS_cmd(clint, buffer);
        //     else {
        //         send(clint->gethasPass(), "Please enter a Valid pass: PASS <password> (Pass as first cmd)\n", 64, 0);
        //         erasing_fd_from_vecteurs(clint->getClientSocketfd());
        //         close(clint->getClientSocketfd());
        //     }
                       
        //     // if (clint->gethasPass() && !clint->hasName && std::strncmp(clint->BUFFER.c_str(), "NICK ", 6) == 0)
        //     //     NICK_cmd(clint);
        //     // if (clint->gethasPass() && !clint->hasUserName && std::strncmp(clint->BUFFER.c_str(), "USER ", 6) == 0)
        //     //     USER_cmd(clint);

        // }
        // else{
        //     // part of you folks
        // }
}

void Server::handleClientData(Client *clint){
    char    buffer[1024];
    std::memset(buffer, 0, 1024);
    if (!clint){
        std::cout << "maybe forget about it !\n" ;
        return ;
    }
    int bytesrecieved = recv(clint->getClientSocketfd(), buffer, 1023, 0);
    switch (bytesrecieved)
    {
        case (-1):{
            // std::cerr << "recv() failed\n";
            const char *msg = "an issue appeared !\n";
            send(clint->getClientSocketfd(), msg, sizeof(msg), 0);
            close(clint->getClientSocketfd());
            return ;
        }
        case (0):{
            // std::cerr << "Client didn't sent any\n";
            const char *msg = "u re disconnected !\n";
            send(clint->getClientSocketfd(), msg, sizeof(msg), 0);
            close(clint->getClientSocketfd());
            return ;
        }
        default:{
            clint->setBuff(std::string(buffer));
            treating_commands(clint);
        }
    }
}

Server::~Server(){
    // need to close all sockets && connections between server and clients !
}