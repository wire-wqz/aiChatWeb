// ChatMessagesHandler.h 
// wire 
// 2025.1.5
/* 
从内存中恢复会话列表的messages,依据chatInformation/tempChatInformation
Post请求,只恢复切换到的那一个会话
*/


#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"


class ChatMessagesHandler : public http::router::RouterHandler
{
public:
    explicit ChatMessagesHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;
private:

private:
    ChatServer*         server_;
};