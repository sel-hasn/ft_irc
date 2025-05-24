
#include "Server.hpp"


int main(int ac, char **av){
    int Port;
    if (ac != 3){
        std::cerr << "the program is working as follow: ./ircserv port passwordn\n";
        return 1;
    }
    try{
        Port = ParsePort(static_cast<std::string>(av[1]));
        PasswordParse(static_cast<std::string>(av[2]));
    } catch (const std::exception& e){
        std::cout << "Program's Error Exception : " << e.what() << std::endl;
        return 1;
    }
    try{
        signal(SIGQUIT, Server::Signals_handler);
        signal(SIGINT, Server::Signals_handler);
        Server srv(Port, static_cast<std::string> (av[2]));
        srv.ServerStarts();
    } 
    catch(const CustomException& e){
        std::cout << "Server's Error Exception : " << e.msg();
        return 1;
    }
    return 0;
}
