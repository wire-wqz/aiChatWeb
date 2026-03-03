#include "../../include/ssl/SslConnection.h"
#include <muduo/base/Logging.h>
#include <openssl/err.h>



namespace ssl
{

// 构造函数 传输muduo conn 指针和上下文指针
SslConnection::SslConnection(const muduo::net::TcpConnectionPtr& conn, SslContext* ctx)
    : ssl_(nullptr)
    , ctx_(ctx)
    , conn_(conn)
    , state_(SSLState::HANDSHAKE)
    , readBio_(nullptr)
    , writeBio_(nullptr)
    , messageCallback_(nullptr)
    {
        // 创建 SSL 对象
        ssl_ = SSL_new(ctx_->getNativeHandle()); // ctx_->getNativeHandle() 返回上下文指针
        if (!ssl_) {
            LOG_ERROR << "Failed to create SSL object: " << ERR_error_string(ERR_get_error(), nullptr);
            return;
        }

        // 创建 BIO 是 OpenSSL 中对输入输出操作的封装。
        readBio_ = BIO_new(BIO_s_mem());
        writeBio_ = BIO_new(BIO_s_mem());
        if (!readBio_ || !writeBio_) {
            LOG_ERROR << "Failed to create BIO objects";
            SSL_free(ssl_);
            ssl_ = nullptr;
            return;
        }
        SSL_set_bio(ssl_, readBio_, writeBio_);

        // 设置 SSL 选项
        SSL_set_accept_state(ssl_);                               // 设置为服务器模式
        SSL_set_mode(ssl_, SSL_MODE_ACCEPT_MOVING_WRITE_BUFFER);  // 允许在 SSL 写入操作期间移动或重新分配写入缓冲区
        SSL_set_mode(ssl_, SSL_MODE_ENABLE_PARTIAL_WRITE);        // 启用部分写入
        
        // 覆盖连接回调
        conn_->setMessageCallback(
                    std::bind(&SslConnection::onRead, this, std::placeholders::_1,
                    std::placeholders::_2, std::placeholders::_3)
                );
    }

// 析构函数
SslConnection::~SslConnection() 
{
    if (ssl_) 
    {
        SSL_free(ssl_);  // 这会同时释放 BIO
    }
}

// ssl握手
void SslConnection::startHandshake()
{
    SSL_set_accept_state(ssl_);  // 设置为服务器模式 安全冗余
    handleHandshake();
}

// SSL 连接中应用层发送数据的入口（应用层的明文数据，加密后的数据通过 TCP 连接发送到网络）
void SslConnection::send(muduo::net::Buffer* plain)
{
    if (!ssl_ || !conn_) return;

    // 如果握手还没完成，可以拒绝或缓存（此处简单拒绝并记录）
    if (state_ != SSLState::ESTABLISHED) {
        LOG_WARN << "SSL not established, cannot send application data now";
        return;
    }

    size_t toWrite = plain->readableBytes();
    const char* data = plain->peek();
    size_t offset = 0;

    while (toWrite > 0) {
        int ret = SSL_write(ssl_, data + offset, static_cast<int>(toWrite));
        if (ret > 0) {
            offset += ret;
            toWrite -= ret;
            // 每次写后都把 writeBio_ 中的密文刷出去
            drainWriteBio();
            continue;
        }

        int err = SSL_get_error(ssl_, ret);
        if (err == SSL_ERROR_WANT_WRITE) {
            // 需要把 writeBio_ 中已有密文发送出去，剩余数据待底层可写时重试
            drainWriteBio();
            break;
        } else if (err == SSL_ERROR_WANT_READ) {
            // 需要先从对端读取数据（很少见），先把已有密文发出
            drainWriteBio();
            break;
        } else {
            // 致命错误
            char errBuf[256];
            unsigned long e = ERR_get_error();
            ERR_error_string_n(e, errBuf, sizeof(errBuf));
            LOG_ERROR << "SSL_write failed: " << errBuf;
            conn_->shutdown();
            return;
        }
    }

    // 从原缓冲区移除已写入的明文字节
    if (offset > 0) {
        plain->retrieve(static_cast<int>(offset));
    }
}

// sslconn的消息回调
void SslConnection::onRead(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp time)
{
    // 将收到的加密数据写入 OpenSSL 的 read BIO（无论当前状态）
    size_t n = buf->readableBytes();
    if (n == 0) return;

    const char* data = buf->peek();
    int written = BIO_write(readBio_, data, static_cast<int>(n));
    buf->retrieveAll(); // 已消费网络数据

    if (written <= 0) {
        LOG_ERROR << "BIO_write failed while feeding encrypted data";
        conn_->shutdown();
        return;
    }

    // 如果仍在握手阶段，继续握手并发送任何待发的握手加密数据
    if (state_ == SSLState::HANDSHAKE) {
        handleHandshake();
        drainWriteBio(); //  将writeBio_的数据发回对端
        return;
    }

    // 已完成握手：尝试从 SSL 中读取已解密的应用层数据
    for (;;) 
    {
        char appBuf[4096];
        int ret = SSL_read(ssl_, appBuf, sizeof(appBuf));
        if (ret > 0) 
        {
            // 转发解密后的数据到上层回调
            if (messageCallback_) 
            {
                muduo::net::Buffer message;
                message.append(appBuf, ret);
                messageCallback_(conn_, &message, time);
            }
            // 继续循环读取，直到 WANT_READ 或无更多应用数据
            continue;
        }

        int err = SSL_get_error(ssl_, ret);
        
        // 需要更多的加密字节，等待下次 onRead
        if (err == SSL_ERROR_WANT_READ) 
        { 
            break;
        } 

        // OpenSSL 要求写出加密字节（例如内部缓冲或重协商）
        else if (err == SSL_ERROR_WANT_WRITE) {
            drainWriteBio();
            break;
        } 
        // 对端正常关闭 TLS 会话
        else if (err == SSL_ERROR_ZERO_RETURN) {
            LOG_INFO << "SSL connection closed by peer";
            conn_->shutdown();
            break;
        } 
        else {
            // 致命错误
            char errBuf[256];
            unsigned long e = ERR_get_error();
            ERR_error_string_n(e, errBuf, sizeof(errBuf));
            LOG_ERROR << "SSL_read failed: " << errBuf;
            conn_->shutdown();
            break;
        }
    }

    // 在任何情况下，尝试将 writeBio_ 中的待发送字节写回对端
    while (BIO_pending(writeBio_) > 0) 
    {
        char outBuf[4096];
        int outLen = BIO_read(writeBio_, outBuf, sizeof(outBuf));
        if (outLen > 0) {
            conn_->send(outBuf, outLen);
        } else {
            break;
        }
    }
}

// 处理SSL/TLS握手过程
void SslConnection::handleHandshake()
{
     int ret = SSL_do_handshake(ssl_);
    
    if (ret == 1) {
        state_ = SSLState::ESTABLISHED;
        LOG_INFO << "SSL handshake completed successfully";
        LOG_INFO << "Using cipher: " << SSL_get_cipher(ssl_);
        LOG_INFO << "Protocol version: " << SSL_get_version(ssl_);
        
        // 握手完成后，确保设置了正确的回调（原消息回调的保存要在握手之前）
        if (!messageCallback_) 
        {
            LOG_WARN << "No message callback set after SSL handshake";
        }
        return;
    }
    
    // 错误处理
    int err = SSL_get_error(ssl_, ret);
    switch (err) {
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
            // 正常的握手过程，需要继续
            break;
            
        default: 
        {
            // 获取详细的错误信息
            char errBuf[256];
            unsigned long errCode = ERR_get_error();
            ERR_error_string_n(errCode, errBuf, sizeof(errBuf));
            LOG_ERROR << "SSL handshake failed: " << errBuf;
            conn_->shutdown();  // 关闭连接
            break;
        }
    }
}

// 把 write BIO 中的加密字节发送到底层连接
void SslConnection::drainWriteBio()
{
    if (!writeBio_ || !conn_) return;
    while (BIO_pending(writeBio_) > 0) 
    {
        char outBuf[4096];
        int outLen = BIO_read(writeBio_, outBuf, sizeof(outBuf));
        if (outLen > 0) 
        {
            conn_->send(outBuf, outLen);
        } 
        else 
        {
            break;
        }
    }
}

// 将OpenSSL错误码转换为自定义错误类型
SSLError SslConnection::getLastError(int ret) 
{
    int err = SSL_get_error(ssl_, ret);
    switch (err) 
    {
        case SSL_ERROR_NONE:
            return SSLError::NONE;
        case SSL_ERROR_WANT_READ:
            return SSLError::WANT_READ;
        case SSL_ERROR_WANT_WRITE:
            return SSLError::WANT_WRITE;
        case SSL_ERROR_SYSCALL:
            return SSLError::SYSCALL;
        case SSL_ERROR_SSL:
            return SSLError::SSL;
        default:
            return SSLError::UNKNOWN;
    }
}

// 错误处理
void SslConnection::handleError(SSLError error) 
{
    switch (error) 
    {
        case SSLError::WANT_READ:
        case SSLError::WANT_WRITE:
            // 需要等待更多数据或写入缓冲区可用
            break;
        case SSLError::SSL:
        case SSLError::SYSCALL:
        case SSLError::UNKNOWN:
            LOG_ERROR << "SSL error occurred: " << ERR_error_string(ERR_get_error(), nullptr);
            state_ = SSLState::ERROR;
            conn_->shutdown();
            break;
        default:
            break;
    }
}

} // namespace ssl