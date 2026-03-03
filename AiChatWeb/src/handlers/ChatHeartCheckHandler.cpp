#include "../../include/handlers/ChatHeartCheckHandler.h"

void ChatHeartCheckHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
{
    try{
        // 获取会话管理器(连接session)
        auto session=server_->getSessionManager()->getSession(req,resp);

         // 判断登录状态:未认证/登录/临时
        if(session->getValue("isLoggedIn").empty())              // 未认证 重新认证登录逻辑 
        {
            // 默认临时账户
            session->setValue("isLoggedIn","false");
            json successResp;
            successResp["success"] = true;
            std::string successBody = successResp.dump(4);
            resp.setStatusLine(req.getVersion(), http::HttpResponse::k401Unauthorized, "Unauthorize");
            resp.setCloseConnection(false);
            resp.setContentType("application/json");
            resp.setContentLength(successBody.size());
            resp.setBody(successBody);
            return;
        }
        else{ // 会话没有过期
            // 封装响应报文
            json successResp;
            successResp["success"] = true;
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