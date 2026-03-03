#include "../include/ChatServer.h"

// 放在.h文件会导致循环引用
#include "../include/handlers/ChatEntryHandler.h" 
#include "../include/handlers/ChatCreateAndSendHandler.h" 
#include "../include/handlers/ChatSendHandler.h" 
#include "../include/handlers/GetStaticFileHandler.h" 
#include "../include/handlers/ChatSessionsHandler.h"
#include "../include/handlers/ChatMessagesHandler.h"  
#include "../include/handlers/ChatHeartCheckHandler.h" 
#include "../include/handlers/ChatLoginHandler.h" 
#include "../include/handlers/ChatLoginStatusHandler.h" 
#include "../include/handlers/ChatLoginOutHandler.h" 
#include "../include/handlers/ChatRegisterHandler.h"
#include "../include/handlers/ChatDeleteSessionHandler.h"
#include "../include/handlers/ChatRechargeUpdateHandler.h"
#include "../include/handlers/ChatRechargeHandler.h"
#include "../include/handlers/ChatRechargeNotifyHandler.h"

#include "../include/MQHandlers/MessageSqlHandler.h"
#include "../include/MQHandlers/RegisterSqlHandler.h"
#include "../include/MQHandlers/TokensSqlHandler.h"
#include "../include/MQHandlers/RechargeSqlHandler.h"
#include "../include/MQHandlers/RechargeNotifySqlHandler.h"

// 构造并初始化服务器
ChatServer::ChatServer(
        int port,
		const std::string& name,
        int& numThreads,
        muduo::net::TcpServer::Option option)
        :httpServer_(port, name, option)
        {
            httpServer_.setThreadNum(numThreads);
            initialize();
        }

// 启动
void ChatServer::start(){
    httpServer_.start();
}

// 全局初始化  
void ChatServer::initialize()   
{
    std::cout << "initialize start" << std::endl;
    initializeMysql();
    initializeSession();
    initializeMiddleware();
    initializeRouter();
    initializeSsl();
    initializeTimerCallback();
    initChatMessage();
    initMessageQueue();
    std::cout << "initialize finished" << std::endl;
}

// MySQL初始化
void ChatServer::initializeMysql(){
    mysqlUtil_.init("tcp://127.0.0.1", "wireMysql", "wire123", "ChatHttpServer", 10);
    std::cout << "initializeMysql finished" << std::endl;
}             

// 会话初始化
void ChatServer::initializeSession(){
    // 会话管理存储方式
    auto sessionStorage = std::make_unique<http::session::MemorySessionStorage>();
    // 会话管理
    auto sessionManager = std::make_unique<http::session::SessionManager>(std::move(sessionStorage),7200);
    // 会话初始化
    httpServer_.setSessionManager(std::move(sessionManager));
    std::cout << "initializeSession finished" << std::endl;
}        

// 中间件初始化
void ChatServer::initializeMiddleware(){
    auto cors = std::make_shared<http::middleware::CorsMiddleware>();
    httpServer_.addMiddleware(std::move(cors));
    std::cout << "initializeMiddleware finished" << std::endl;
}

// 路由初始化
void ChatServer::initializeRouter()
{
    httpServer_.Get("/",std::make_shared<ChatEntryHandler>(this));
    httpServer_.Get("/chat",std::make_shared<ChatEntryHandler>(this));
    httpServer_.Get("/chat/sessions",std::make_shared<ChatSessionsHandler>(this));
    httpServer_.Get("/chat/check",std::make_shared<ChatHeartCheckHandler>(this));
    httpServer_.Get("/chat/loginStatus", std::make_shared<ChatLoginStatusHandler>(this));

    // httpServer_.Post("/chat/send-new-session", std::make_shared<ChatCreateAndSendHandler>(this));
    httpServer_.Post("/chat/send", std::make_shared<ChatCreateAndSendHandler>(this));
    httpServer_.Post("/chat/messages", std::make_shared<ChatMessagesHandler>(this));
    httpServer_.Post("/chat/login", std::make_shared<ChatLoginHandler>(this));
    httpServer_.Post("/chat/logout", std::make_shared<ChatLoginOutHandler>(this));
    httpServer_.Post("/chat/register", std::make_shared<ChatRegisterHandler>(this));
    httpServer_.Post("/chat/delSession", std::make_shared<ChatDeleteSessionHandler>(this));
    httpServer_.Post("/chat/rechargeUpdate", std::make_shared<ChatRechargeUpdateHandler>(this));
    httpServer_.Post("/chat/recharge", std::make_shared<ChatRechargeHandler>(this));
    httpServer_.Post("/chat/rechargeAliNotify", std::make_shared<ChatRechargeNotifyHandler>(this));

    // 动态路由（正则匹配）
    httpServer_.addRoute(http::HttpRequest::kGet,"/css/.*",std::make_shared<GetStaticFileHandler>(this));
    httpServer_.addRoute(http::HttpRequest::kGet,"/images/.*",std::make_shared<GetStaticFileHandler>(this));
    httpServer_.addRoute(http::HttpRequest::kGet,"/js/.*",std::make_shared<GetStaticFileHandler>(this));
    
    std::cout << "initializeRouter finished" << std::endl;
}      

// SSL加密套件初始化
void ChatServer::initializeSsl()
{
    httpServer_.enableSSL(true); // 启用ssl加密
    ssl::SslConfig config;
    config.setPrivateKeyFile("/root/AiHttpServer/AiChatWeb/resource/ssl/server.key");
    config.setCertificateFile("/root/AiHttpServer/AiChatWeb/resource/ssl/server.crt");
    httpServer_.setSslConfig(config);
    std::cout << "initializeSsl finished" << std::endl;
}            

// 初始化时间回调 
void ChatServer::initializeTimerCallback()
{
    httpServer_.setTimerCallback(3600.0,[this]()
        {
            std::cout<<"开始定时清理过期临时会话"<<std::endl;
            auto sessionManager=httpServer_.getSessionManager();
            
            // 处理登录会话的清理
            auto expiredSessions=sessionManager->getExpiredSessions();
            for(const auto& s:expiredSessions){
                if(s->getValue("isLoggedIn")=="true")
                {
                    int userId = stoi(s->getValue("userId"));
                    std::lock_guard<std::mutex> lock(mutexForOnlineUsers_);
                    onlineUsers_[userId] = false;
                    std::cout<< "用户Id:"<<userId<<"下线"<<std::endl;
                }
            }
            // 处理临时会话的清理（可以优化，从expiredsessions中处理）
            std::vector<std::string> toRemove;
            {
                std::lock_guard<std::mutex> lock(mutexForTempChatInformation);  
                // std::cout<<"tempSessions清理前:"<<tempSessionsIdsMap.size()<<std::endl;
                for(const auto kv:tempSessionsIdsMap)
                {
                    const std::string tempId=kv.first;
                    if(!sessionManager->isValid(tempId)) // 过期(会自动将过期的移除,Vaild表示存在且未过期)
                    {
                        // tempSessionsIdsMap.erase(tempId);              // 在迭代的过程中被删除
                        // tempChatInformation.erase(tempId);
                        toRemove.push_back(tempId);
                    }
                }
                for(const auto& removeTempId: toRemove)
                {
                    std::cout<<"tempId:"<<removeTempId<<"已过期,清理"<<std::endl;
                    tempSessionsIdsMap.erase(removeTempId);  
                    tempChatInformation.erase(removeTempId);  
                }
            }
            // std::cout<<"tempSessions清理后:"<<tempSessionsIdsMap.size()<<std::endl;
            // 从内存中删除过期会话
            sessionManager->cleanExpiredSessions();
            std::cout<<"清理完成"<<std::endl;
        }
    );
    std::cout << "initializeTimerCallback finished" << std::endl;
}  

// 从sql中读取登录用户的历史消息，存储在内存中(chatInformation等)
void ChatServer::initChatMessage()
{
    // 查询数据库
    std::string sql= "select id,session_id,content,ts from chat_message";
    sql::ResultSet* res;
    try 
    {
        res = mysqlUtil_.executeQuery(sql);
    }
    catch (const std::exception& e) {
        std::cerr << "MySQL query failed: " << e.what() << std::endl;
        return;
    }
    

    // 逐行处理
    while (res->next())
    {
        int userId;
        std::string sessionId;
        std::string messContent;
        long long ts;
        
        // 读取数据
        try{
            userId      = res->getInt64("id");
            sessionId   = res->getString("session_id");
            messContent = res->getString("content");
            ts          = res->getInt64("ts");
        }
        catch (const std::exception& e) {
            std::cerr << "Failed to read row: " << e.what() << std::endl;
            continue; 
        }

        // 保存到类内内存中
        std::lock_guard<std::mutex> lock(mutexForChatInformation);
        // 用户id不存在则创建
        auto& userSessions = chatInformation[userId]; // 这里一定要是引用&
        // 如果该用户id的sessionid没有存储
        if (userSessions.find(sessionId) == userSessions.end())  
        {
            userSessions.emplace(sessionId, std::make_shared<AiHelper>());
            sessionsIdsMap[userId].push_back(sessionId);   // 向vector中添加具有的sessionId
        }
        // 存储对话内容
        userSessions[sessionId]->restoreMessage(messContent,ts);
    }  
    std::cout << "initChatMessage finished" << std::endl;
}

// 初始化消息队列(现在存在的问题：MQserver不是全局函数，初始化函数结束就被消除)
void ChatServer::initMessageQueue(){

    // // 消费者线程数量
    // int threadNum_MQ = 2;

    // // 注册消息消费函数
    // auto MQserver=make_shared<http::messagequeue::RabbitMQThreadPool>();

    // // 注册
    // MQserver->registerConsumer("message_sql_queue",threadNum_MQ,[this](std::string sql){
    //         mysqlUtil_.executeUpdate(sql);
    //         std::cout<<"MQ:SQL写入完成"<<std::endl;
    //     }
    // );

    // // 启动
    // MQserver->start();

    // // 保存到类内池子中
    // mqConsumers_.push_back(std::move(MQserver));

    // 消费者
    httpServer_.addMqConsumer("message_sql_queue",std::make_shared<MessageSqlHandler>(this)); // 父类指针指向子类对象
    httpServer_.startMqConsumer("message_sql_queue");
    
    httpServer_.addMqConsumer("register_sql_queue",std::make_shared<RegisterSqlHandler>(this)); // 父类指针指向子类对象
    httpServer_.startMqConsumer("register_sql_queue");

    httpServer_.addMqConsumer("tokens_sql_queue",std::make_shared<TokensSqlHandler>(this)); // 父类指针指向子类对象
    httpServer_.startMqConsumer("tokens_sql_queue");

    httpServer_.addMqConsumer("recharge_sql_queue",std::make_shared<RechargeSqlHandler>(this)); // 父类指针指向子类对象
    httpServer_.startMqConsumer("recharge_sql_queue");

    httpServer_.addMqConsumer("rechargeNotify_sql_queue",std::make_shared<RechargeNotifySqlHandler>(this)); // 父类指针指向子类对象
    httpServer_.startMqConsumer("rechargeNotify_sql_queue");

    

    // 给线程时间启动
    std::this_thread::sleep_for(std::chrono::seconds(2));

    std::cout << "initMessageQueue finished" << std::endl;
}        

 // http响应报装函数（提供给友元RouterHandler使用）
void ChatServer::packageResp(const std::string& version, http::HttpResponse::HttpStatusCode statusCode, // 响应行
    const std::string& statusMsg, bool close, const std::string& contentType,                           // 响应头
    int contentLen, const std::string& body, http::HttpResponse& resp)                                  // 响应体 和 具体的响应对象
{
    if (&resp == nullptr)
    {
        LOG_ERROR << "Response pointer is null";
        return;
    }
    
    try
    {
        resp.setVersion(version);
        resp.setStatusCode(statusCode);
        resp.setStatusMessage(statusMsg);
        resp.setCloseConnection(close);
        resp.setContentType(contentType);
        resp.setContentLength(contentLen);
        resp.setBody(body);

        LOG_INFO << "Response packaged successfully";
    }
    catch (const std::exception& e)
    {
        LOG_ERROR << "Error in packageResp: " << e.what();
        resp.setStatusCode(http::HttpResponse::k500InternalServerError);
        resp.setStatusMessage("Internal Server Error");
        resp.setCloseConnection(true);
    }


}