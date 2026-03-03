#include "../../include/http/HttpServer.h"

#include <any>
#include <functional>
#include <memory>

namespace http
{



HttpServer::HttpServer(int port,
        const std::string& name,
        bool useSSL,
        muduo::net::TcpServer::Option option)
        : listenAddr_(port)
        , server_(&mainLoop_, listenAddr_, name, option)
        , useSSL_(useSSL)
        , httpCallback_(std::bind(&HttpServer::handleRequest, this, std::placeholders::_1, std::placeholders::_2))  
        // 将httpCallback_ 回调设置为类成员函数HttpServer::handleRequest
        // 等效于 httpCallback_([this](const HttpRequest& req, HttpResponse* resp){handleRequest(req,resp);})  
        {
            initialize(); // 设置连接回调/消息回调
        }


void HttpServer::initialize(){
    server_.setConnectionCallback(std::bind(&HttpServer::onConnection,this,std::placeholders::_1));
    server_.setMessageCallback
            (
                std::bind(&HttpServer::onMessage,
                this,
                std::placeholders::_1,
                std::placeholders::_2,
                std::placeholders::_3)
            );
}

// 设置ssl配置，并创建ssl上下文
void HttpServer::setSslConfig(const ssl::SslConfig& config)
{
    if (useSSL_)
    {
        sslCtx_ = std::make_unique<ssl::SslContext>(config);
        if (!sslCtx_->initialize())
        {
            LOG_ERROR << "Failed to initialize SSL context";
            abort();
        }
    }
}

void HttpServer::start(){
    LOG_WARN << "HttpServer[" << server_.name() << "] starts listening on" << server_.ipPort();
    server_.start();
    mainLoop_.loop();
}

//  连接管理回调
void HttpServer::onConnection(const muduo::net::TcpConnectionPtr& conn){
    if(conn->connected()){
        if (useSSL_)
        {
            // 创建ssl连接
            auto sslConn = std::make_unique<ssl::SslConnection>(conn, sslCtx_.get()); 
            // 存储原消息回调
            sslConn->setMessageCallback(std::bind(&HttpServer::onMessage, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
            // 加入连接池
            sslConns_[conn] = std::move(sslConn);
            // 握手
            sslConns_[conn]->startHandshake();
        }
        conn->setContext(HttpContext());
    }
    else
    {
        if (useSSL_)
        {
            sslConns_.erase(conn);
        }
    }
}

//  数据接收回调
void HttpServer::onMessage(
                const muduo::net::TcpConnectionPtr& conn,                 
                muduo::net::Buffer* buf,
                muduo::Timestamp receiveTime){
    try
    {
        if (useSSL_)
        {
            /* ssl 逻辑 */
        }
 
        // 基于HttpContext对象解析请求报文，封装到HttpRequest对象中
        HttpContext *context = boost::any_cast<HttpContext>(conn->getMutableContext()); // 与 conn->setContext(HttpContext()); 对应
        if(!context->parseRequest(buf,receiveTime))
        {
            // 解析失败
            conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conn->shutdown();
        }

        if(context->gotAll())
        {
            // 解析完成（状态为kGotAll）
            onRequest(conn, context->getRequest());       // 交给onRequest 处理封装后的请求
            context->reset();                             // 重置
        }

    }
    catch (const std::exception &e)
    {
        // 捕获异常，返回错误信息
        LOG_ERROR << "Exception in onMessage: " << e.what();
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }
}

// 处理请求
void HttpServer::onRequest(const muduo::net::TcpConnectionPtr& conn, const HttpRequest& req){

    const std::string &connection = req.getHeader("Connection"); // 获取HTTP请求中的Connection头部字段 判断连接方式
    bool close = 
    ((connection == "close") ||(req.getVersion() == "HTTP/1.0" && connection != "Keep-Alive")); // close =1 代表短链接
    // 响应报文封装类
    HttpResponse response(close);

    // 依据请求报文封装响应报文
    httpCallback_(req, response);

    // 将响应报文内容放在发送缓冲区
    muduo::net::Buffer buf;
    response.appendToBuffer(&buf);
    // LOG_INFO << "Sending response:\n" << buf.toStringPiece().as_string();    // 打印完整的响应内容用于调试
    // 发送
    if(!useSSL_) // 未使用ssl 直接发送
    {
        conn->send(&buf);
    }
    else         // 借助sslconn 发送加密数据
    {
        // 查找连接池中对应的连接
        auto it=sslConns_.find(conn);
        if(it != sslConns_.end())
        {
            it->second->send(&buf);
        }

    }
    
    // 如果是短连接的话，返回响应报文后就断开连接
    if (response.getCloseConnection())
    {
        conn->shutdown();
    }
}

//  依据请求报文封装响应报文，被设置函数别名httpCallback_
void HttpServer::handleRequest(const HttpRequest& req, HttpResponse& resp){

   try{
        // 中间件处理请求
        HttpRequest mutableReq = req;
        middlewareChain_.processBefore(mutableReq); // req 是const，processBefore会对响应修改，因此这里修改为非常量
        
        // 执行路由器（根据HTTP请求的方法和路径，找到对应的处理函数并执行）
        if(!router_.route(mutableReq, resp))
        {
                LOG_INFO << "请求的url：" << req.getMethod() << " " << req.getPath();
                LOG_INFO << "未找到路由，返回404";
                resp.setVersion(req.getVersion());
                resp.setStatusCode(HttpResponse::k404NotFound);
                resp.setStatusMessage("Not Found");
                resp.setBody("Sorry Not Found");
                resp.setCloseConnection(true);
        }

        // 中间件处理响应
            middlewareChain_.processAfter(resp);
    }
    catch (const HttpResponse& res) 
    {
        // 处理中间件抛出的响应（如CORS预检请求）
        resp = res;
    }
    catch (const std::exception& e) 
    {
        // 错误处理
        resp.setStatusCode(HttpResponse::k500InternalServerError);
        resp.setBody(e.what());
    }

}    

 // 设置runEvery时间回调函数
void HttpServer::setTimerCallback(double interval, const muduo::net::TimerCallback& cb){
    mainLoop_.runEvery(interval,cb);
}



} //namespace http