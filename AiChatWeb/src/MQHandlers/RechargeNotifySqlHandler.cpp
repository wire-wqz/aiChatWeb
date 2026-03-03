#include "../../include/MQHandlers/RechargeNotifySqlHandler.h"

void RechargeNotifySqlHandler::handle(const std::string& mes)
{
     auto dbConn = mysqlUtil_.getDbConn();


    try{
        // 解析
        json j = json::parse(mes);
        std::string out_trade_no = j["out_trade_no"];
        std::string payTime      = j["payTime"];
        double price             = j["price"];
        int tokens               = j["tokens"];

        // // // 启动事务,非自动提交
        dbConn->setAutoCommit(0);
        
    

        // 从recharge_orders查询userID和price
        string sql1 = "SELECT user_id,price FROM recharge_orders where out_trade_no = ?";
        auto res1 = dbConn->executeQuery(sql1,out_trade_no);
        string user_id;
        double priceSql;
        if(res1->next())
        {
            user_id=res1->getString("user_id");
            priceSql = res1->getDouble("price");
        }

        // 修改recharge_orders的status和pay_time
        string sql2 = "UPDATE recharge_orders SET status='paid',pay_time=? where out_trade_no = ?";
        if(!dbConn->executeUpdate(sql2,payTime,out_trade_no))
            throw ("RechargeNotifySqlHandler: Failed to execute SQL update.");
        

        // 验证金额是否匹配
        if(priceSql!=price)
            throw ("RechargeNotifySqlHandler: 金额不匹配");


        // 查询原有tokens
        std::string sql3= "SELECT tokens FROM users WHERE id = ?";
        auto res2 = dbConn->executeQuery(sql3,stoi(user_id));
        long long totalTokens;
        if(res2->next())
        {
            totalTokens = res2->getInt64("tokens");
        }
        

        // 充值tokens
        totalTokens += tokens;
        std::string sql4= "UPDATE users SET tokens=? WHERE id = ?";
        if(!dbConn->executeUpdate(sql4,totalTokens,stoi(user_id)))
            throw ("RechargeNotifySqlHandler: Failed to execute SQL update.");

        std::cout<<"RechargeNotifySqlHandler:SQL写入完成"<<std::endl;

        dbConn->commit();
        dbConn->setAutoCommit(1);



    }
    catch(const std::exception& e)
    {
        std::cerr<<"RechargeNotifySqlHandler error:"<<e.what()<<std::endl;

        dbConn->rollback();
        dbConn->setAutoCommit(1);
    }
}
