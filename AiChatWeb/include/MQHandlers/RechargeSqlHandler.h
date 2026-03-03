// RechargeSqlHandler.h 
// wire 
// 2025.2.5
/* 
消息队列中的消费者消息处理类，父类为虚拟类
填写订单消息
*/

#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/messagequeue/MQConsumerHandler.h"

class RechargeSqlHandler:public http::messagequeue::MQConsumerHandler
{
public:
    // 构造函数，用于传入参数
    explicit RechargeSqlHandler(ChatServer* server) : server_(server) {}
    void handle(const std::string& mes)  override;
private:
    ChatServer*         server_;
    http::MysqlUtil     mysqlUtil_;
};