#include "Server.hpp"

bool Server::Signal = false;

Server::Server(int port, std::string Pass): ServerSocketFD(-1), Port(port), PassWord(Pass){
}

Client* Server::getClient(std::string name){
    for(size_t i = 0; i < Clients.size(); i++){
        if (Clients[i].getName() == name){
            return &Clients[i];
        }
    }
    return NULL;
}

void    Server::ServerPrepa(){
    std::memset(&SAddress, 0, sizeof(SAddress));
    SAddress.sin_family = AF_INET;
    SAddress.sin_port = htons(Port);
    SAddress.sin_addr.s_addr = INADDR_ANY;

    ServerSocketFD = socket(PF_INET, SOCK_STREAM, 0);
    if (ServerSocketFD < 0)
        throw CustomException("Failed to create socket\n");
    int en = 1;
    if(setsockopt(ServerSocketFD, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1)
        throw(CustomException("faild to set option (SO_REUSEADDR) on socket"));
    if (fcntl(ServerSocketFD, F_SETFL, O_NONBLOCK) == -1)
        throw(CustomException("faild to set option (O_NONBLOCK) on socket"));
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

void Server::erasing_from_inviting_channels(Client client)
{
    for (size_t i = 0; i < Channels.size(); i++)
    {
        Channels[i].RemoveFromInvited(client);
    }
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
        if (PollFDs[0].revents & POLLIN)
            handleNewClient();
        for (size_t i = 1; i < PollFDs.size(); i++) {
            if (PollFDs[i].revents & POLLIN) {
                int tmp = PollFDs[i].fd;
                std::cout << "Handling client data for fd: " << tmp << std::endl;
                try{
                    handleClientData(tmp);
                }
                catch(const CustomException& e){
                    std::cout << "cought exception inside server's poll's func() :" << e.msg();

                    erasing_from_inviting_channels(*getClient_byfd(tmp));

                    std::vector<std::string> input;
                    input.push_back("JOIN");
                    input.push_back("0");

                    Join(*getClient_byfd(tmp), input);

                    close(tmp);
                    erasing_fd_from_client_vecteurs(tmp);
                    erasing_fd_from_poll_vecteurs(tmp);
                }
            }

        }
    }
    server_ends();
}

void Server::handleNewClient(){
    sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);

    int client_fd = accept(ServerSocketFD, (sockaddr*)&client_addr, &addr_len);
    if (client_fd < 0) {
        std::cerr << "accept () failed .\n";
        return;
    }

    if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1){
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

    std::cout << "New client connected: " << newClient.getClientSocketfd() << std::endl;
}

void Server::handleClientData(int fd){
    char    buffer[1024];
    std::memset(buffer, 0, 1024);
    Client *clint = getClient_byfd(fd);
    if (!clint)
        throw CustomException(" client is not exist anymore\n");
    int bytesrecieved = recv(clint->getClientSocketfd(), buffer, 1023, 0);
    switch (bytesrecieved)
    {
        case (-1):{
            clint->sethasPass(false);
            clint->sethasName(false);
            clint->sethasrealName(false);
            clint->sethasUname(false);
            throw CustomException(" Issue\n");
        }
        case (0):{
            clint->sethasPass(false);
            clint->sethasName(false);
            clint->sethasrealName(false);
            clint->sethasUname(false);
            throw CustomException(" client is disconnecting\n");
        }
        default:{
                clint->addtoBuff(buffer);
                size_t npos = clint->getBUFFER().find('\n');
                if (npos != std::string::npos) {
                    std::string command = clint->getBUFFER().substr(0, npos);
                    clint->setBuff(command);
                    treating_commands(clint);
                    clint->setBuff("");
                }
        }
    }
}

Server::~Server(){
    std::cout << "Server ends gracefully.\n";
    std::cout << "Final state of server:\n";
    std::cout << "CLients size is : " << Clients.size() << " || CLients vecteur : ";
    for(size_t i = 0; i < Clients.size(); i++){
        std::cout << Clients[i].getClientSocketfd() << " ";
    }
    std::cout << std::endl;
    std::cout << "PollFDS size is : " << PollFDs.size() << " || PollFDS vecteur : ";
    for(size_t i = 0; i < PollFDs.size(); i++){
        std::cout << PollFDs[i].fd << " ";
    }
    std::cout << std::endl;
    close(ServerSocketFD);
}