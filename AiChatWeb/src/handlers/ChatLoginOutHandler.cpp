#include "../../include/handlers/ChatLoginOutHandler.h"

void ChatLoginOutHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
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

        // 退出登录
        if(is_Login)
        {
            // 取出userId
            int userId = std::stoi(session->getValue("userId"));
            
            // 将会话中的所有值clear
            session->clear();

            // 销毁本会话
           server_->getSessionManager()->destroySession(session->getId());

           // onlineUsers_表下线
           {
                std::lock_guard<std::mutex> lock(server_->mutexForOnlineUsers_);
                server_->onlineUsers_[userId]=false;
           }

           
            
        }


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
    catch (const std::exception& e)
    {
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

