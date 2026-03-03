// ChatHeartCheckHandler.h 
// wire 
// 2025.1.6
/* 
前端心跳检测,判断会话是否过期
同时，也会刷新会话的过期时间
*/


#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"


class ChatHeartCheckHandler : public http::router::RouterHandler
{
public:
    explicit ChatHeartCheckHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;
private:

private:
    ChatServer*         server_;
};