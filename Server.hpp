
#ifndef SERVER_HPP
#define SERVER_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <vector>
#include <set>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <signal.h>
#include <sstream> 
#include "CustomException.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include <poll.h>
#include "Replies.hpp"
#include <ctime>


class Client;
class Channel;

class Server{
    private:
    Server();
    void                    ServerPrepa();
    int                     ServerSocketFD;
    int                     Port;
    std::string             PassWord;
    std::vector<Client>     Clients;
    std::vector<Channel>    Channels;
    std::vector<pollfd>     PollFDs;
    sockaddr_in             SAddress;
    
    public:
        Server& operator=(const Server& other);
        Server(const Server& other);
        Server(int port, std::string Password);
        ~Server();
        static bool             Signal;

        /* getter  of server */
        std::string             getPassword() const { return PassWord; };
        int                     getPort() const { return Port; };
        int                     getServerSocketFD() const { return ServerSocketFD; };
        
        /* setter  of server */
        void                    setPassword(std::string newpass) { PassWord = newpass; };
        void                    setPort(int newport) { Port = newport; };
        void                    setServerSocketFD(int newfd) { ServerSocketFD = newfd; };

        void                    PASS_cmd(Client *clint, std::string &buffer);
        void                    NICK_cmd(Client *clint, std::string &buffer);
        void                    USER_cmd(Client *clint, std::string &buffer);
        void                    treating_commands(Client *clint);
        void                    handleClientData(Client *clint);
        void                    sendReply(int cSockfd, std::string message);
        void                    handleNewClient();
        void                    ServerStarts();
        static void             Signals_handler(int signum);
        Client*                 getClient(int fd);
        Client*                 getClient(std::string name);
        void                    erasing_fd_from_poll_vecteurs(int fd);
        void                    erasing_fd_from_client_vecteurs(int fd);
        void                    server_ends();

        Channel *getChannel(std::string name);

        //COMANDS

        void Kick(Client client, std::vector<std::string> input, std::string buffer);
        void Invite(Client client, std::vector<std::string> input);
        void Topic(Client client, std::vector<std::string> input, std::string buffer);
        void Mode(Client client, std::vector<std::string> input);
};

int ParsePort(const std::string& av1);
void PasswordParse(std::string av2);
std::vector<std::string> split(const std::string& str);
void eraser_samenewlines(std::string& receivedData);

#endif