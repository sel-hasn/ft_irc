
#include "Server.hpp"

bool Server::Signal = false;

Server::Server(int port, std::string Pass): ServerSocketFD(-1), Port(port), PassWord(Pass){
}

void    Server::sendReply(int cSocketfd, std::string message){
    send(cSocketfd, message.c_str(), message.length(), 0);
    // close(cSocketfd);
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

Client* Server::getClient(std::string name){
    for(size_t i = 0; i < Clients.size(); i++){
        if (Clients[i].getName() == name){
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
    Channel b;
    std::cout << "Creat Channel #j\n";
    b.setName("#j");
    b.setTopicProtected(true);
    Channels.push_back(b);
    std::cout << "Pushed channel: " << Channels.back().getName()
          << " | TopicProtected: " << (Channels.back().getTopicProtected() ? "true" : "false") << "\n";

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
                    std::cout<<"very Good\n";
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

    std::stringstream ss;
    ss << "defaultNick" << Clients.size() + 1;  // e.g., defaultNick1, defaultNick2, ...
    newClient.setName(ss.str());

    Clients.push_back(newClient);

    Channels[0].members.push_back(newClient);
    Channels[0].admines.push_back(newClient);

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

Channel* Server::getChannel(std::string name)
{
    for (size_t i = 0; i < Channels.size(); i++) {
        if (Channels[i].getName() == name) {
            return &Channels[i];
        }
    }
    return NULL;
}

void Server::Kick(Client client, std::vector<std::string> input, std::string buffer)
{
    if (input.size() < 3)
    {
        sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
        return ;
    }
    Channel *channel = getChannel(input[1]);
    if (!channel){
        sendReply(client.getClientSocketfd(), ERR_NOSUCHCHANNEL(input[1]));
        return ;
    }
    Client *toKickclient = getClient(input[2]);
    if (!toKickclient){
        sendReply(client.getClientSocketfd(), ERR_USERNOTINCHANNEL(input[2], input[1]));
        return ;
    }
    if (!channel->inChannel(client)){
        sendReply(client.getClientSocketfd(), ERR_NOTONCHANNEL(client.getName(), input[1]));
        return ;
    }
    std::string couse;
    if (input.size() > 3)
        couse = buffer.substr(buffer.find(":") + 1);
    else 
        couse = "";
    if (channel->isOperator(client)) {
        std::string reply;
        if (couse.empty())
            reply = ":" + client.getHostname() + " KICK " + channel->getName() + " " + toKickclient->getName() + "\r\n";
        else 
            reply = ":" + client.getHostname() + " KICK " + channel->getName() + " " + toKickclient->getName() + " :" + couse + "\r\n";
        for (size_t i = 0; i < channel->members.size(); ++i)
        {
            if (channel->members[i].getName() != toKickclient->getName())
                sendReply(channel->members[i].getClientSocketfd(), reply);
        }
        sendReply(toKickclient->getClientSocketfd(), reply);
        if (channel->isOperator(*toKickclient)){
            channel->RemoveMember(*toKickclient);
            channel->RemoveOperator(*toKickclient);
        }
        else 
            channel->RemoveMember(*toKickclient);
    }
    else 
        sendReply(client.getClientSocketfd(), ERR_CHANOPRIVSNEEDED(input[1]));
}

void Server::Invite(Client client, std::vector<std::string> input)
{
    if (input.size() < 3)
    {
        sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
        return ;
    }
    Channel *channel = getChannel(input[1]);
    if (!channel){
        sendReply(client.getClientSocketfd(), ERR_NOSUCHCHANNEL(input[2]));
        return ;
    }
    if (!channel->inChannel(client)){
        sendReply(client.getClientSocketfd(), ERR_NOTONCHANNEL(client.getName(), input[2]));
        return ;
    }
    Client *toInviteClient = getClient(input[1]);
    if (!toInviteClient){
        sendReply(client.getClientSocketfd(), ERR_NOSUCHNICK(input[1]));
        return ;
    }
    if (channel->inChannel(*toInviteClient))
    {
        sendReply(client.getClientSocketfd(), ERR_USERONCHANNEL(input[1], input[2]));
        return ;
    }
    if (channel->getInviteOnly() && channel->isOperator(client))
    {
        sendReply(client.getClientSocketfd(), ERR_CHANOPRIVSNEEDED(input[2]));
        return ;
    }
    sendReply(client.getClientSocketfd(), RPL_INVITING(client.getName(), toInviteClient->getName(), channel->getName()));
    std::string reply = ":" + client.getHostname() + " INVITE " + toInviteClient->getName() + " " + channel->getName() + " :\r\n";
    channel->members.push_back(*toInviteClient);
    for (size_t i = 0; i < channel->members.size(); ++i)
    {
        sendReply(channel->members[i].getClientSocketfd(), reply);
    }
}

std::string toString(time_t val) {
    std::stringstream ss;
    ss << val;
    return ss.str();
}

void Server::Topic(Client client, std::vector<std::string> input, std::string buffer)
{
    if (input.size() < 2){
        sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
        return ;
    }
    Channel *channel = getChannel(input[1]);
    if (!channel){
        sendReply(client.getClientSocketfd(), ERR_NOSUCHCHANNEL(input[1]));
        return ;
    }
    if (!channel->inChannel(client)){
        sendReply(client.getClientSocketfd(), ERR_NOTONCHANNEL(client.getName(), input[1]));
        return ;
    }
    if (input.size() > 2){
        if (channel->getTopicProtected() && !channel->isOperator(client)){
            sendReply(client.getClientSocketfd(), ERR_CHANOPRIVSNEEDED(input[1]));
            return ;
        }
        std::string topic = buffer.substr(buffer.find(":") + 1);
        std::string reply;
        if (topic.empty()){
            channel->setTopic("");
            reply = ":" + client.getHostname() + " TOPIC " + channel->getName() + " :\r\n";
        }
        else {
            channel->setTopic(topic);
            reply = ":" + client.getHostname() + " TOPIC " + channel->getName() + " :" + topic + "\r\n";
            channel->setTopicsetAtime(std::time(0));
            channel->sethowsetTopic(client.getName());
        }
        for (size_t i = 0; i < channel->members.size(); ++i)
        {
            sendReply(channel->members[i].getClientSocketfd(), reply);
        }
    }
    else {
        std::string currentTopic = channel->getTopic();
        if (currentTopic.empty())
            sendReply(client.getClientSocketfd(), RPL_NOTOPIC(client.getName(), channel->getName()));
        else {
            sendReply(client.getClientSocketfd(), RPL_TOPIC(client.getName(), channel->getName(), currentTopic));
            sendReply(client.getClientSocketfd(), RPL_TOPICWHOTIME(client.getName(), channel->getName(), channel->gethowsetTopic(), toString(channel->getTopicsetAtime())));
        }
    }
}

void Server::Mode(Client client, std::vector<std::string> input)
{
    if (input.size() < 2)
    {
        sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
        return ;
    }
    Channel *channel = getChannel(input[1]);
    if (!channel){
        sendReply(client.getClientSocketfd(), ERR_NOSUCHCHANNEL(input[1]));
        return ;
    }
    if (!channel->inChannel(client)){
        sendReply(client.getClientSocketfd(), ERR_NOTONCHANNEL(client.getName(), input[1]));
        return ;
    }
    if (input.size() == 2)
    {
        std::string mode = "+";
        std::string argc = "";
        if (channel->getInviteOnly())
            mode += "i";
        if (channel->getTopicProtected())
            mode += "t";
        if (channel->get_pass_flag())
        {
            argc = channel->getPass();
            mode += "t";
        }
        sendReply(client.getClientSocketfd(), RPL_CHANNELMODEIS(client.getName(), channel->getName(), mode, argc));
        return;
    }
    if (!channel->isOperator(client)){
        sendReply(client.getClientSocketfd(), ERR_CHANOPRIVSNEEDED(input[1]));
        return ;
    }
    std::string mode = input[2];
    if (mode.length() != 2 || (mode[0] != '+' && mode[0] != '-'))
    {
        sendReply(client.getClientSocketfd(), ERR_UNKNOWNMODE(client.getName(), input[1], mode));
        return;
    }

    bool add;
    
    if (mode[0] == '+')
        add = true;
    else if (mode[0] == '-')
        add = false;
    
    if (mode[1] == 'i')
        channel->setInviteOnly(add);
    else if (mode[1] == 't')
        channel->setTopicProtected(add);
    else if (mode[1] == 'k')
    {
        if (add == true){
            if (input.size() < 4){
                sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
                return;
            }
            channel->set_pass_flag(add);
            channel->setPass(input[3]);
        }
        else {
            channel->set_pass_flag(add);
            channel->setPass("");
        }
    }
    else if (mode[1] == 'o'){
        if (input.size() < 4){
            sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
            return;
        }
        Client *target = getClient(input[3]);
        if (!target || !channel->inChannel(*target)) {
            sendReply(client.getClientSocketfd(), ERR_USERNOTINCHANNEL(input[3], input[1]));
            return;
        }
        if (add){
            channel->admines.push_back(*target);
        }
        else {
            channel->RemoveOperator(*target);
        }
    }
    else if (mode[1] == 'l'){
        if (add){
            if (input.size() < 4){
                sendReply(client.getClientSocketfd(), ERR_NEEDMOREPARAMS(input[0]));
                return;
            }
            int limit = 0;
            bool valid = true;
            std::string limitStr = input[3];
            if (limitStr.empty()){
                return ;
            }
            if (!limitStr.empty()){
                for (size_t i = 0; i < limitStr.size(); ++i){
                    if (!isdigit(limitStr[i])){
                        valid = false;
                        break ;
                    }
                }
            }
            if (valid)
            {
                limit = std::atoi(limitStr.c_str());
                if (limit >= 0)
                {
                    channel->setUserLimit(limit);
                    channel->set_UserLimitFlag(true);
                }
            }   
        }
        else {
            channel->setUserLimit(0);
            channel->set_UserLimitFlag(false);
        }
    }
    else 
        sendReply(client.getClientSocketfd(), ERR_UNKNOWNMODE(client.getName(), input[1], input[2]));
}


void  Server::treating_commands(Client *client){
        std::string buffer = client->getBUFFER();
        eraser_samenewlines(buffer);
        std::vector<std::string> input = split(buffer);
        // if (input[0] == "JOIN")
        //     //do JOIN
        //     ;
        
        if (input.empty())
            return ;
        if (input[0] == "KICK")
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
        // else if (input[0] == "PRIVMSG")
        //     //do PRIVMSG
        //     ;
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