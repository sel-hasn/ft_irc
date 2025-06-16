#include "bot.hpp"

bool setport(std::string str, int &port)
{
    std::istringstream stream(str);
    
    stream >> port;
    return (stream.eof() && !stream.fail());
}

int main(int arc, char **arv)
{
    int port;

    if (arc != 4 || !setport(arv[2], port))
    {
        std::cerr << "Usage: ./bot <hostname> <port> <password>"<<std::endl;
        return 1;
    }
    try {
        bot bot(arv[1], port, arv[3]);
        bot.startbot();
    }
    catch (std::exception &exp){
        std::cerr<<exp.what()<<std::endl;
    }    
}