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
    usleep(10000);

    sendMessage("NICK bot");
    usleep(10000);

    sendMessage("USER quizbot 0 * :Quiz Bot");
}

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";

    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, last - first + 1);
}

std::string getMessage(std::string buffer) {
    size_t colon = buffer.find(" :");
    if (colon != std::string::npos) {
        std::string rawMsg = buffer.substr(colon + 2);
        return trim(rawMsg);
    }
    return "";
}

std::string getSender(std::string buffer) {
    if (buffer[0] == ':'){
        size_t Pos = buffer.find(" ");
        if (Pos != std::string::npos) {
            std::string rawMsg = buffer.substr(0, Pos);
            return trim(rawMsg);
        }
    }
    size_t spacePos = buffer.find(" :");
    if (spacePos != std::string::npos) {
        std::string rawMsg = buffer.substr(0, spacePos);
        return trim(rawMsg);
    }
    return "";
}

player *bot::getplayer(std::string sender)
{
    for (size_t i = 0; i < players.size(); i++)
    {
        if (sender == players[i].Nick)
            return &players[i];
    }
    return NULL;
}

void bot::sendQuizToPlayer(player &p) {
    if (quizDatabase.empty()) {
        sendMessage("PRIVMSG " + p.Nick + " : Error: Quiz database is empty. Please contact the administrator.");
        return;
    }

    if (p.quizIndix >= quizDatabase.size())
        p.quizIndix = 0;

    Quiz &q = quizDatabase[p.quizIndix];

    std::string questionMsg = "PRIVMSG " + p.Nick + " : " + q.question;
    sendMessage(questionMsg);
    usleep(100000);

    for (size_t i = 0; i < 3; ++i) {
        std::string choiceMsg = "PRIVMSG " + p.Nick + " : " + q.choices[i];
        sendMessage(choiceMsg);
        usleep(100000);
    }
}

void bot::checkanswer(player &p, const std::string &message) {
    if (p.quizIndix >= quizDatabase.size()) {
        sendMessage("PRIVMSG " + p.Nick + " : You have completed the quiz!");
        usleep(100000);
        sendMessage("PRIVMSG " + p.Nick + " : No more questions. Thanks for playing!");
        return;
    }

    const Quiz &q = quizDatabase[p.quizIndix];
    if (message[0] == q.answer) {
        sendMessage("PRIVMSG " + p.Nick + " : Correct! 🎉");
        p.quizIndix++;
    } else {
        sendMessage("PRIVMSG " + p.Nick + " : Wrong answer. 😢, The correct one is " + q.answer);
        p.quizIndix++;
    }
    usleep(10000);

    if (p.quizIndix < quizDatabase.size()){
        sendQuizToPlayer(p);
        return ;
    }
    sendMessage("PRIVMSG " + p.Nick + " : You have completed the quiz!");
}

void bot::playGame(std::string &sender, std::string &message)
{
    player *p = getplayer(sender);

    if (p == NULL)
    {
        std::cout << "New player joined: " << sender << "\n";

        player newPlayer;
        newPlayer.quizIndix = 0;
        newPlayer.Nick = sender;
        players.push_back(newPlayer);

        p = &players.back();
    }

    if (message == "a" || message == "b" || message == "c")
        checkanswer(*p, message);
    else if (p->quizIndix >= quizDatabase.size()){
        sendMessage("PRIVMSG " + p->Nick + " : You have completed the quiz!");
        usleep(100000);
        sendMessage("PRIVMSG " + p->Nick + " : No more questions. Thanks for playing!");
    }
    else if (message == "game" || message == "GAME")
        sendQuizToPlayer(*p);
    else {
        sendMessage("PRIVMSG " + p->Nick + " : Wrong answer. You have to shose a,b or c!");
        usleep(10000);
        sendQuizToPlayer(*p);
    }
}

void bot::sendrespon(std::string &message, std::string &sender)
{
    if (sender == ":IRCServer")
        return ;
    if (message == "game" || message == "GAME" || getplayer(sender) != NULL)
        playGame(sender, message);
    else {
        std::string respond;
        respond = "PRIVMSG " + sender + " :Hello! 🎮 If you'd like to play the quiz game, just send 'game' or 'GAME' to get started!";
        sendMessage(respond);
    }
}

void bot::initQuizDatabase() {
    std::ifstream file("bonus/quiz_data.txt");
    if (!file.is_open()) {
        std::cerr << "Failed to open quiz_data.txt" << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string segment;
        Quiz q;

        std::getline(ss, q.question, '|');
        for (int i = 0; i < 3; ++i)
            std::getline(ss, q.choices[i], '|');
        std::string answer;
        std::getline(ss, answer, '|');
        if (!answer.empty())
            q.answer = answer[0];

        quizDatabase.push_back(q);
    }

    file.close();
}

void bot::startbot()
{
    connectToServer();
    authenticate();

    char buffer[512];
    std::string message;
    std::string sender;

    initQuizDatabase();

    while (true)
    {
        int bytesrecieved = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
        if (bytesrecieved == 0){
            close(sockfd);
            throw std::runtime_error("disconcet from the server\n");
        }
        else if (bytesrecieved < 0){
            close(sockfd);
            throw std::runtime_error("error in reseving data\n");
        }

        buffer[bytesrecieved] = '\0';
        // std::cout<<buffer<<std::endl;
        sender = getSender(buffer);
        message = getMessage(buffer);
        std::cout<<sender<<" : "<<message<<std::endl;

        sendrespon(message, sender);
    }
}

bot::~bot()
{
}
