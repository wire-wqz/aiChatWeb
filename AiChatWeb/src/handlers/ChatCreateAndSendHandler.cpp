#include "../../include/handlers/ChatCreateAndSendHandler.h"

/*
前端传入信息
const response = await fetch('/chat/send-new-session', 
                {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/json' },
                    body: JSON.stringify({ question, modelType: modelTypeSelect.value })
                });
*/
void ChatCreateAndSendHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp){

    try
    {
        // 获取会话管理器(连接session)
        auto session=server_->getSessionManager()->getSession(req,resp);

        // 解析请求体
        std::string userQuestion;
        std::string modelType;
        std::string sessionId;
        bool isMcp = false;
        bool isGoogle = false;
        auto body = req.getBody();
        if (!body.empty()) 
        {
            auto j = json::parse(body);
            // 解析:question
            if (j.contains("question")) 
                userQuestion = j["question"];
            // 解析:modelType,默认"1"
            modelType = j.contains("modelType") ? j["modelType"].get<std::string>() : "1";
            // 解析:sessionId
            if(j.contains("sessionId"))
            {
                sessionId = j["sessionId"];
                std::cout<<"his-session-Id:"<<sessionId<<std::endl;
            }
            else
            {
                AiSessionIdGenerator generator;
                sessionId = generator.generate();
                std::cout<<"new-session-Id:"<<sessionId<<std::endl;
            }
            // 解析isMcp
            if (j.contains("isMcp")) 
                isMcp = j["isMcp"].get<bool>();
             // isGoogle
            if (j.contains("isGoogle")) 
                isGoogle = j["isGoogle"].get<bool>();

        }

        // 判断登录状态:未认证/登录/临时
        if(session->getValue("isLoggedIn").empty())              // 未认证 重新认证登录逻辑 
        {
            session->setValue("isLoggedIn","false");
        }
        bool is_Login=session->getValue("isLoggedIn")=="true";   // 登录/临时
      

        // 与AI服务器交互
        string aiInformation;                                    // 返回的AI消息
        int totalTokens;
        if (!is_Login)   // 临时账户(临时账户依托SessionManager,数据仅存储在内存中,不存储到sql)
        {
            // 获得临时ID
            string tempId=session->getId();
            // 每个用户的每个聊天会话session使用单独的AIHelperPtr
            std::shared_ptr<AiHelper> AIHelperPtr;
            {
                // 互锁
                std::lock_guard<std::mutex> lock(server_->mutexForTempChatInformation);
                // 如果存在userId，则取出，不存在则创建(创建)
                auto& userSessions = server_->tempChatInformation[tempId];
                // 如果userSessions中不存在sessionId则创建(创建)
                if (userSessions.find(sessionId) == userSessions.end()) {
                    userSessions.emplace( 
                        sessionId,
                        std::make_shared<AiHelper>()
                    );
                    server_->tempSessionsIdsMap[tempId].push_back(sessionId); // 向vector中添加具有的sessionId
                }
                // 取出对应的AIHelperPtr
                AIHelperPtr= userSessions[sessionId];
                AIHelperPtr->isGoogle_=isGoogle;
                AIHelperPtr->isMCP_=isMcp;
            }
            json aiResult = AIHelperPtr->chat(tempId,is_Login,sessionId,userQuestion,modelType);
            aiInformation = aiResult["aiInformation"];
        }
        else{  // 将聊天消息存储在sql中
            // 获得用户ID
            int userId=stoi(session->getValue("userId"));
            // 每个用户的每个聊天会话session使用单独的AIHelperPtr
            std::shared_ptr<AiHelper> AIHelperPtr;
            {
                // 互锁
                std::lock_guard<std::mutex> lock(server_->mutexForChatInformation);
                // 如果存在userId，则取出，不存在则创建(创建)
                auto& userSessions = server_->chatInformation[userId];
                // 如果userSessions中不存在sessionId则创建(创建)
                if (userSessions.find(sessionId) == userSessions.end()) {
                    userSessions.emplace( 
                        sessionId,
                        std::make_shared<AiHelper>()
                    );
                    server_->sessionsIdsMap[userId].push_back(sessionId); // 向vector中添加具有的sessionId
                }
                // 取出对应的AIHelperPtr
                AIHelperPtr= userSessions[sessionId];
                AIHelperPtr->isGoogle_=isGoogle;
                AIHelperPtr->isMCP_=isMcp;
            }
            json aiResult = AIHelperPtr->chat(std::to_string(userId),is_Login,sessionId,userQuestion,modelType); 
            aiInformation = aiResult["aiInformation"];
            totalTokens = aiResult["totalTokens"];
        }
        

        // // 临时的AIHelper(调试代码)
        // auto AIHelperPtr=std::make_shared<AiHelper>();
        // // 基于临时账号实现AI对话
        // std::string aiInformation=AIHelperPtr->chat(123, "root",sessionId, userQuestion, modelType);

        // 封装响应报文
        json successResp;
        successResp["success"] = true;
        successResp["Information"] = aiInformation;
        successResp["sessionId"] = sessionId;
        if(is_Login)
            successResp["totalTokens"] = totalTokens;  
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


// 基于id查询Tokens
int ChatCreateAndSendHandler::queryTokens(int& userId)
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