#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

// using namespace std;

int main() {

    int server_fd, client_fd;
    char buffer[1024];

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    // Server address
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3000);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset(&(server_addr.sin_zero), '\0',8);
    // Bind
    bind(server_fd,
         (struct sockaddr*)&server_addr,
         sizeof(server_addr));

    // Listen
    listen(server_fd, 10);


    std::cout << "hey \n";

    // Accept client
    unsigned sin_size = sizeof(struct sockaddr_in);
    sockaddr_in client_addr{};
    client_fd = accept(server_fd, (struct sockaddr*)& client_addr , &sin_size);

    // Receive
    int n = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    buffer[n] = '\0';

    std::cout << buffer << std::endl;

    // Send
    char msg[] = "Even Dead I am The HERO";

    send(client_fd, msg, strlen(msg), 0);

    // Close
    close(client_fd);
    close(server_fd);

    return 0;
}