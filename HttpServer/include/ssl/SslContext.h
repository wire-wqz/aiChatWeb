// SslConfig.h 
// wire 
// 2025.12.4
/* 
引入https使用OpenSSL库，添加加密层

关键类：

SslConfig：     存储SSL配置，包括证书和密钥文件路径
SslContext:     初始化和管理SSL上下文
SslConnection： 处理SSL连接，包括握手，数据加密、解密

SslContext      初始化和管理SSL上下文
初始化SSL环境(设置SSL协议版本和选项)
加载证书
配置SSL参数
*/


#pragma once
#include "SslConfig.h"
#include <openssl/ssl.h>
#include <memory>
#include <muduo/base/noncopyable.h>

namespace ssl 
{

class SslContext : muduo::noncopyable 
{
public:
    // 构造函数传入证书封装类
    explicit SslContext(const SslConfig& config);
    ~SslContext();

    // 初始化
    bool initialize();

    // 返回SSL上下文
    SSL_CTX* getNativeHandle() { return ctx_; }



private:
    // 加载证书
    bool loadCertificates();
    
    // 配置ssl协议参数
    bool setupProtocol();
    
    // 设置会话缓存
    void setupSessionCache();

    // 错误处理
    static void handleSslError(const char* msg);


private:
    SSL_CTX*  ctx_;     // SSL上下文
    SslConfig config_;  // SSL配置

};

}  //namespace ssl 
