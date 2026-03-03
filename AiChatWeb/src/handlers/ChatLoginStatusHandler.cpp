#include "../../include/handlers/ChatLoginStatusHandler.h"

void ChatLoginStatusHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
{
    try{
        // 获取会话管理器(连接session)
        auto session=server_->getSessionManager()->getSession(req,resp);

        // 判断登录状态:未认证/登录/临时
        if(session->getValue("isLoggedIn").empty())              // 未认证 重新认证登录逻辑 
        {
            session->setValue("isLoggedIn","false");
        }
        bool is_Login=session->getValue("isLoggedIn")=="true";   // 登录/临时

       
        if(is_Login)
        {
            int userId = std::stoi(session->getValue("userId"));
            // 封装响应报文
            json successResp;
            successResp["success"] = is_Login;
            successResp["userName"] = session->getValue("userName");    
            successResp["totalTokens"] = queryTokens(userId);   
            std::string successBody = successResp.dump(4);
            resp.setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
            resp.setCloseConnection(false);
            resp.setContentType("application/json");
            resp.setContentLength(successBody.size());
            resp.setBody(successBody);
            return;
        }
        else
        {
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

    }
    catch (const std::exception& e)
    {
        json failureResp;
        failureResp["success"] = false;
        failureResp["status"] = "error";
        failureResp["message"] = e.what();
        std::string failureBody = failureResp.dump(4);
        resp.setStatusLine(req.getVersion(), http::HttpResponse::k400BadRequest, "Bad Request");
        resp.setCloseConnection(true);
        resp.setContentType("application/json");
        resp.setContentLength(failureBody.size());
        resp.setBody(failureBody);
    }
}


// 基于id查询Tokens
int ChatLoginStatusHandler::queryTokens(int& userId)
{
    try{
        int tokens = 0;
        std::string sql= "SELECT tokens FROM users WHERE id = ?";
        auto res = mysqlUtil_.executeQuery(sql,userId);
        while (res->next())
        {
            tokens = res->getInt64("tokens");
        }
        return tokens;
    }
    catch(const std::exception& e) {
        std::cerr << "getTokensFromMysql Failed : " << e.what() << std::endl;
        return 0; 
    }
}