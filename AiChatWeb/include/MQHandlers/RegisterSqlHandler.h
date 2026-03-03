// RegisterSqlHandler.h 
// wire 
// 2025.1.16
/* 
消息队列中的消费者消息处理类，父类为虚拟类
处理登录信息的sql入库
*/

#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/messagequeue/MQConsumerHandler.h"


class RegisterSqlHandler:public http::messagequeue::MQConsumerHandler
{

public:
    // 构造函数，用于传入参数
    explicit RegisterSqlHandler(ChatServer* server) : server_(server) {}

    void handle(const std::string& mes)  override;
private:
    ChatServer*         server_;
    http::MysqlUtil     mysqlUtil_;
};
