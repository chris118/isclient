#ifndef TCPCLIENT_H
#define TCPCLIENT_H
#include <iostream>
 #include<stdio.h>
 #include<stdlib.h>
 #include<string.h>
 #include<errno.h>
 #include<sys/types.h>
 #include<sys/socket.h>
 #include<netinet/in.h>
 #include<arpa/inet.h>
 #include<unistd.h>


using namespace std;

#define MAX_SIZE 1024 * 10

namespace hhit
{
class TcpClient
{
    public:
        TcpClient();
        bool Connect(string ip, int port);
        int Send(char* buff, size_t size);
        int Receive(char* buff);
        void ReConnect();
        static void signal_func(int i) {}
        void registerReconnect(std::function<void()> func);
    private:
        string ip_;
        size_t port_;
        int sockfd_;
        struct sockaddr_in  servaddr_;
        bool is_connected;
        bool is_first_connect;
        std::function<void()> reconnect_callback_; 
};
}


#endif