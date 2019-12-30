#include <iostream>
#include <thread>

#include <logging.h>
// #include "tcpclient.h"
#include "tcpclient.hpp"

using namespace std;
// using namespace hhit;
using namespace hhi1;

void printBuffer(char* buff, int size, std::string tag){
    std::cout << tag << std::endl;
        for(int i = 0; i < size; i++){
            // printf("%02x", buff[i]);
            printf("%02x", buff[i]);
    }
    std::cout << std::endl << std::endl;
}

int main(int argc, const char *argv[]) {
    
    // TcpClient client;

    // client.Connect("127.0.0.1", 3000);
    
    // while (true)
    // {
    //     printf("send: hello\n");
    //     string hello = "hello";
    //     int ret = client.Send((char*)hello.c_str(), hello.length());

    //     char buffer[MAX_SIZE];
    //     memset(buffer, 0, MAX_SIZE);
    //     int received_ret = client.Receive(buffer);
    //     printf("received: %s %d\n ", buffer, received_ret);

    //     sleep(1);
    // }

    TcpClient client( [&](NetStatus status) { 
        LOG( INFO ) << "onstatus: " << status << "thread_id: " <<  std::this_thread::get_id();
    });
    bool re = client.connect("127.0.0.1", 2099);
    

    while (true)
    {
        printf("send: hello\n");
        string hello = "hello";
        int ret = client.Send((char*)hello.c_str(), hello.length());

        char buffer[MAX_SIZE];
        memset(buffer, 0, MAX_SIZE);
        int received_ret = client.Receive(buffer);
        printf("received: %s %d\n ", buffer, received_ret);

        sleep(1);
    }
    
}
