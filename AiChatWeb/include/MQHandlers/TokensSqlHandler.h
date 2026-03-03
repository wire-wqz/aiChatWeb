// TokensSqlHandler.h 
// wire 
// 2025.1.28
/* 
消息队列中的消费者消息处理类，父类为虚拟类
处理用户Tokens的sql修改
*/

#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/messagequeue/MQConsumerHandler.h"


class TokensSqlHandler:public http::messagequeue::MQConsumerHandler
{
public:
    // 构造函数，用于传入参数
    explicit TokensSqlHandler(ChatServer* server) : server_(server) {}
    void handle(const std::string& mes)  override;
private:
    ChatServer*         server_;
    http::MysqlUtil     mysqlUtil_;
};