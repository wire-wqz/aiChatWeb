



#include "../../include/MQHandlers/RechargeSqlHandler.h"

void RechargeSqlHandler::handle(const std::string& mes)
{
    try{
        // 解析
        json j = json::parse(mes);
        std::string sql          = j["sql"];
        std::string out_trade_no = j["out_trade_no"];
        std::string user_id      = j["user_id"];
        double price             = j["price"];
        std::string status       = j["status"];
        std::string subject      = j["subject"];
        std::string create_time  = j["create_time"];

        // 写入sql
        mysqlUtil_.executeUpdate(sql,out_trade_no,user_id,price,status,subject,create_time);
        std::cout<<"RechargeMQ:SQL写入完成"<<std::endl;
    }
    catch(const std::exception& e)
    {
        std::cerr<<"RechargeSqlHandler error:"<<e.what()<<std::endl;
    }
}
