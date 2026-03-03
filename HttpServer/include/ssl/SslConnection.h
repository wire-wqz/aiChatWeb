// SslConfig.h 
// wire 
// 2025.12.4
/* 
引入https使用OpenSSL库，添加加密层

关键类：

SslConfig：     存储SSL配置，包括证书和密钥文件路径
SslContext:     初始化和管理SSL上下文
SslConnection： 处理SSL连接，包括握手，数据加密、解密

SslConnection： 处理SSL连接，包括握手，数据加密、解密

// 使用说明
// 创建SSL配置的封装类
ssl::SslConfig config;
// 设置config
*****省略*****
// 创建SslContext指针初始化ssl
std::unique_ptr<ssl::SslContext> sslContextPtr(new ssl::SslContext(config));
sslContextPtr-> initialize();
// 创建SslConnection类，传入TcpConnectionPtr连接指针和SslContext指针
std::unique_ptr<ssl::SslConnection> sslConnectionPtr(conn,sslContextPtr.get());
sslConn->setMessageCallback(onmessage); 保存原有回调

*/



#pragma once
#include "SslContext.h"
#include <muduo/net/TcpConnection.h>
#include <muduo/net/Buffer.h>
#include <muduo/base/noncopyable.h>
#include <openssl/ssl.h>
#include <memory>


namespace ssl 
{
class SslConnection : muduo::noncopyable 
{
public:
    
    // 构造函数 传输muduo conn 指针和上下文指针
    SslConnection(const muduo::net::TcpConnectionPtr& conn, SslContext* ctx);
    ~SslConnection();

    // ssl握手
    void startHandshake();
    
    // 判断握手是否成功
    bool isHandshakeCompleted() const { return state_ == SSLState::ESTABLISHED; }

    // sslconn的消息回调，会覆盖conn的原有回调
    void onRead(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp time);

    // SSL 连接中应用层发送数据的入口（应用层的明文数据，加密后的数据通过 TCP 连接发送到网络）
    void send(muduo::net::Buffer* plain);

    // 设置消息回调函数(存储原conn中的消息回调函数，新加密解密逻辑回调函数onread设置为conn的实际回调
    // 也就说是消息到来时，执行onread，onread 执行 解密 messageCallback_ 加密
    void setMessageCallback(const muduo::net::MessageCallback& cb) { messageCallback_ = cb; }
     

private:
    // 处理SSL/TLS握手过程
    void handleHandshake();      
    
    // 把 write BIO 中的加密字节发送到底层连接
    void drainWriteBio();
    
    // 将OpenSSL错误码转换为自定义错误类型
    SSLError getLastError(int ret);

    // 错误处理
    void handleError(SSLError error);

private:
    SSL*                            ssl_;              // SSL 连接
    SslContext*                     ctx_;              // SSL 上下文
    muduo::net::TcpConnectionPtr    conn_;             // TCP 连接
    SSLState                        state_;            // SSL 状态
    BIO*                            readBio_;          // 网络数据 -> SSL
    BIO*                            writeBio_;         // SSL -> 网络数据
    // muduo::net::Buffer*             readBuffer_;       // 读缓冲区
    // muduo::net::Buffer*             writeBuffer_;      // 写缓冲区
    // muduo::net::Buffer*             decryptedBuffer_;  // 解密后的数据
    muduo::net::MessageCallback     messageCallback_;  // 消息回调

};

} // namespace ssl