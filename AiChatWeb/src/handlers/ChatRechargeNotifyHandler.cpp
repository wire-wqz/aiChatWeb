 #include "../../include/handlers/ChatRechargeNotifyHandler.h"

void ChatRechargeNotifyHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
{
    try{
        auto payStrategy = PayStrategyFactory::instance().create("AliPay");
        auto responseMap = payStrategy->parseNotify(req.getBody());

        string payTime = responseMap["gmt_payment"];
        string out_trade_no = responseMap["out_trade_no"];
        double total_amount = stod(responseMap["total_amount"]);
        
        std::cout <<responseMap["gmt_create"]<<std::endl;

        queryIDandPushTokens(out_trade_no,payTime,total_amount);

        resp.setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
        resp.setCloseConnection(false);
        return;

    }
    catch (const std::exception& e)
    {
        std::cerr<<"ChatRechargeUpdateHandler error"<<e.what()<<std::endl;
        resp.setStatusLine(req.getVersion(), http::HttpResponse::k400BadRequest, "Bad Request");
        resp.setCloseConnection(true);
    }
}

int ChatRechargeNotifyHandler::priceToTokens(double price){
    int priceint = int(price);
    switch (priceint)
    {
    case 6:
        return 150000;
        break;
    case 30:
        return 800000;
        break;
    case 128:
        return 4000000;
        break;
    case 328:
        return 1200000;
        break;
    case 648:
        return 3000000;
        break;
    default:
        return 0;
        break;
    }

}



// 查询订单号获得useId,并充值
void ChatRechargeNotifyHandler::queryIDandPushTokens(std::string out_trade_no, string payTime, double price)
{
    // // 从recharge_orders查询userID和price
    // string sql1 = "SELECT user_id,price FROM recharge_orders where out_trade_no = ?";

    // // 修改recharge_orders的status和pay_time
    // string sql2 = "UPDATE recharge_orders SET status=paid,pay_time=? where out_trade_no = ?";

    // // 验证金额是否匹配并转化为tokens

    // // 向users中充值tokens
    // std::string sql3= "SELECT tokens FROM users WHERE id = ?";
    // std::string sql4= "UPDATE users SET tokens=? WHERE id = ?";
    

   
    // 消息编码
    json sqlJson;
    sqlJson["out_trade_no"] = out_trade_no;
    sqlJson["payTime"] = payTime;
    sqlJson["price"] = price;
    sqlJson["tokens"] = priceToTokens(price);
    
    // 转化为string格式
    std::string mes = sqlJson.dump(4);

    // 消息发送
    http::messagequeue::MQPublisher::instance().publish("rechargeNotify_sql_queue", mes);
}