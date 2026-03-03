// ChatSendHandler.h 
// wire 
// 2025.12.30
/* 
在聊天会话已经建立的基础上 处理聊天会话发送的消息
*/


#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"


class ChatSendHandler : public http::router::RouterHandler
{
public:
    explicit ChatSendHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;
private:

private:
    ChatServer*         server_;
    http::MysqlUtil     mysqlUtil_;
};