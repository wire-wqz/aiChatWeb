// ChatLoginHandler.h 
// wire 
// 2025.1.12
/* 
post 请求 验证登录信息
*/


#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"
#include <sodium.h>


class ChatLoginHandler : public http::router::RouterHandler
{
public:
    explicit ChatLoginHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;
private:
    // 基于用户名查询mysqlId
    int queryUserId(const std::string& username, const std::string& password);
    
    // // 基于id查询Tokens
    // int queryTokens(int& id);


private:
    ChatServer* server_;
    http::MysqlUtil     mysqlUtil_;
};