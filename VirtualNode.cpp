#include <cstring> 
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstdint>
#include <sys/select.h>

struct Connect_msg{
    uint32_t ip;
    uint16_t port;
};
int main(int argc, char* argv[]) {
  
    //1a,b
    char* on_ip = argv[1];
    char* vn_ip = argv[2];
    int vn_port = std::stoi(argv[3]);   

    
     
    //1c
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
        exit(1);
    };
    //2a
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
                exit(1);
            }; 
    //2b
    Connect_msg msg;
    inet_pton(AF_INET, vn_ip, &msg.ip);
    msg.port = htons(vn_port);
    // const char* msg = "Hello ji";
    send(tcp_socket,&msg,sizeof(msg),0);
    
    //2c
    while(true){
        fd_set read_fds;
        FD_ZERO(&read_fds);

        FD_SET(udp_socket, &read_fds);
        FD_SET(tcp_socket,&read_fds);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int activity = select(FD_SETSIZE, &read_fds, NULL, NULL, &tv);

        if(activity < 0){
            std::cerr << "ERROR : select failed" << std::endl;
            close(udp_socket);
            close(tcp_socket);
            exit(1);
        }
        //TCP socket
        if(FD_ISSET(tcp_socket,&read_fds)){
            std::cout << "ON sent a message" << std::endl;
        }

        //UDP socket
        if(FD_ISSET(udp_socket,&read_fds)){
            std::cout << "UDP message received" << std::endl;
        }

        usleep(500000); //500ms
    }
    return 0;
}
