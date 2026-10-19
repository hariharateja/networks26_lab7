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

struct Tuple {
    char id;
    uint32_t ip;
    uint16_t port;
    int32_t cost;
};

//lsp 1,3
struct LSP_msg{
    char OriginID;
    uint32_t seq_num;
    int32_t costs[26];
};

int graph[26][26];
uint32_t my_seq_num = 0;
uint32_t highest_seq_num[26];
int max_nodes =0;

//our node details
char my_id = '?';
int my_costs[26];
bool is_neighbour[26];
sockaddr_in neighbour_addrs[26];





//helper to send lsp to all neighbours
void send_lsp(int udp_socket){
    if(my_id == '?'){
        std::cerr << "ERROR : my_id is not set" << std::endl;
        return;
    }

    my_seq_num++;
    highest_seq_num[my_id - 'A'] = my_seq_num;

    LSP_msg lsp;
    lsp.OriginID = my_id;
    lsp.seq_num = htonl(my_seq_num);
    for(int i=0; i<26; i++){
        lsp.costs[i] = htonl(my_costs[i]);
    }

    for(int i=0;i<26;i++){
        if(is_neighbour[i]){
            sendto(udp_socket,&lsp,sizeof(lsp),0,(struct sockaddr*)&neighbour_addrs[i],sizeof(neighbour_addrs[i]));
        }
    }
}
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
    
    // send(tcp_socket,)
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
            //2d
            uint32_t net_count =0;
            if(recv(tcp_socket, &net_count, sizeof(net_count),MSG_WAITALL) < 0){
                std::cerr << "error : didn't receive bytes " << std::endl;
                break;
            }

            int count = ntohl(net_count);

            Tuple tuples[26];
            if(recv(tcp_socket , tuples , count*sizeof(Tuple),MSG_WAITALL) < 0){
                std::cerr << "error : didn't receive bytes " << std::endl;
                break;
            }

            memset(my_costs,-1,sizeof(my_costs));
            memset(is_neighbour,false,sizeof(is_neighbour));

            for(int i=0; i < count; i++){
                char node_alphabet = tuples[i].id;
                int cost = ntohl(tuples[i].cost);
                
                
                if(cost == 0){
                    my_id = node_alphabet;
                    my_costs[node_alphabet - 'A'] = 0;
                }else{
                    my_costs[node_alphabet - 'A'] = cost;
                    is_neighbour[node_alphabet - 'A'] = true;
                    
                    neighbour_addrs[node_alphabet - 'A'].sin_family = AF_INET;
                    neighbour_addrs[node_alphabet - 'A'].sin_addr.s_addr = tuples[i].ip;
                    neighbour_addrs[node_alphabet - 'A'].sin_port = tuples[i].port;   
                }
            }
            for(int i =0;i<26;i++){
                graph[i][node_alphabet - 'A'] = my_costs[i];
                graph[node_alphabet - 'A'][i] = my_costs[i];
            }
            std:: cout << my_id << " " << "0";
            for(int i=0; i<26; i++){
                if(is_neighbour[i]){
                    std::cout << " " << (char)(i+'A') << "=" << my_costs[i];
                }
            }
            std::cout << std::endl;
            //lsp send
            send_lsp(udp_socket);
        }

        //UDP socket
        if(FD_ISSET(udp_socket,&read_fds)){
            std::cout << "UDP message received" << std::endl;
            LSP_msg lsp;
            sockaddr_in sender_addr{};
            socklen_t len = sizeof(len);
            //lsp receive
            int n = recvfrom(udp_socket,&lsp,sizeof(lsp),0,(struct sockaddr*)&sender_addr,&len) ;
            
            if(n == sizeof(LSP_msg)){
                char origin_id = lsp.OriginID;
                uint32_t seq_num = ntohl(lsp.seq_num);

                if(origin_id >= 'A' && origin_id <= 'Z' && seq_num > highest_seq_num[origin_id - 'A']){
                    highest_seq_num[origin_id - 'A'] = seq_num;
                    if(origin_id -'A' + 1  > max_nodes){
                        max_nodes = origin_id - 'A' + 1;
                    }

                    for(int i=0; i<26; i++){
                        int c  = ntohl(lsp.costs[i]);
                        graph[origin_id - 'A'][i] = ntohl(c);
                        graph[i][origin_id - 'A'] = ntohl(c);
                        if(c != -1 && i+1 > max_nodes){
                            max_nodes = i+1;
                        }
                    }
                    for(int i=0;i<26;i++){
                        if(is_neighbour[i]){
                            //do not send back to the sender
                            if(neighbour_addrs[i].sin_addr.s_addr == sender_addr.sin_addr.s_addr && neighbour_addrs[i].sin_port == sender_addr.sin_port){
                                continue;
                            }
                            sendto(udp_socket,&lsp,sizeof(lsp),0,(struct sockaddr*)&neighbour_addrs[i],sizeof(neighbour_addrs[i]));
                        }
                    }
                    
                }

            }

        }

        usleep(500000); //500ms
    }
    return 0;
}
