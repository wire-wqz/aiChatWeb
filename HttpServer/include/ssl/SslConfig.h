// SslConfig.h 
// wire 
// 2025.12.4
/* 
引入https使用OpenSSL库，添加加密层

关键类：

SslConfig：     存储SSL配置，包括证书和密钥文件路径
SslContext:     初始化和管理SSL上下文
SslConnection： 处理SSL连接，包括握手，数据加密、解密

SslConfig：     SSL配置封装类，存储SSL配置，包括证书和密钥文件路径
*/

#pragma once
#include "SslTypes.h"
#include <string>
#include <vector>

namespace ssl 
{

class SslConfig 
{
public:
    // 构造函数：采用默认配置
    SslConfig()
    : version_(SSLVersion::TLS_1_2)
    , cipherList_("HIGH:!aNULL:!MDS")
    , verifyClient_(false)
    , verifyDepth_(4)
    , sessionTimeout_(300)
    , sessionCacheSize_(20480L)
    {
    }
    
    ~SslConfig() = default;

    // set方法
    // 证书配置   
    void setCertificateFile(const std::string& certFile) { certFile_ = certFile; }
    void setPrivateKeyFile(const std::string& keyFile) { keyFile_ = keyFile; }
    void setCertificateChainFile(const std::string& chainFile) { chainFile_ = chainFile; }

    // 协议版本和加密套件配置
    void setProtocolVersion(SSLVersion version) { version_ = version; }
    void setCipherList(const std::string& cipherList) { cipherList_ = cipherList; }

    // 客户端验证配置
    void setVerifyClient(bool verify) { verifyClient_ = verify; }
    void setVerifyDepth(int depth) { verifyDepth_ = depth; }
    
    // 会话配置
    void setSessionTimeout(int seconds) { sessionTimeout_ = seconds; }
    void setSessionCacheSize(long size) { sessionCacheSize_ = size; }


    // get方法
    const std::string& getCertificateFile() const { return certFile_; }
    const std::string& getPrivateKeyFile() const  { return keyFile_; }
    const std::string& getCertificateChainFile() const { return chainFile_; }
    SSLVersion getProtocolVersion() const { return version_; }
    const std::string& getCipherList() const { return cipherList_; }
    bool getVerifyClient() const { return verifyClient_; }
    int getVerifyDepth() const { return verifyDepth_; }
    int getSessionTimeout() const { return sessionTimeout_; }
    long getSessionCacheSize() const { return sessionCacheSize_; }


private:
    std::string certFile_;              // 证书文件
    std::string keyFile_;               // 证书私钥文件
    std::string chainFile_;             // 证书链文件
    SSLVersion  version_;               // 协议版本
    std::string cipherList_;            // 加密套件
    bool        verifyClient_;          // 是否验证客户端
    int         verifyDepth_;           // 验证深度
    int         sessionTimeout_;        // 会话超时时间
    long        sessionCacheSize_;      // 会话缓存大小
};

} // namespace ssl


