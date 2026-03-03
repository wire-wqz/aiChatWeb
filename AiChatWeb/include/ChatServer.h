// ChatServer.h 
// wire 
// 2025.12.8
/* 
AiChar 网页的核心类，成员属性包括httpserver类
在类内实现服务器：路由管理、会话管理、mysql连接池管理、ssl管理
同时，包含多个映射表

*/

#pragma once

#include <atomic>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <mutex>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <vector>


#include "../../HttpServer/include/http/HttpServer.h"            // httpserver 连接
#include "../../HttpServer/include/utils/MysqlUtil.h"            // mysql 连接工具
#include "../../HttpServer/include/utils/FileUtil.h"             // 文件读取工具 
#include "../../HttpServer/include/utils/JsonUtil.h"             // Json工具类

#include "../include/AiUtil/AiHelper.h"                          // AI对话处理                      
#include "../../HttpServer/include/messagequeue/MQManager.h"     // MQ消息队列


class ChatEntryHandler;



class ChatServer {

private:
	friend class ChatCreateAndSendHandler;
    friend class ChatSendHandler;
    friend class ChatEntryHandler;
    friend class ChatSessionsHandler;
    friend class ChatMessagesHandler;
    friend class ChatHeartCheckHandler;
    friend class ChatLoginHandler;
    friend class ChatLoginStatusHandler;
    friend class ChatLoginOutHandler;
    friend class ChatDeleteSessionHandler;
    friend class ChatRechargeUpdateHandler;
    friend class ChatRechargeHandler;
    
public:
    // 构造并初始化服务器
    ChatServer(
        int port,
		const std::string& name,
        int& numThreads,
        muduo::net::TcpServer::Option option = muduo::net::TcpServer::kNoReusePort);
	// 启动
    void start();
	
private:
	void initialize();                  // 全局初始化  
    void initializeMysql();             // MySQL初始化
	void initializeSession();           // 会话初始化
    void initializeMiddleware();        // 中间件初始化
	void initializeRouter();            // 路由初始化
    void initializeSsl();               // SSL加密套件初始化
    void initializeTimerCallback();     // 初始化时间回调 
    void initChatMessage();             // 从sql中读取登录用户的历史消息，存储在内存中(chatInformation等)
    void initMessageQueue();            // 初始化消息队列
    

    // http响应报装函数（提供给友元RouterHandler使用）
    void packageResp(const std::string& version, http::HttpResponse::HttpStatusCode statusCode, // 响应行
		const std::string& statusMsg, bool close, const std::string& contentType,               // 响应头
		int contentLen, const std::string& body, http::HttpResponse& resp);                     // 响应体 和 具体的响应对象
    	
    // 获取httpServer_的路由管理器（提供给友元RouterHandler使用）
    http::session::SessionManager* getSessionManager() const
	{
		return httpServer_.getSessionManager();
	}

private:
    http::HttpServer	httpServer_;                           // httpserver 类
    http::MysqlUtil		mysqlUtil_;                            // mysql工具类
    
    std::unordered_map<int, bool>	onlineUsers_;        // 在线用户状态管理
    std::mutex	mutexForOnlineUsers_;                    // 互斥锁

    std::unordered_map<int, std::unordered_map<std::string,std::shared_ptr<AiHelper> > > chatInformation;   // 用户id 聊天sessionid AIHelper双重对应
    std::mutex	mutexForChatInformation;

    std::unordered_map<int,std::vector<std::string> > sessionsIdsMap;                                       // 存储了用户id和聊天sessionid 的对应关系
    // std::mutex mutexForSessionsId;

    // std::unordered_map<int, std::shared_ptr<ImageRecognizer> > ImageRecognizerMap;
    // std::mutex	mutexForImageRecognizerMap;

    // 管理未登录用户,依托SessionManager
    std::unordered_map<std::string, std::unordered_map<std::string,std::shared_ptr<AiHelper> > > tempChatInformation;   // http会话session 聊天sessionid AIHelper双重对应
    std::mutex	mutexForTempChatInformation;

    std::unordered_map<std::string,std::vector<std::string> > tempSessionsIdsMap;                                      // http会话session和聊天sessionid 的对应关系
    // std::mutex mutexForTempSessionsId;                                                                              // 仅使用一把锁,避免冗余,因为tempChatInformation和tempSessionsIdsMap几乎同时访问

};



