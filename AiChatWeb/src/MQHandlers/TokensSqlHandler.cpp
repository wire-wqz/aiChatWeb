



#include "../../include/MQHandlers/TokensSqlHandler.h"

void TokensSqlHandler::handle(const std::string& mes)
{
    // 消息解码
    json jsonSql = json::parse(mes);
    std::string sql =  jsonSql["sql"];
    int totalTokens = jsonSql["totalTokens"];
    std::string userId = jsonSql["userId"];

    // 写入sql
    mysqlUtil_.executeUpdate(sql,totalTokens,userId);
    std::cout<<"MQ:SQL写入完成"<<std::endl;
}
