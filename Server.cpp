
#include "Server.hpp"

Server::Server(int port, std::string Pass): Port(port), PassWord(Pass){
    std::memset(&SAddress, 0, sizeof(SAddress));
    SAddress.sin_family = AF_INET; // ipv4
    SAddress.sin_port = htons(Port);
    SAddress.sin_addr.s_addr = INADDR_ANY;

        // socket creation
    ServerSocketFD = socket(PF_INET, SOCK_STREAM, 0);
    if (ServerSocketFD == -1)
        throw CustomException("Failed to create socket\n");
    std:: cout << Port << ' ' << PassWord << std::endl;

        // linking socket to internet !
    if (bind(ServerSocketFD, (struct sockaddr*)&SAddress, sizeof(SAddress)) < 0)
        throw CustomException("Bind failed\n");

    // Listen to network and  waiting for expected connections
    if (listen(ServerSocketFD, 128) < 0)
        throw CustomException("Listen failed\n");

    // Accept connections if server wants to !
    while (1){
        int clientfd;
        sockaddr_in   ClientAddress;
    
        
        socklen_t address_size = sizeof(ClientAddress);
        try {
            clientfd = accept(ServerSocketFD, (struct sockaddr*)&ClientAddress, &address_size);
            if (clientfd < 0) 
                throw CustomException("Accept failed\n");
            std::cout << "Client connected: " << inet_ntoa(ClientAddress.sin_addr) << "\n";
            newClientHandler(clientfd, ClientAddress);
        }
        catch(const CustomException& e){
            std::cout << "Server Running Warnings : " << e.msg();
            continue;
        }
        const char* msg = "Welcome to IRC Server!\n";
        send(clientfd, msg, strlen(msg), 0);
        std::cout << "Client authenticated successfully :)\n";
    }
}

Server::~Server(){
    // need to close all sockets && connections between server and clients !
}

void Server::newClientHandler(int cSocketfd, sockaddr_in Client__Address){
    char ClientRecivedData[255];
    bzero(ClientRecivedData, 255);

    ssize_t bytesRead = recv(cSocketfd, ClientRecivedData, 254, 0);
    if (bytesRead <= 0)
        close(cSocketfd);
    std::string receivedPass(ClientRecivedData, bytesRead);
    if (receivedPass.length() > 128){
        const char* msg = "ERROR : Hey Password by rules is not contains alot of chars 128 max\n";
        send(cSocketfd, msg, strlen(msg), 0);
        close(cSocketfd);
        throw CustomException("Client did write alot of chars.\n");
    }

    if (std::strncmp(ClientRecivedData, "PASS ", 5) == 0)
        receivedPass = receivedPass.substr(5);
    else {
        const char* msg = "ERROR : First command must be PASS <your_password>\n";
        send(cSocketfd, msg, strlen(msg), 0);
        close(cSocketfd);
        throw CustomException("Client did not send PASS as first command.\n");
    }
    for (size_t i = 0; i < receivedPass.length(); ) {
        if (receivedPass[i] == '\n' || receivedPass[i] == '\r')
            receivedPass.erase(i, 1);
        else
            ++i;
    }

    if (receivedPass != this->PassWord) {
        const char* msg = "ERROR : Wrong password. U're Disconnecting ...\n";
        send(cSocketfd, msg, strlen(msg), 0);
        close(cSocketfd);
        throw CustomException("Client provided wrong password. Disconnected.\n");
    }
    /// Nick checking here !
    bzero(ClientRecivedData, 255);
    bytesRead = recv(cSocketfd, ClientRecivedData, 254, 0);
    if (bytesRead <= 0)
        close(cSocketfd);
    std::string receivedNick(ClientRecivedData, bytesRead);
    if (std::strncmp(ClientRecivedData, "NICK ", 5) == 0)
        receivedNick = receivedNick.substr(5);
    else {
        const char* msg = "ERROR : Second command after PASS must be NICK <your_nickname>\n";
        send(cSocketfd, msg, strlen(msg), 0);
        close(cSocketfd);
        throw CustomException("Client did not send NICK as Second command.\n");
    }
    for (size_t i = 0; i < receivedNick.length(); ) {
        if (receivedNick[i] == '\n' || receivedNick[i] == '\r')
            receivedNick.erase(i, 1);
        else
            ++i;
    }
    if (receivedNick.empty() || receivedNick.length() > 30) {
        const char* msg = "ERROR :Invalid nickname length\n";
        send(cSocketfd, msg, strlen(msg), 0);
        close(cSocketfd);
        throw CustomException("Client sent invalid nickname\n");
    }
    for (size_t i = 0; i < receivedNick.length(); i++ ) {
        if (!std::isalpha(receivedNick[i]) && receivedNick[i] != '-' && receivedNick[i] != '_'){
            const char* msg = "ERROR :Invalid nickname with alpha and - and _ chars accepted only\n";
            send(cSocketfd, msg, strlen(msg), 0);   
            close(cSocketfd);
            throw CustomException("Client sent invalid nickname (chars invalid)\n");
        }
    }

    // Check uniqueness
    for (size_t i = 0; i < Clients.size(); i++) {
        if (Clients[i].Name == receivedNick) {
            const char* msg = "ERROR :Nickname already in use\n";
            send(cSocketfd, msg, strlen(msg), 0);
            close(cSocketfd);
            throw CustomException("Duplicate nickname\n");
        }
    }

    Client first;
    Clients.push_back(first);
    Client& newClient = Clients.back();
    newClient.ClientSocketfd = cSocketfd;
    newClient.ClientAddress = Client__Address;
    newClient.Name = receivedNick;
    newClient.Pass = receivedPass;
    newClient.hasPass = true;
}