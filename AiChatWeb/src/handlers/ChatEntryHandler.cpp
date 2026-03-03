#include "../../include/handlers/ChatEntryHandler.h"


void ChatEntryHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
{
    
    // 获取会话管理器(连接session)
    auto session=server_->getSessionManager()->getSession(req,resp);
    // 验证是否登录(todo)
    if(session->getValue("isLoggedIn").empty())  
    {
        //登录逻辑todo(登录逻辑默认为临时,只有按了登录案例才切换为true,会话过期则重置)
        session->setValue("isLoggedIn","false");
    }
    
    
    std::string reqFile;
    reqFile.append("/root/AiHttpServer/AiChatWeb/resource/AiChat.html"); // main.cpp 相对于 html 的路径
    FileUtil fileOperater(reqFile);
    if (!fileOperater.isValid())
    {
        LOG_WARN << reqFile << " not exist";
        fileOperater.resetDefaultFile(); // 返回NotFound.html  
    }

    std::vector<char> buffer(fileOperater.size());
    fileOperater.readFile(buffer); 
    std::string bufStr = std::string(buffer.data(), buffer.size());

    resp.setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
    resp.setCloseConnection(false);
    resp.setContentType("text/html");
    resp.setContentLength(bufStr.size());
    resp.setBody(bufStr);
}