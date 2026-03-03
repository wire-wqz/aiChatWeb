 #include "../../include/handlers/ChatRechargeUpdateHandler.h"

void ChatRechargeUpdateHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
{
    try{
        // 获取会话管理器(连接session)
        auto session=server_->getSessionManager()->getSession(req,resp);

        // 解析请求体
        int totalTokens;
        auto body = req.getBody();
        if (!body.empty()) 
        {
            auto j = json::parse(body);
            if (j.contains("totalTokens")) 
                totalTokens = j["totalTokens"].get<int>();
        }

        std::cout << "接收到的 totalTokens: " << totalTokens << std::endl;

        // 判断登录状态:未认证/登录/临时
        if(session->getValue("isLoggedIn").empty())              // 未认证 重新认证登录逻辑 
        {
            session->setValue("isLoggedIn","false");
        }
        bool is_Login=session->getValue("isLoggedIn")=="true";   // 登录/临时

        if(is_Login)
        {
            int userId = std::stoi(session->getValue("userId"));
            pushTokensToMysql(std::to_string(userId),totalTokens);
        }

        // 封装响应报文
        json successResp;
        successResp["success"] = is_Login;    
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
        std::cerr<<"ChatRechargeUpdateHandler error"<<e.what()<<std::endl;
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


void ChatRechargeUpdateHandler::pushTokensToMysql(std::string userId,int totalTokens)
{
    std::string sql= "UPDATE users SET tokens=? WHERE id = ?";
    // 消息编码
    json sqlJson;
    sqlJson["sql"] = sql;
    sqlJson["totalTokens"] = totalTokens;
    sqlJson["userId"] = userId;
   
    // 转化为string格式
    std::string mes = sqlJson.dump(4);

    // 消息发送
    http::messagequeue::MQPublisher::instance().publish("tokens_sql_queue", mes);
}