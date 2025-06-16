#include "bot.hpp"

void bot::sendMessage(const std::string &msg)
{
    std::string fullMsg = msg + "\r\n";
    if (send(this->sockfd, fullMsg.c_str(), fullMsg.length(), 0) < 0)
        throw std::runtime_error("Failed to send: " + msg);
}

bot::bot(char *hostname, int port, char *password)
{
    this->serverport = port;
    this->serverHostname = hostname;
    this->password = password;
    this->sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (this->sockfd < 0)
        throw std::runtime_error("Failed to create socket\n");
}

void bot::connectToServer()
{
    struct sockaddr_in serverAddr;
    struct hostent *server;

    server = gethostbyname(this->serverHostname);
    if (server == NULL)
        throw std::runtime_error("Error: No such host\n");

    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    std::memcpy(&serverAddr.sin_addr.s_addr, server->h_addr, server->h_length);
    serverAddr.sin_port = htons(this->serverport);

    if (connect(this->sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
        throw std::runtime_error("Error: Connection failed\n");
}

void bot::authenticate()
{
    sendMessage("PASS " + std::string(this->password));
    usleep(100);

    sendMessage("NICK quizbot");
    usleep(100);

    sendMessage("USER quizbot 0 * :Quiz Bot");

    char buffer[512];
    std::memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = read(this->sockfd, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0)
        std::cout << buffer << std::endl;
}

std::string getmessage(std::string buffer)
{
    
}

void bot::startbot()
{
    connectToServer();
    authenticate();

    char buffer[512];
    std::string message;
    std::string sender;

    while (true)
    {
        int bytesrecieved = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytesrecieved == 0)
            throw std::runtime_error("disconcet from the server\n");
        else if (bytesrecieved < 0)
            throw std::runtime_error("error in reseving data\n");
        buffer[bytesrecieved] = '\0';
        message = getmessage(std::string(buffer));
        sender = getsinder(std::string(buffer));
        sendrespon(messag, sender);
    }
}

bot::~bot()
{
}
