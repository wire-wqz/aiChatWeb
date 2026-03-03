// ChatRechargeNotifyHandler.h 
// wire 
// 2025.2.5
/* 
接收支付软件的异步通知，修改金额
*/

#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"
#include "../AiUtil/PayFactory.h"




class ChatRechargeNotifyHandler : public http::router::RouterHandler
{
public:

    explicit ChatRechargeNotifyHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;

private:
    // 查询订单号获得useId,并充值
    void queryIDandPushTokens(std::string out_trade_no, string payTime, double price);

    int priceToTokens(double price);

private:
    ChatServer* server_;
    http::MysqlUtil     mysqlUtil_;
};