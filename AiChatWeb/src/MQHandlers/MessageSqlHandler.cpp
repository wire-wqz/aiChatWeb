#include "../../include/MQHandlers/MessageSqlHandler.h"

void MessageSqlHandler::handle(const std::string& mes)
{
    // 消息解码
    json sqlJson = json::parse(mes);
    std::string sql = sqlJson["sql"];
    int userId = sqlJson["userId"];
    std::string sessionId = sqlJson["sessionId"];
    bool is_user =sqlJson["is_user"];
    std::string content = sqlJson["content"];
    long long ts = sqlJson["ts"];

    // 写入sql
    mysqlUtil_.executeUpdate(sql,userId,sessionId,is_user,content,ts);
    std::cout<<"MQ:SQL写入完成"<<std::endl;
}