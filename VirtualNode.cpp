#include <cstring> 
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


int main(int argc, char* argv[]) {

    char* on_ip = argv[0];
    char* vn_ip = argv[1];
    int vn_port = std::stoi(argv[2]);   

    int tcp_socket = socket(AF_INET,SOCK_STREAM,0);
    
    sockaddr_in on_addr{};
    on_addr.sin_family = AF_INET;
    on_addr.sin_port = htons(5000);
    memset(&(on_addr.sin_zero), '\0',8);

    inet_pton(AF_INET, on_ip,
              &on_addr.sin_addr);

    if( connect(tcp_socket, (struct sockaddr*)&on_addr,
            sizeof(on_addr)) < 0 ){
                std :: cerr << "ERROR : TCP connection failed" << std::endl;
                close(tcp_socket);
            }; 

    const char* msg = "Hello ji";
    send(tcp_socket,msg,strlen(msg),0);
    close(tcp_socket);   
    
    int udp_socket = socket(AF_INET,SOCK_DGRAM,0); 
    sockaddr_in udp_addr{};
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(vn_port);
    memset(&(udp_addr.sin_zero), '\0',8);

    inet_pton(AF_INET, vn_ip,
              &udp_addr.sin_addr);

    if( bind(udp_socket, (struct sockaddr*)&udp_addr, sizeof(udp_addr)) < 0){
        std::cerr << "ERROR : UDP bind failed" << std::endl;
        close(udp_socket);
    };
    return 0;
}
