#include "Server.hpp"

void  Server::NICK_cmd(Client *clint, std::string &buffer){
    std::vector<std::string> splited = split(buffer);
    if (splited.size() != 2){
        sendReply(clint->getClientSocketfd(), ERR_NEEDMOREPARAMS(splited[0]));
        std::cerr << "Client sent invalid nickname args\n";
        return ;
    }
    // if (clint->gethasName()){
    //     sendReply(clint->getClientSocketfd(), ERR_ALREADYREGISTRED(ircName));
    //     return;
    // }
    if (splited[1].length() < 4 || splited[1].length() > 9) {
        sendReply(clint->getClientSocketfd(), ERR_NONICKNAMEGIVEN(splited[1]));
        std::cerr << "Client sent invalid nickname length\n";
        return ;
    }
    if (splited[1][0] == ' ' || splited[1][0] == ':' || splited[1][0] == '#' || splited[1][0] == '&' || std::isdigit(splited[1][0])){
        sendReply(clint->getClientSocketfd(), ERR_NOSUCHNICK(splited[1]));
        std::cerr << "Client sent invalid nickname length\n";
        return ;
    }
    for (size_t i = 0; i < splited[1].length(); i++ ) {
        if (!std::isalpha(splited[1][i]) && splited[1][i] != '-' && splited[1][i] != '_'){
            sendReply(clint->getClientSocketfd(),  "ERROR :Invalid nickname with alpha and - and _ chars accepted only\n");
            std::cerr << "Client sent invalid nickname (chars invalid)\n";
            return ;
        }
    }
    
    for (size_t i = 0; i < Clients.size(); i++) {
        if (Clients[i].getName() == splited[1]) {
            sendReply(clint->getClientSocketfd(), ERR_NICKNAMEINUSE(splited[1]));
            std::cerr << "Duplicate nickname\n";
            return ;
        }
    }
    clint->setName(splited[1]);
    clint->sethasName(true);
    if (clint->gethasUserName()){
        sendReply(clint->getClientSocketfd(), RPL_WELCOME(splited[1], "Welcome to irc server !"));
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
        erasing_fd_from_vecteurs(clint->getClientSocketfd());
        close(clint->getClientSocketfd());
        return ;
    }
    else{
        if (splited[1] != getPassword()){
            sendReply(clint->getClientSocketfd(), ERR_PASSWDMISMATCH(clint->getName()));
            erasing_fd_from_vecteurs(clint->getClientSocketfd());
            close(clint->getClientSocketfd());
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
    if (buffer.find(':') == std::string::npos){
        sendReply(clint->getClientSocketfd(), ERR_NEEDMOREPARAMS(splited[0]));
        std::cerr << "Client sent invalid USER without real name ':' \n";
        return ;
    }
    clint->setrealName(splited[3]);
    clint->sethasrealName(true);
    clint->sethasUname(true);
    if (clint->gethasName()){
        sendReply(clint->getClientSocketfd(), RPL_WELCOME(clint->getName(), "Welcome to irc server !"));
        clint->setRegister(true);
    }
}