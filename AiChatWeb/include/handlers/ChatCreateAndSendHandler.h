// ChatCreateAndSendHandler.h 
// wire 
// 2025.12.30
/* 
聊天会话的第一次创建和第一次处理发送的消息
*/


#pragma once
#include "../ChatServer.h"
#include "../AiUtil/AiSessionIdGenerator.h"
#include "../../../HttpServer/include/router/RouterHandler.h"


class ChatCreateAndSendHandler : public http::router::RouterHandler
{
public:
    explicit ChatCreateAndSendHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;
private:
     int queryTokens(int& id);
     
private:
    ChatServer*         server_;
    http::MysqlUtil     mysqlUtil_;
};