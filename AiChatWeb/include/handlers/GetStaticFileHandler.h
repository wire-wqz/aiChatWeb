// GetStaticFile.h 
// wire 
// 2025.12.30
/* 
给客户端返回静态路由请求
*/


#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"


class GetStaticFileHandler : public http::router::RouterHandler
{
public:
    explicit GetStaticFileHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;
private:

private:
    ChatServer* server_;
};