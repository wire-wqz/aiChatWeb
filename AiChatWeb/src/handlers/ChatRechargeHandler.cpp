#include "../../include/handlers/ChatRechargeHandler.h"

void ChatRechargeHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
{
    try{
        // 获取会话管理器(连接session)
        auto session=server_->getSessionManager()->getSession(req,resp);

        // 解析请求体
        double price;
        auto body = req.getBody();
        if (!body.empty()) 
        {
            auto j = json::parse(body);
            if (j.contains("price")) 
                price = j["price"].get<int>();
        }

        std::cout << "接收到的price " << price << std::endl;

        // 判断登录状态:未认证/登录/临时
        if(session->getValue("isLoggedIn").empty())              // 未认证 重新认证登录逻辑 
        {
            session->setValue("isLoggedIn","false");
        }
        bool is_Login=session->getValue("isLoggedIn")=="true";   // 登录/临时

        std::string htmlUrl;
        if(is_Login)
        {
            // 获取时间戳
            
            // 订单号
            string outTradeNo = paystrategy_->generateSimpleOrderNo();

            // 传入Api参数
            string subject = "支付宝:Tokens充值";
            auto contentMap = paystrategy_->getPagePayContent(outTradeNo, price, subject);

            // 传入notify_url和return_url
            std::ifstream file("/root/AiHttpServer/AiChatWeb/resource/AiUtils/PayTools/AiChatUrl.json");
            if (!file.is_open()) {
                throw std::runtime_error("Failed to open AiChatUrl file");
            }
            json j;
            file >> j;
            string notify_url = j["notify_url"];
            string return_url = j["return_url"];
            auto extendParamMap = paystrategy_->getextendParamMap(notify_url,return_url);

            // 获得支付宝HTMLurl
            htmlUrl = paystrategy_->getHtmlUrl(contentMap,extendParamMap);
            cout<<"支付HTML_URL:"<<htmlUrl<<endl;

            // 将订单信息写入sql
            rechargeOrder order;
            order.out_trade_no = outTradeNo;
            order.user_id = session->getValue("userId");
            order.price = price;
            order.status = "pending";
            order.subject = subject;
            order.create_time = getCurrentLocalTimeString();
            pushRechargeToMysql(order); 
        }

        // 封装响应报文
        json successResp;
        successResp["success"] = is_Login;    
        successResp["htmlUrl"] = htmlUrl; 
        std::string successBody = successResp.dump(4);
        resp.setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
        resp.setCloseConnection(false);
        resp.setContentType("application/json");
        resp.setContentLength(successBody.size());
        resp.setBody(successBody);
        return;

    }
    catch (const std::exception& e)
    {
        std::cerr<<"ChatRechargeHandler error"<<e.what()<<std::endl;
        json failureResp;
        failureResp["success"] = false;
        std::string failureBody = failureResp.dump(4);
        resp.setStatusLine(req.getVersion(), http::HttpResponse::k400BadRequest, "Bad Request");
        resp.setCloseConnection(true);
        resp.setContentType("application/json");
        resp.setContentLength(failureBody.size());
        resp.setBody(failureBody);
    }
}

void ChatRechargeHandler::setPayStrategy(std::string name)
{
    paystrategy_ = PayStrategyFactory::instance().create(name);
}

std::string ChatRechargeHandler::getCurrentLocalTimeString() 
{
    auto now = std::time(nullptr);
    std::tm* tm = std::localtime(&now); // 使用本地时区（若需 UTC，改用 gmtime）
    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void ChatRechargeHandler::pushRechargeToMysql(rechargeOrder& order)
{
    std::string  sql = 
        "INSERT INTO recharge_orders ("
        "out_trade_no, user_id, price, status, subject, create_time"
        ") VALUES (?, ?, ?, ?, ?, ?)";

    // 消息编码
    json sqlJson;
    sqlJson["sql"] = sql;
    sqlJson["out_trade_no"]     =   order.out_trade_no;
    sqlJson["user_id"]          =   order.user_id;
    sqlJson["price"]            =   order.price;
    sqlJson["status"]           =   order.status;
    sqlJson["subject"]          =   order.subject;
    sqlJson["create_time"]      =   order.create_time;
   
    // 转化为string格式
    std::string mes = sqlJson.dump(4);

    // 消息发送
    http::messagequeue::MQPublisher::instance().publish("recharge_sql_queue", mes);
}