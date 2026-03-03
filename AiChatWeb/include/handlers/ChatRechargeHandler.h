// ChatRechargeHandler.h 
// wire 
// 2025.2.3
/* 
充值逻辑，接入外部充值软件API
*/


#pragma once
#include <ctime>
#include <iomanip>
#include <sstream>
#include "../ChatServer.h"
#include "../AiUtil/PayFactory.h"
#include "../../../HttpServer/include/router/RouterHandler.h"




struct rechargeOrder
{
    string out_trade_no;
    string user_id;
    double price;
    string status;
    string subject;
    string create_time;
};


class ChatRechargeHandler : public http::router::RouterHandler
{
public:

    explicit ChatRechargeHandler(ChatServer* server) : server_(server) {
        // // 工厂注册
        PayStrategyRegister<AliPayStrategy>("AliPay");

        // 默认设置
        setPayStrategy("AliPay");
    }

    // 路由函数
    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;

private:
    // 设置支付策略：默认阿里
    void setPayStrategy(std::string name);
    
    // 订单信息写入sql
    void pushRechargeToMysql(rechargeOrder& order);

    // 获取当前时间
    std::string getCurrentLocalTimeString();

private:
    std::shared_ptr<PayStrategy> paystrategy_;                      // 使用的支付类指针
    ChatServer* server_;
};