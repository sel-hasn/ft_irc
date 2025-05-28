
#include "Server.hpp"

bool Server::Signal = false;

Server::Server(int port, std::string Pass): ServerSocketFD(-1), Port(port), PassWord(Pass), ircName("*"){
}

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
                    catch(const CustomException& e){
                        erasing_fd_from_vecteurs(PollFDs[i].fd);
                        close(PollFDs[i].fd);
                        std::cout << "cought exception inside server's poll's func() :" << e.msg();
                    }
                }
            }
        }
    }
    // close all sockets before leaving the door;
}

void Server::handleNewClient(){
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept(ServerSocketFD, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        std::cerr << "accept () failed .\n";
        return;
    }
    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1){//-> set the socket option (O_NONBLOCK) for non-blocking socket
        std::cout << "fcntl() failed" << std::endl; 
        return;
    }
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

void  Server::treating_commands(Client *clint){
    if (clint->getBUFFER().length() == 0)
        return ;
    std::string buffer = clint->getBUFFER();
    eraser_samenewlines(buffer);
    if (buffer.length() == 0)
        return ;
    std::vector<std::string> input = split(buffer);
    if (input[0] == "PASS")
        PASS_cmd(clint, buffer);
    // else if (clint->gethasPass()){
        if (input[0] == "NICK")
            NICK_cmd(clint, buffer);
        else if (input[0] == "USER")
            USER_cmd(clint, buffer);
        // else if (input[0] == "USER")
        //     //do USER
        //     ;
        // else if (input[0] == "JOIN")
        //     //do JOIN
        //     ;
        // else if (input[0] == "KICK")
        //     //do KICK
        //     ;
        // else if (input[0] == "INVITE")
        //     //do INVITE
        //     ;
        // else if (input[0] == "TOPIC")
        //     //do TOPIC
        //     ;
        // else if (input[0] == "MODE")
        //     //do MODE
        //     ;
        // else if (input[0] == "PRIVMSG")
        //     //do PRIVMSG
        //     ;
        else {
            sendReply(clint->getClientSocketfd(), ERR_UNKNOWNCOMMAND(clint->getName(), buffer));
        }
    // }
    // else{
    //     sendReply(clint->getClientSocketfd(), ERR_NOTREGISTERED);
    //     erasing_fd_from_vecteurs(clint->getClientSocketfd());
    //     std::cout << "client with sockfd :" << clint->getClientSocketfd() << " is disconnected !\n";
    //     close(clint->getClientSocketfd());
    // }
}

void Server::handleClientData(Client *clint){
    char    buffer[1024];
    std::memset(buffer, 0, 1024);
    if (!clint)
        throw CustomException("client is not exist anymore\n");
    int bytesrecieved = recv(clint->getClientSocketfd(), buffer, 1023, 0);
    switch (bytesrecieved)
    {
        case (-1):{
            const char *msg = "an issue appeared !\n";
            send(clint->getClientSocketfd(), msg, sizeof(msg), 0);
            close(clint->getClientSocketfd());
            return ;
        }
        case (0):{
            const char *msg = "u re disconnected !\n";
            send(clint->getClientSocketfd(), msg, sizeof(msg), 0);
            close(clint->getClientSocketfd());
            return ;
        }
        default:{
            clint->setBuff(std::string(buffer));
            treating_commands(clint);
            clint->setBuff("");
        }
    }
}

Server::~Server(){
}