// ChatSessionsHandler.h 
// wire 
// 2025.1.5
/* 
从内存中恢复会话列表的sessionID和name,依据tempSessionsIdsMap/sessionsIdsMap
*/


#pragma once
#include "../ChatServer.h"
#include "../../../HttpServer/include/router/RouterHandler.h"


class ChatSessionsHandler : public http::router::RouterHandler
{
public:
    explicit ChatSessionsHandler(ChatServer* server) : server_(server) {}

    void handle(const http::HttpRequest& req, http::HttpResponse& resp) override;
private:

private:
    ChatServer*         server_;
};