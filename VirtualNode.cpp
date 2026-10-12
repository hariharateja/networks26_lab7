#include <cstring> 
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


int main(){
    string ON_ipaddr;
    int ON_port;
    std:: cin >> ON_ipaddr >> ON_port;

    int clientSocket = socket(AF_INET,SOCK_STREAM,0);
    
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    memset(&(serverAddress.sin_zero), '\0',8);

    inet_pton(AF_INET, "10.183.118.42",
              &serverAddress.sin_addr);

    if( connect(clientSocket, (struct sockaddr*)&serverAddress,
            sizeof(serverAddress)) < 0 ){
                std :: cerr << "ERROR : connecting";
                close(clientSocket);
            }; 

    close(clientSocket);   
    
    return 0;
}
