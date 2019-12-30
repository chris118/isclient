/*
 * @Description:
 * @Version: 2.0
 * @Autor: xudongxu
 * @Date: 2019-11-18 00:08:35
 * @LastEditors: xudongxu
 * @LastEditTime: 2019-11-21 13:15:07
 */
#ifndef TCP_CLIENT_H
#define TCP_CLIENT_H

#include <arpa/inet.h>
#include <errno.h>
#include <functional>
#include <iostream>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <functional>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <thread>
#include <unistd.h>
#include <logging.h>
#include "timer.hpp"

using namespace std;

#define MAX_SIZE 1024
#define RECONNECT_INTERVAL 2000 // 2秒

namespace hhi1
{

enum NetStatus
{
    CONNECTED,
    DISCONNECTED,
    CONNECTING
};

class TcpClient
{
public:
    TcpClient(std::function<void(NetStatus)> reconnect_callback);
    bool Connect(string ip, int port);
    int Send(char *buff, size_t size);
    int Receive(char *buff);

private:
    void StartReconnectTimer();
    void StopReconnectTimer();

private:
    string ip_;
    size_t port_;
    int sockfd_;
    struct sockaddr_in servaddr_;
    NetStatus status_;

    Timer reconnect_timer;
    std::function<void(NetStatus)> on_status_;
};

TcpClient::TcpClient(std::function<void(NetStatus)> on_status) : status_(CONNECTING),
                                                                 on_status_(on_status)
{

    signal(SIGPIPE, SIG_IGN);

    StartReconnectTimer();
}

bool TcpClient::Connect(string ip, int port)
{
    bool ret = false;

    ip_ = ip;
    port_ = port;
    if ((sockfd_ = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        LOG(INFO) << "create socket error: " << strerror(errno);
        return false;
    }

    int flag = 1;
    setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &flag, sizeof(flag));

    int nSndBufferLen = 128;
    int nLen = sizeof(int);
    setsockopt(sockfd_, SOL_SOCKET, SO_SNDBUF, (char *)&nSndBufferLen, nLen);
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVBUF, (char *)&nSndBufferLen, nLen);

    struct timeval timeout = {10, 0}; // 3s
    setsockopt(sockfd_, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
    setsockopt(sockfd_, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

    memset(&servaddr_, 0, sizeof(servaddr_));
    servaddr_.sin_family = AF_INET;
    servaddr_.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &servaddr_.sin_addr) <= 0)
    {
        LOG(INFO) << "inet_pton error for " << ip;
        return false;
    }

    status_ = CONNECTING;

    if (connect(sockfd_, (struct sockaddr *)&servaddr_, sizeof(servaddr_)) < 0)
    {
        LOG(INFO) << "connect error:" << strerror(errno);
        status_ = DISCONNECTED;
        ret =  false;
    }
    else
    {
        LOG(INFO) << "connect success";
        status_ = CONNECTED;
        ret =  true;
    }

    on_status_(status_);

    return ret;
}

int TcpClient::Send(char *buff, size_t size)
{
    int ret = send(sockfd_, buff, size, 0);
    if (ret == -1 && errno == EAGAIN)
    {
        LOG(INFO) << "send timeout";
        status_ = DISCONNECTED;
    }
    else if (ret < 0)
    {
        LOG(INFO) << "send msg error:" << strerror(errno);
        status_ = DISCONNECTED;
    }
    return ret;
}

int TcpClient::Receive(char *buff)
{
    int ret = recv(sockfd_, buff, MAX_SIZE, 0);
    if (ret == -1 && errno == EAGAIN)
    {
        LOG(INFO) << "receive timeout";
        status_ = DISCONNECTED;
    }
    else if (ret <= 0 && errno != EINTR)
    {
        printf("recv msg error: %s(errno: %d)\n", strerror(errno), errno);
        status_ = DISCONNECTED;
    }
    return ret;
}

void TcpClient::StartReconnectTimer()
{
    LOG( INFO ) << "main thread id: " << std::this_thread::get_id();
    reconnect_timer.setInterval([&]() {
        LOG(INFO) << "fd_: " << sockfd_ << " status: " << status_
                << " thread_id: " << std::this_thread::get_id();
        if (status_ == DISCONNECTED)
        {
            Connect(ip_, port_);
        }
    },RECONNECT_INTERVAL);
}

void TcpClient::StopReconnectTimer()
{
    LOG(INFO) << "StopReconnectTimer";
    reconnect_timer.stop();
}

} // namespace hhi1

#endif
