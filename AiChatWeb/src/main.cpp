#include <string>
#include <iostream>
#include <thread>
#include <chrono>

#include"../include/ChatServer.h"


int main() 
{
    // http 服务器开启
    std::string serverName = "ChatServer";
    int port = 80;
    int threadNum_http = 2;

    ChatServer server(port, serverName, threadNum_http);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    server.start();
}  