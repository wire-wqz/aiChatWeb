// SslTypes.h 
// wire 
// 2025.12.4
/* 
引入https使用OpenSSL库，添加加密层

关键类：

SslConfig：     存储SSL配置，包括证书和密钥文件路径
SslContext:     初始化和管理SSL上下文
SslConnection： 处理SSL连接，包括握手，数据加密、解密


SslTypes:       定义了常使用的枚举类型
*/



#pragma once
#include <string>

namespace ssl 
{

// SSL/TLS 协议版本
enum class SSLVersion 
{
    TLS_1_0,
    TLS_1_1,
    TLS_1_2,
    TLS_1_3
};

// SSL 错误类型
enum class SSLError 
{
    NONE,
    WANT_READ,
    WANT_WRITE,
    SYSCALL,
    SSL,
    UNKNOWN
};

// SSL 状态
enum class SSLState 
{
    HANDSHAKE,            // 握手
    ESTABLISHED,          // 连接已建立
    SHUTDOWN,             // 连接关闭
    ERROR
};

} // namespace ssl