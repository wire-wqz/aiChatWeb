#include "../../include/handlers/ChatSendHandler.h"

/*
  const response = await fetch('/chat/send', {
                        method: 'POST',
                        headers: { 'Content-Type': 'application/json' },
                        body: JSON.stringify({ question, modelType: modelTypeSelect.value, sessionId: currentSessionId })
                    });
*/
void ChatSendHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp){
    try
    {
        // 获取会话管理器(连接session)
        auto session=server_->getSessionManager()->getSession(req,resp);

        // 登录与否逻辑
        // LOG_INFO << "session->getValue(\"isLoggedIn\") = " << session->getValue("isLoggedIn");
        // if (session->getValue("isLoggedIn") != "true")
        // {
        //     json errorResp;
        //     errorResp["status"] = "error";
        //     errorResp["message"] = "Unauthorized";
        //     std::string errorBody = errorResp.dump(4);

        //     server_->packageResp(req.getVersion(), http::HttpResponse::k401Unauthorized,
        //         "Unauthorized", true, "application/json", errorBody.size(),
        //         errorBody, resp);
        //     return;
        // }
        // int userId = std::stoi(session->getValue("userId"));
        // std::string username = session->getValue("username");

        // 从请求体中解析userQuestion、modelType和currentSessionId
        std::string userQuestion;
        std::string modelType;
        std::string sessionId;
        auto body = req.getBody();
        if (!body.empty()) 
        {
            // 解析字符串并创建 JSON 对象
            auto j = json::parse(body);
            if (j.contains("question")) 
                userQuestion = j["question"];
            if (j.contains("sessionId")) 
                sessionId = j["sessionId"];
            modelType = j.contains("modelType") ? j["modelType"].get<std::string>() : "1";
        }

        // 与ai服务器交互
        string aiInformation;  // 返回的AI消息
        bool is_Login = session->getValue("isLoggedIn") == "true";

        // 临时账户(临时账户依托SessionManager,数据仅存储在内存中,不存储到sql)
        if (!is_Login)
        {
            // 获得临时ID
            string tempId=session->getId();
             // 每个用户的每个聊天会话session使用单独的AIHelperPtr
            std::shared_ptr<AiHelper> AIHelperPtr;
            {
                // 互锁
                std::lock_guard<std::mutex> lock(server_->mutexForTempChatInformation);
                // 如果存在userId，则取出，不存在则创建(取出)
                auto& userSessions = server_->tempChatInformation[tempId];
                // 如果userSessions中不存在sessionId则创建(不执行)
                if (userSessions.find(sessionId) == userSessions.end()) {
                    userSessions.emplace( 
                        sessionId,
                        std::make_shared<AiHelper>()
                    );
                    server_->tempSessionsIdsMap[tempId].push_back(sessionId);  // 不执行,无用
                }
                // 取出对应的AIHelperPtr
                AIHelperPtr= userSessions[sessionId];
            }
            aiInformation=AIHelperPtr->chat(tempId, is_Login,sessionId, userQuestion, modelType);
        }

        // // 每个用户的每个聊天会话session使用单独的AIHelperPtr 
        // std::shared_ptr<AiHelper> AIHelperPtr;
        // {
        //     // 互锁
        //     std::lock_guard<std::mutex> lock(server_->mutexForChatInformation);
        //     // 如果存在userId，则取出，不存在则创建(取出)
        //     auto& userSessions = server_->chatInformation[userId];
        //     // 如果userSessions中不存在sessionId则创建(不执行)
        //     if (userSessions.find(sessionId) == userSessions.end()) {

        //         userSessions.emplace( 
        //             sessionId,
        //             std::make_shared<AiHelper>()
        //         );
        //         server_->sessionsIdsMap[userId].push_back(sessionId); // 这里不需要这个，这个是啥暂时未考究
        //     }
        //     // 取出对应的AIHelperPtr
        //     AIHelperPtr= userSessions[sessionId];
        // }
        // std::string aiInformation=AIHelperPtr->chat(userId, username,sessionId, userQuestion, modelType);

        // // 临时的AIHelper(调试代码)
        // auto AIHelperPtr=std::make_shared<AiHelper>();
        // // 基于临时账号实现AI对话
        // std::string aiInformation=AIHelperPtr->chat(123,sessionId, userQuestion, modelType);

        // // 封装响应报文
        json successResp;
        successResp["success"] = true;
        successResp["Information"] = aiInformation;
        successResp["sessionId"] = sessionId;
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