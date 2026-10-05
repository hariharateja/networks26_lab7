#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

// using namespace std;

int main() {

    int sockfd;
    char buffer[1024];

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // Server address
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(3000);

    memset(&(server_addr.sin_zero), '\0',8);

    inet_pton(AF_INET, "10.183.118.42",
              &server_addr.sin_addr);

    // Connect
    connect(sockfd,
            (struct sockaddr*)&server_addr,
            sizeof(server_addr));

    // Send
    char msg[] = "EDITH";

    send(sockfd, msg, strlen(msg), 0);

    // Receive
    int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);

    buffer[n] = '\0';

    std::cout << buffer << std::endl;

    // Close
    close(sockfd);

    return 0;
}