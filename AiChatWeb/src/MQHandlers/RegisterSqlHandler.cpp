#include "../../include/MQHandlers/RegisterSqlHandler.h"

void RegisterSqlHandler::handle(const std::string& mes)
{
    // 消息解码
    json jsonSql = json::parse(mes);
    std::string sql =  jsonSql["sql"];
    std::string username = jsonSql["username"];
    std::string hashed_password = jsonSql["hashed_password"];
    int totalTokens = jsonSql["totalTokens"];
    // 写入sql
    mysqlUtil_.executeUpdate(sql,username,hashed_password,totalTokens);
    std::cout<<"MQ:SQL写入完成"<<std::endl;
}