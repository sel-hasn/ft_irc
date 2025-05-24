/*

    std::string receivedPass(ClientRecivedData, bytesRead);
    if (receivedPass.length() > 128){
        sendReply(cSocketfd, "ERROR : Hey Password by rules is not contains alot of chars 128 max\n");
        std::cerr << "Client did write alot of chars.\n";
    }
    if (std::strncmp(ClientRecivedData, "PASS ", 5) != 0){
        sendReply(cSocketfd, "ERROR : First command must be PASS <your_password>\n");
        std::cerr << "Client did not send PASS as first command.\n";
    }
    else {
        receivedPass = receivedPass.substr(5);
        std::cout << receivedPass << std::endl;
    }
    for (size_t i = 0; i < receivedPass.length(); ) {
        if (receivedPass[i] == '\n' || receivedPass[i] == '\r')
            receivedPass.erase(i, 1);
        else
            ++i;
    }
    std::cout << receivedPass << std::endl;
    if (receivedPass != this->PassWord) {
        sendReply(cSocketfd, "ERROR : Wrong password. U're Disconnecting ...\n");
        std::cerr << "Client provided wrong password. Disconnected.\n";
    }
    else
        std::cout << "\033[32m" << "Password typed in Corrected way \n" << "\033[37m";
    
        /// Nick checking here !
    bzero(ClientRecivedData, sizeof(ClientRecivedData));
    bytesRead = recv(cSocketfd, ClientRecivedData, sizeof(ClientRecivedData), 0);
    if (bytesRead <= 0){
        close(cSocketfd);
        std::cerr << "recv() failed\n";
        return;
    }
    
    std::string receivedNick(ClientRecivedData, bytesRead);
    if (std::strncmp(ClientRecivedData, "NICK ", 5) == 0)
        receivedNick = receivedNick.substr(5);
    else {
        sendReply(cSocketfd, "ERROR : Second command after PASS must be NICK <your_nickname>\n");
        std::cerr << "Client did not send NICK as Second command.\n";
    }
    
    for (size_t i = 0; i < receivedNick.length(); ) {
        if (receivedNick[i] == '\n' || receivedNick[i] == '\r')
            receivedNick.erase(i, 1);
        else
            i++;
    }
    if (receivedNick.empty() || receivedNick.length() > 30) {
        sendReply(cSocketfd, "ERROR :Invalid nickname length\n");
        std::cerr << "Client sent invalid nickname\n";
    }
    for (size_t i = 0; i < receivedNick.length(); i++ ) {
        if (!std::isalpha(receivedNick[i]) && receivedNick[i] != '-' && receivedNick[i] != '_'){
            sendReply(cSocketfd,  "ERROR :Invalid nickname with alpha and - and _ chars accepted only\n");
            std::cerr << "Client sent invalid nickname (chars invalid)\n";
        }
    }
    
    // Check uniqueness
    for (size_t i = 0; i < Clients.size(); i++) {
        if (Clients[i].Name == receivedNick) {
            sendReply(cSocketfd, "ERROR :Nickname already in use\n");
            std::cerr << "Duplicate nickname\n";
        }
    }
    
    // USER cmd parse :
    bzero(ClientRecivedData, 255);
    bytesRead = recv(cSocketfd, ClientRecivedData, 254, 0);
    if (bytesRead <= 0)
        close(cSocketfd);
    std::string receivedUSER(ClientRecivedData, bytesRead);
    if (std::strncmp(ClientRecivedData, "USER ", 5) == 0)
        receivedUSER = receivedUSER.substr(5);
    else {
        sendReply(cSocketfd, "ERROR : third command after NICK must be USER <username> * * : <realname>\n");
        std::cerr << "Client did not send USER as third command.\n";
    }
    if (receivedUSER.empty()){
        sendReply(cSocketfd, "ERROR :Invalid username length\n");
        std::cerr << "Client sent invalid USER\n";
    }
    std::vector<std::string> splited = split(receivedUSER);
    if (splited.size() < 4){
        sendReply(cSocketfd, "ERROR :Invalid username args\n");
        std::cerr << "Client sent invalid USER args\n";
    }
    
    
    Client *first = getClient(cSocketfd);
    first->Name = receivedNick;
    first->Pass = receivedPass;
    first->realName = splited[3];
    first->UserName = splited[0];
    first->hasPass = true;
    
    std::cout << "Client :: " << first->Name
    << " with UserName : " << first->UserName 
    << " with Socketfd : " << first->ClientSocketfd
    << " with realName : " << first->realName
    << " with Pass : " << first->Pass
    << " \n";

*/