// ChatRegisterHandler.h 
// wire 
// 2025.1.13
/* 
主页 返回AiChat.html给客户端
*/

#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"
#include <sodium.h>


class ChatRegisterHandler : public http::router::RouterHandler
{
public:
    explicit ChatRegisterHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;
private:
    // 基于用户名查询mysqlId
    bool addUserId(const std::string& username, const std::string& password); 

private:
    ChatServer* server_;
    http::MysqlUtil mysqlUtil_;
};