// ChatRechargeUpdateHandler.h 
// wire 
// 2025.1.29
/* 
充值成功后，将充值的Tokens数上传到sql中
*/


#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"

class ChatRechargeUpdateHandler : public http::router::RouterHandler
{
public:

    explicit ChatRechargeUpdateHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;

private:
    // 修改id的tokens数
    void pushTokensToMysql(std::string userId,int totalTokens);

private:
    ChatServer* server_;
    http::MysqlUtil     mysqlUtil_;
};