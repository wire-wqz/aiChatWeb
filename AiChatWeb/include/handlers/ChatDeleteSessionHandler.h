// ChatDeleteSessionHandler.h 
// wire 
// 2025.1.16
/* 
会话删除路由
临时会话删除对应的tempChatInformation 和 tempSessionsIdsMap
登录会话删除 chatInformation sessionsIdsMap 和sql 数据库
*/

#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"
#include <sodium.h>


class ChatDeleteSessionHandler : public http::router::RouterHandler
{
public:
    explicit ChatDeleteSessionHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;
private:
    // 删除数据库数据
    void deleteSql(int userId,const std::string& sessionId); 

private:
    ChatServer* server_;
    http::MysqlUtil mysqlUtil_;
};