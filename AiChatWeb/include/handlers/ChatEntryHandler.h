// ChatHandler.h 
// wire 
// 2025.12.30
/* 
主页 返回AiChat.html给客户端
*/


#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"


class ChatEntryHandler : public http::router::RouterHandler
{
public:
    explicit ChatEntryHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;
private:

private:
    ChatServer* server_;
};