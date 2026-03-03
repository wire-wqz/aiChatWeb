#include "../../include/handlers/ChatSessionsHandler.h"

void ChatSessionsHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
{
    try{
        // 获取会话管理器(连接session)
        auto session=server_->getSessionManager()->getSession(req,resp);

         // 判断登录状态:未认证/登录/临时
        if(session->getValue("isLoggedIn").empty())              // 未认证 重新认证登录逻辑 
        {
            //登录逻辑todo
            session->setValue("isLoggedIn","false");
        }
        bool is_Login=session->getValue("isLoggedIn")=="true";   // 登录/临时

        // 返回sessionsId和sessionsName
        std::vector<std::string> sessionsId;
        std::vector<std::string> sessionsName;
        if(!is_Login)         // 临时账户(临时账户依托SessionManager,数据仅存储在内存中,不存储到sql)
        {
            std::string tempId=session->getId(); 
            {
                std::lock_guard<std::mutex> lock(server_->mutexForTempChatInformation);
                // 获取sessionsId
                sessionsId=server_->tempSessionsIdsMap[tempId];
                // 获取sessionsName(可以直接将name存储,这里从aihelp中查询)
                auto& userSessions=server_->tempChatInformation[tempId];
                for(auto sid:sessionsId)
                {
                    auto AiHelperPtr = userSessions[sid];
                    string sname=AiHelperPtr->GetMessages()[0].first;
                    sessionsName.push_back(sname);
                }
            }
        }
        else                 // 登录账号
        {
            int userId=stoi(session->getValue("userId"));
            {
                std::lock_guard<std::mutex> lock(server_->mutexForChatInformation);
                // 获取sessionsId
                sessionsId=server_->sessionsIdsMap[userId];
                // 获取sessionsName(可以直接将name存储,这里从aihelp中查询)
                auto& userSessions=server_->chatInformation[userId];
                for(auto sid:sessionsId)
                {
                    auto AiHelperPtr = userSessions[sid];
                    string sname=AiHelperPtr->GetMessages()[0].first;
                    sessionsName.push_back(sname);
                }
            }
        }

        // 封装响应报文
        json successResp;
        successResp["success"] = true;
        // 将vector的sessionsId和sessionsName封装为json::array
        successResp["sessions"] = json::array();
        for (size_t i = 0; i < sessionsId.size(); ++i) {
            json s;
            s["sessionId"] = sessionsId[i];
            s["name"] =sessionsName[i];  
            successResp["sessions"].push_back(s);
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