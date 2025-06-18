#include "Server.hpp"

static int check_names(std::string splited, int flag){
    if (flag){
        if (splited.length() < 1)
            return -1;
        for (size_t i = 0; i < splited.length(); i++ ) {
            if (!std::isalnum(splited[i]) && splited[i] != '-'
                && splited[i] != '_' && splited[i] != '.'){
                return -1;
            }
        }
    }
    else {
        std::cout << "splited >" << splited << std::endl;
        // if (splited[0] != ':')
        //     return -1;
        for (size_t i = 1; i < splited.length(); i++ ) {
            if (!std::isalpha(splited[i]) && splited[i] != '-' 
                && splited[i] != '_' && splited[i] != 32){
                return -1;
            }
        }
    }
    return 1;
}

static int check_secthird(std::string splited){
    if(splited.length() != 1)
        return 1;
    if(splited[0] != '0' && splited[0] != '*')
        return 1;
    return 0;
}

void  Server::NICK_cmd(Client *clint, std::string &buffer){
    std::vector<std::string> splited = split(buffer);
    if (splited.size() != 2){
        sendReply(clint->getClientSocketfd(), ERR_NEEDMOREPARAMS(splited[0]));
        std::cerr << "Client sent invalid nickname args\n";
        return ;
    }
    if (splited[1].length() < 3) {
        sendReply(clint->getClientSocketfd(), ERR_NONICKNAMEGIVEN(splited[1]));
        std::cerr << "Client sent invalid nickname length\n";
        return ;
    }
    if (std::isspace(splited[1][0]) || splited[1][0] == ':' || splited[1][0] == '#' || splited[1][0] == '&' || std::isdigit(splited[1][0])){
        sendReply(clint->getClientSocketfd(), ERR_NOSUCHNICK(splited[1]));
        std::cerr << "Client sent invalid nickname leadchar\n";
        return ;
    }
    if (check_names(splited[1], 1) == -1){
        sendReply(clint->getClientSocketfd(),  ERR_ERRONEUSNICKNAME(splited[1]));
        std::cerr << "Client sent invalid nickname (chars invalid)\n";
        return ;
    }
    for (size_t i = 0; i < Clients.size(); i++) {
        if (Clients[i].getName() == splited[1]) {
            sendReply(clint->getClientSocketfd(), ERR_NICKNAMEINUSE(splited[1]));
            std::cerr << "Duplicate nickname\n";
            return ;
        }
    }
    if (clint->gethasName() && clint->getisRegistered()){
        std::cout << ":" << clint->getName() << " changed his nickname to " << splited[1] << std::endl;
        clint->setName(splited[1]);
        return ;
    }
    clint->setName(splited[1]);
    clint->sethasName(true);
    if (clint->gethasUserName()){
        sendReply(clint->getClientSocketfd(), RPL_WELCOME(splited[1], "Welcome to irc server"));
        clint->setRegister(true);
    }
}

void Server::PASS_cmd(Client *clint, std::string &buffer){
    if (clint->gethasPass()){
        sendReply(clint->getClientSocketfd(), ERR_ALREADYREGISTRED(clint->getName()));
        return ;
    }
    std::vector<std::string> splited = split(buffer);
    if (splited.size() != 2){
        sendReply(clint->getClientSocketfd(), ERR_NEEDMOREPARAMS(splited[0]));
        return ;
    }
    else{
        if (splited[1] != getPassword()){
            sendReply(clint->getClientSocketfd(), ERR_PASSWDMISMATCH(clint->getName()));
            return ;
        }
        else{
            clint->setPass(splited[1]);
            clint->sethasPass(true);
        }
    }
}


void  Server::USER_cmd(Client *clint, std::string &buffer){
    std::vector<std::string> splited = split(buffer);
    if (clint->gethasUserName()){
        sendReply(clint->getClientSocketfd(), ERR_ALREADYREGISTRED(clint->getName()));
        return;
    }
    if (splited.size() != 5){
        sendReply(clint->getClientSocketfd(), ERR_NEEDMOREPARAMS(splited[0]));
        std::cerr << "Client sent invalid USER args\n";
        return ;
    }
    if (check_names(splited[1], 1) == -1){
        sendReply(clint->getClientSocketfd(), ERR_NEEDMOREPARAMS(splited[0]));
        std::cerr << "Client sent invalid USER Username ! \n";
        return ;
    }
    if (check_names(splited[4], 0) == -1){
        sendReply(clint->getClientSocketfd(), ERR_NEEDMOREPARAMS(splited[0]));
        std::cerr << "Client sent invalid USER without real name ':' \n";
        return ;
    }
    if (check_secthird(splited[2]) || check_secthird(splited[3])){
        sendReply(clint->getClientSocketfd(), ERR_NEEDMOREPARAMS(splited[0]));
        std::cerr << "Client sent invalid second and third params \n";
        return ;
    }
    std::string newrealname;
    size_t i = 1;
    for (; i < splited[4].length(); i++)
    {
        if (!std::isspace(splited[4][i]))
            break ;
    }
    for (; i < splited[4].length(); i++)
    {
        newrealname += splited[4][i];
    }
    std::cout << "new new real name is >" << newrealname << std::endl;
    clint->setrealName(newrealname);
    clint->sethasrealName(true);
    clint->sethasUname(true);
    clint->setUserName(splited[1]);
    if (clint->gethasName()){
        sendReply(clint->getClientSocketfd(), RPL_WELCOME(clint->getName(), "Welcome to irc server"));
        clint->setRegister(true);
    }
}