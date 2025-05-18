#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#define IRC_PORT 6667

int main() {
    // Create a socket
    int ServerSocketfd = socket(PF_INET, SOCK_STREAM, 0);
    if (ServerSocketfd == -1) {
        std::cerr << "Failed to create socket\n";
        return 1;
    }

    // Define server address
    sockaddr_in SAddress;
    std::memset(&SAddress, 0, sizeof(SAddress));
    SAddress.sin_family = AF_INET;
    SAddress.sin_port = htons(IRC_PORT);
    SAddress.sin_addr.s_addr = INADDR_ANY;

    // Bind
    if (bind(ServerSocketfd, (struct sockaddr*)&SAddress, sizeof(SAddress)) < 0) {
        std::cerr << "Bind failed\n";
        return 1;
    }

    // Listen
    if (listen(ServerSocketfd, 5) < 0) {
        std::cerr << "Listen failed\n";
        return 1;
    }

    std::cout << "Server is listening on port " << IRC_PORT << "...\n";

    // Accept
    while (1){
        socklen_t addr_size = sizeof(SAddress);
        int clientfd = accept(ServerSocketfd, (struct sockaddr*)&SAddress, &addr_size);
        if (clientfd < 0) {
            std::cerr << "Accept failed\n";
            return 1;
        }

        std::cout << "Client connected!\n";

        // Optional: send welcome message
        const char* msg = "Welcome to IRC Server!\n";
        send(clientfd, msg, strlen(msg), 0);

        // close(clientfd);
        // close(ServerSocketfd);
    }
    return 0;
}
