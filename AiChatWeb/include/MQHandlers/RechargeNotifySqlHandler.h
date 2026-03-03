// RechargeNotifySqlHandler.h 
// wire 
// 2025.2.6
/* 
消息队列中的消费者消息处理类，父类为虚拟类
支付异步通知，回调函数
*/

#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/messagequeue/MQConsumerHandler.h"

class RechargeNotifySqlHandler:public http::messagequeue::MQConsumerHandler
{
public:
    // 构造函数，用于传入参数
    explicit RechargeNotifySqlHandler(ChatServer* server) : server_(server) {}
    void handle(const std::string& mes)  override;
private:
    ChatServer*         server_;
    http::MysqlUtil     mysqlUtil_;
};