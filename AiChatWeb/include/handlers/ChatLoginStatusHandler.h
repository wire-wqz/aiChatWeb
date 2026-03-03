// ChatLoginStatusHandler.h 
// wire 
// 2025.1.12
/* 
get 请求 返回登录信息
*/


#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"


class ChatLoginStatusHandler : public http::router::RouterHandler
{
public:
    explicit ChatLoginStatusHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;
private:
    // 基于id查询Tokens
    int queryTokens(int& id);

private:
    ChatServer* server_;
    http::MysqlUtil     mysqlUtil_;
};