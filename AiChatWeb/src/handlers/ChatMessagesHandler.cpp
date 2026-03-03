#include "../../include/handlers/ChatMessagesHandler.h"

void ChatMessagesHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
{
    try{
        // 获取会话管理器(连接session)
        auto session=server_->getSessionManager()->getSession(req,resp);

        // 解析请求
        std::string sessionId;
        auto body = req.getBody();
        if (!body.empty()) {
            auto j = json::parse(body);
            if (j.contains("sessionId")) sessionId = j["sessionId"];
        }

        // 判断登录状态:未认证/登录/临时
        if(session->getValue("isLoggedIn").empty())              // 未认证 重新认证登录逻辑 
        {
            //登录逻辑todo
            session->setValue("isLoggedIn","false");
        }
        bool is_Login=session->getValue("isLoggedIn")=="true";  // 登录/临时

        // 返回 messages
        std::vector<std::pair<std::string, long long>> messages;
        if(!is_Login)   // 临时账号
        {
            std::string tempId=session->getId();
            // 获取messages
            {
                std::lock_guard<std::mutex> lock(server_->mutexForTempChatInformation);
                auto& userSessions=server_->tempChatInformation[tempId];
                auto AiHelperPtr = userSessions[sessionId];
                messages=AiHelperPtr->GetMessages();
            }   
        }
        else            // 登录账号
        {
            int userId=stoi(session->getValue("userId"));
            // 获取messages
            {
                std::lock_guard<std::mutex> lock(server_->mutexForChatInformation);
                auto& userSessions=server_->chatInformation[userId];
                auto AiHelperPtr = userSessions[sessionId];
                messages=AiHelperPtr->GetMessages();
            }  
        }
       
        // 封装响应报文
        json successResp;
        successResp["success"] = true;
        successResp["messages"] = json::array();
        for(size_t i = 0; i < messages.size(); ++i)
        {
            json msgJson; 
            msgJson["is_user"] = (i % 2 == 0);
            msgJson["content"] = messages[i].first;
            successResp["messages"].push_back(msgJson);
        }
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