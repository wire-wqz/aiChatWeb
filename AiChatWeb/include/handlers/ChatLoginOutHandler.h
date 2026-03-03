// ChatLoginOutHandler.h 
// wire 
// 2025.1.13
/* 
post 请求 退出本次会话的登录信息
*/


#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"


class ChatLoginOutHandler : public http::router::RouterHandler
{
public:
    explicit ChatLoginOutHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;
private:

private:
    ChatServer* server_;
};