#include "../../include/ssl/SslContext.h"
#include <muduo/base/Logging.h>
#include <openssl/err.h>


namespace ssl
{

// 构造函数，为config_赋值
SslContext::SslContext(const SslConfig& config)
    : ctx_(nullptr)
    , config_(config)
{
}

// 析构函数，释放 CTX 对象
SslContext::~SslContext()
{
    if (ctx_)
    {
        SSL_CTX_free(ctx_); // 安全地释放 OpenSSL 的 SSL CTX 对象
    }
}

// 上下文初始化
bool SslContext::initialize()
{
    // 初始化 OpenSSL
    OPENSSL_init_ssl(OPENSSL_INIT_LOAD_SSL_STRINGS | 
                    OPENSSL_INIT_LOAD_CRYPTO_STRINGS, nullptr); 
    // OPENSSL_INIT_LOAD_SSL_STRINGS         加载 SSL 相关的错误信息字符串
    // OPENSSL_INIT_LOAD_CRYPTO_STRINGS      加载加密算法相关的错误信息字符串
        

    // 创建 SSL 上下文
    const SSL_METHOD* method = TLS_server_method();      // 专门用于创建 SSL/TLS 服务器 的配置
    ctx_ = SSL_CTX_new(method);
    if (!ctx_)
    {
        handleSslError("Failed to create SSL context");
        return false;
    }

    // 设置 SSL 选项
    long options = SSL_OP_NO_SSLv2 |                 // 禁用 SSL2.0 有严重安全漏洞，必须禁用   
                   SSL_OP_NO_SSLv3 |                 // 禁用 SSL3.0 易受 POODLE 攻击，应该禁用
                   SSL_OP_NO_COMPRESSION |           // 禁用压缩
                   SSL_OP_CIPHER_SERVER_PREFERENCE;  //  服务器优先选择密码套件
    SSL_CTX_set_options(ctx_, options);

    // 加载证书和私钥
    if (!loadCertificates())
    {
        return false;
    }

    // 设置协议版本
    if (!setupProtocol())
    {
        return false;
    }

    // 设置会话缓存
    setupSessionCache();

    LOG_INFO << "SSL context initialized successfully";
    return true;
}

// 加载证书 私钥 证书链
bool SslContext::loadCertificates()
{
    // 加载证书
    if (SSL_CTX_use_certificate_file(ctx_, config_.getCertificateFile().c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        handleSslError("Failed to load server certificate");
        return false;
    }

    // 加载私钥
    if (SSL_CTX_use_PrivateKey_file(ctx_, config_.getPrivateKeyFile().c_str(), SSL_FILETYPE_PEM) <= 0)
    {
        handleSslError("Failed to load private key");
        return false;
    }

    // 验证私钥
    if (!SSL_CTX_check_private_key(ctx_))
    {
        handleSslError("Private key does not match the certificate");
        return false;
    }

    // 加载证书链
    if (!config_.getCertificateChainFile().empty())
    {
        if (SSL_CTX_use_certificate_chain_file(ctx_,
            config_.getCertificateChainFile().c_str()) <= 0)
        {
            handleSslError("Failed to load certificate chain");
            return false;
        }
    }

    return true;
}

// 配置ssl协议参数
bool SslContext::setupProtocol(){
    // 设置 SSL/TLS 协议版本
    long options = SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3; // 总是禁用不安全的SSL
    switch (config_.getProtocolVersion())
    {
        case SSLVersion::TLS_1_0:  // 只允许TLS 1.0
        options |= SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_2 | SSL_OP_NO_TLSv1_3;
        break;
        
        case SSLVersion::TLS_1_1:  // 只允许TLS 1.1
        options |= SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_2 | SSL_OP_NO_TLSv1_3;
        break;
        
        case SSLVersion::TLS_1_2:  // 只允许TLS 1.2
        options |= SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_3;
        break;
        
        case SSLVersion::TLS_1_3:  // 只允许TLS 1.3
        options |= SSL_OP_NO_TLSv1 | SSL_OP_NO_TLSv1_1 | SSL_OP_NO_TLSv1_2;
        break;
    }
    SSL_CTX_set_options(ctx_, options);
    
    // 设置加密套件
    if (!config_.getCipherList().empty())
    {
        if (SSL_CTX_set_cipher_list(ctx_, config_.getCipherList().c_str()) <= 0)
        {
            handleSslError("Failed to set cipher list");
            return false;
        }
    }

    return true;
}


// 设置会话缓存
void SslContext::setupSessionCache()
{
    // 保存SSL/TLS会话信息，以便在客户端重新连接时能够快速恢复会话，避免完整的SSL握手过程。
    SSL_CTX_set_session_cache_mode(ctx_, SSL_SESS_CACHE_SERVER);           // 启用会话缓存
    SSL_CTX_sess_set_cache_size(ctx_, config_.getSessionCacheSize());      // 会话缓存大小
    SSL_CTX_set_timeout(ctx_, config_.getSessionTimeout());                // 会话超时时间
} 


// 错误处理
void SslContext::handleSslError(const char* msg)
{
    char buf[256];
    ERR_error_string_n(ERR_get_error(), buf, sizeof(buf));
    LOG_ERROR << msg << ": " << buf;
}

}