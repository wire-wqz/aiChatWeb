#include "../../include/handlers/ChatDeleteSessionHandler.h"

void ChatDeleteSessionHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
{
    try{
        
        // 获取会话管理器(连接session)
        auto session=server_->getSessionManager()->getSession(req,resp);

        // 解析请求体
        std::string sessionId;
        auto body = req.getBody();
        if (!body.empty()) 
        {
            auto j = json::parse(body);
            if (j.contains("sessionId")) 
                sessionId = j["sessionId"];
        }

        // 判断登录状态:未认证/登录/临时
        if(session->getValue("isLoggedIn").empty())              // 未认证 重新认证登录逻辑 
        {
            session->setValue("isLoggedIn","false");
        }
        bool is_Login=session->getValue("isLoggedIn")=="true";   // 登录/临时

        
        // 删除聊天会话
        if(!is_Login){   // 临时会话
            std::string tempId=session->getId();

            std::lock_guard<std::mutex> lock(server_->mutexForTempChatInformation);
            
            // 从unordered_map中删除
            auto& chatMap = server_->tempChatInformation[tempId];
            if (chatMap.find(sessionId) != chatMap.end()) {
                chatMap.erase(sessionId);
            }

            // 从vector中删除
            auto& sessionsVector = server_->tempSessionsIdsMap[tempId];
            auto it = std::find(sessionsVector.begin(), sessionsVector.end(), sessionId);
            if (it != sessionsVector.end()) {
                 sessionsVector.erase(it);
            }
        
        }
        else
        {
            int userId=stoi(session->getValue("userId"));

            std::lock_guard<std::mutex> lock(server_->mutexForChatInformation);
            
            // 从unordered_map中删除
            auto& chatMap = server_->chatInformation[userId];
            if (chatMap.find(sessionId) != chatMap.end()) {
                chatMap.erase(sessionId);
            }

            // 从vector中删除
            auto& sessionsVector = server_->sessionsIdsMap[userId];
            auto it = std::find(sessionsVector.begin(), sessionsVector.end(), sessionId);
            if (it != sessionsVector.end()) {
                 sessionsVector.erase(it);
            }

            // 从sql中删除对应的sessionId
            deleteSql(userId,sessionId);
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

void ChatDeleteSessionHandler::deleteSql(int userId,const std::string& sessionId){

    std:: string sql = "DELETE FROM chat_message WHERE id = ? and session_id = ?";
    mysqlUtil_.executeUpdate(sql,userId,sessionId);
}