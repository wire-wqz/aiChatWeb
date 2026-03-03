#include "../../include/handlers/ChatLoginHandler.h"


void ChatLoginHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
{
    try{
        // 获取会话管理器(连接session)
        auto session=server_->getSessionManager()->getSession(req,resp);

        // 解析请求体
        std::string userName;
        std::string password;
        auto body = req.getBody();
        if (!body.empty()) 
        {
            auto j = json::parse(body);
            // 解析:userId
            if (j.contains("userName")) 
                userName = j["userName"];
            if (j.contains("password")) 
                password = j["password"];
        }

        // 判断登录状态:未认证/登录/临时
        if(session->getValue("isLoggedIn").empty())              // 未认证 重新认证登录逻辑 
        {
            session->setValue("isLoggedIn","false");
        }
        bool is_Login=session->getValue("isLoggedIn")=="true";   // 登录/临时

        // 判断userId和password在数据库中是否存在
        if(!is_Login)
        {
            // 查询数据库
            int userId = queryUserId(userName, password); 
            if (userId != -1) // 存在该数据（验证通过）
            {
                is_Login = true;
                session->setValue("userId",std::to_string(userId));
                session->setValue("userName", userName);
                session->setValue("isLoggedIn","true");


                // 在线用户表添加
                if (server_->onlineUsers_.find(userId) == server_->onlineUsers_.end() || server_->onlineUsers_[userId] == false)
                {
                    {
                        std::lock_guard<std::mutex> lock(server_->mutexForOnlineUsers_);
                        server_->onlineUsers_[userId] = true;
                    }
                }

                // 将session从临时列表中手动去除
                {
                    std::lock_guard<std::mutex> lock(server_->mutexForTempChatInformation);
                    std::string tempId = session->getId();
                    server_->tempChatInformation.erase(tempId);
                    server_->tempSessionsIdsMap.erase(tempId);
                }
            }
        }


        // 封装响应报文
        json successResp;
        successResp["success"] = true;    
        successResp["is_Login"] = is_Login;   
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

// 基于用户名查询mysqlId
int ChatLoginHandler::queryUserId(const std::string& username, const std::string& inputPassword)
{
    std::string sql = "SELECT id, password FROM users WHERE username = ?";
    // std::vector<std::string> params = {username, password};
    auto res = mysqlUtil_.executeQuery(sql, username);
    if (res->next())
    {
        int id = res->getInt("id");
        std::string stored_hash = res->getString("password");
        // 验证密码
        if (crypto_pwhash_str_verify(
                stored_hash.c_str(), 
                inputPassword.c_str(), 
                inputPassword.length()
            ) == 0) {
            return id;
        }
        return -1;        
    }
    return -1;
}

