// HttpServer.h 
// wire 
// 2025.11.26
/* 
框架核心，接入muduo网络库
新连接建立时的回调
接收连接数据的消息回调
对业务层提供业务回调接口
*/

#pragma once
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <unordered_map>

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Logging.h>

#include "HttpContext.h"                            // 解析请求报文
#include "HttpRequest.h"                            // 请求报文封装
#include "HttpResponse.h"                           // 响应报文封装
#include "../router/Router.h"                       // 路由管理模块
#include "../session/SessionManager.h"              // 会话管理模块
#include "../middleware/MiddlewareChain.h"          // 中间件管理模块
#include "../middleware/cors/CorsMiddleware.h"      // Cors中间件
#include "../ssl/SslConnection.h"                   // ssl 连接管理模块
#include "../ssl/SslContext.h"                      // ssl上下文管理模块
#include "../messagequeue/MQManager.h"              // MQ消息队列管理

// 类声明
class HttpRequest;
class HttpResponse;

using namespace std;

namespace http
{

class HttpServer : muduo::noncopyable  // muduo::noncopyable是 muduo 网络库中的一个工具类，用于禁止类的拷贝操作。
{
public:
    using HttpCallback = std::function<void (const http::HttpRequest&, http::HttpResponse&)>;

    // 构造函数（完整定义在cpp）
    HttpServer(int port,
            const std::string& name,
            bool useSSL = false,
            muduo::net::TcpServer::Option option = muduo::net::TcpServer::kNoReusePort);
    
    // 设置服务器的线程数量
    void setThreadNum(int numThreads) 
    {
        server_.setThreadNum(numThreads);
    }

    // 开启服务器
    void start();

    // 获取loop
    muduo::net::EventLoop* getLoop() const 
    { 
        return server_.getLoop(); 
    }

    // 重新设置 HttpCallback 回调： 默认handleRequest
    void setHttpCallback(const HttpCallback& cb)
    {
        httpCallback_ = cb;
    }

    // 处理路由处理器

    // 注册静态路由处理器（Get 方法）
    void Get(const std::string& path, const HttpCallback& cb)
    {
        router_.registerCallback(HttpRequest::kGet, path, cb);
    }
    void Get(const std::string& path, router::Router::HandlerPtr handler)
    {
        router_.registerHandler(HttpRequest::kGet, path, handler);
    }

    // 注册静态路由处理器（Post 方法）
    void Post(const std::string& path, const HttpCallback& cb)
    {
        router_.registerCallback(HttpRequest::kPost, path, cb);
    }
    void Post(const std::string& path, router::Router::HandlerPtr handler)
    {
        router_.registerHandler(HttpRequest::kPost, path, handler);
    }

    // 注册动态路由器（正则化注册）
    void addRoute(HttpRequest::Method method,const string& path, const HttpCallback& cb)
    {
        router_.addRegexCallback(method,path,cb);
    }

    void addRoute(HttpRequest::Method method, const std::string& path, router::Router::HandlerPtr handler)
    {
        router_.addRegexHandler(method, path, handler);
    }

    // 设置会话管理器
    void setSessionManager(std::unique_ptr<session::SessionManager> manager)
    {
        sessionManager_ = std::move(manager);
    }

    // 获取会话管理器
    session::SessionManager* getSessionManager() const
    {
        return sessionManager_.get(); // 返回智能指针的裸指针实例
    }

    // 添加中间件
    void addMiddleware(std::shared_ptr<middleware::Middleware> middleware) 
    {
        middlewareChain_.addMiddleware(middleware);
    }

    // 设置ssl许可
    void enableSSL(bool enable) 
    {
        useSSL_ = enable;
    }

    // 设置ssl配置，并创建ssl上下文
    void setSslConfig(const ssl::SslConfig& config);

    // 设置runEvery时间回调函数
    void setTimerCallback(double interval, const muduo::net::TimerCallback& cb);

    // 添加注册MQ消费者
    void addMqConsumer(string queueName,messagequeue::HandlerFunc handler)
    {
        messagequeue::MQConsumersPool::instance().addConsumer(queueName,handler);
    }
    void addMqConsumer(string queueName,messagequeue::HandlerPtr handler)
    {
        messagequeue::MQConsumersPool::instance().addConsumer(queueName,handler);
    }

    // MQ消费者启动
    void startMqConsumer(string queueName)
    {
        messagequeue::MQConsumersPool::instance().startConsumer(queueName); 
    }

    // MQ消费者停止
    void shutdownMqConsumer(string queueName)
    {
        messagequeue::MQConsumersPool::instance().shutdownConsumer(queueName); 
    }

private:
    void initialize();

    void onConnection(const muduo::net::TcpConnectionPtr& conn);             //  连接管理回调

    void onMessage(const muduo::net::TcpConnectionPtr& conn,                 //  数据接收回调
                   muduo::net::Buffer* buf,
                   muduo::Timestamp receiveTime);

    void onRequest(const muduo::net::TcpConnectionPtr& conn, const HttpRequest& req); // HTTP请求分发,将解析好的HTTP请求分发给具体的处理逻辑

    void handleRequest(const HttpRequest& req, HttpResponse& resp);          // 封装响应报文对象，被设置函数别名httpCallback_


private:
    muduo::net::InetAddress                      listenAddr_;               // 监听地址
    muduo::net::TcpServer                        server_;                   // 服务器类实例
    muduo::net::EventLoop                        mainLoop_;                 // 主循环
    HttpCallback                                 httpCallback_;             // 回调函数
    router::Router                               router_;                   // 路由
    std::unique_ptr<session::SessionManager>     sessionManager_;           // 会话管理器
    middleware::MiddlewareChain                  middlewareChain_;          // 中间件链
    bool                                         useSSL_;                   // 是否使用 SSL  
    std::unique_ptr<ssl::SslContext>             sslCtx_;                   // SSL 上下文
    std::map<muduo::net::TcpConnectionPtr, std::unique_ptr<ssl::SslConnection>> sslConns_;  // ssl连接池
};

} // namespace http