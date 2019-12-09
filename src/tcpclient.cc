#include "tcpclient.h"
#include <thread>


#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<stdlib.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<sys/time.h>
#include<arpa/inet.h>
#include<netinet/tcp.h>

namespace hhit
{
    TcpClient::TcpClient(): is_connected(true){
        signal(SIGPIPE,signal_func);

         std::thread t_connect([this]() {
             while(true) {
                 if(!this->is_connected) { // 程序第一次启动不重连
                    printf("reconnect \n");
                    this->ReConnect();
                    sleep(1);
                 }
            }
        });
        t_connect.detach();
    }
    bool TcpClient::Connect(string ip, int port) {
        ip_ = ip;
        port_ = port;
        if( (sockfd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
            return false;
        }

         struct timeval timeout={3,0};//3s
        int ret=setsockopt(sockfd_,SOL_SOCKET,SO_SNDTIMEO,(const char*)&timeout,sizeof(timeout));
        ret=setsockopt(sockfd_,SOL_SOCKET,SO_RCVTIMEO,(const char*)&timeout,sizeof(timeout));


        memset(&servaddr_, 0, sizeof(servaddr_));
        servaddr_.sin_family = AF_INET;
        servaddr_.sin_port = htons(port);
        if( inet_pton(AF_INET, ip.c_str(), &servaddr_.sin_addr) <= 0){
            printf("inet_pton error for %s\n",ip.c_str());
            return false;
        }

        if( connect(sockfd_, (struct sockaddr*)&servaddr_, sizeof(servaddr_)) < 0){
            printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
            is_connected = false;
            return false;
        }else {
            is_connected = true;
            printf("connect success \n");
        }
        return true;
    }

    int TcpClient::Send(char* buff, size_t size) {
        int ret = send(sockfd_, buff, size, 0);
         if(ret==-1 && errno==EAGAIN)
        {
            printf("timeout\n");
        }else if( ret < 0){
            printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
            // is_connected = false;
        }
        return ret;
    }

    int TcpClient::Receive(char* buff){
        int ret = recv(sockfd_, buff, MAX_SIZE, 0);
        if(ret==-1 && errno==EAGAIN)
        {
            printf("timeout\n");
        }else if( ret < 0){
            printf("recv msg error: %s(errno: %d)\n", strerror(errno), errno);
            // is_connected = false;
        }
        return ret;
    }

    void TcpClient::ReConnect() {
        close(sockfd_);
        Connect(ip_, port_);
    }

    void TcpClient::registerReconnect(std::function<void()> func) {
        reconnect_callback_ = func;
    }
} 



