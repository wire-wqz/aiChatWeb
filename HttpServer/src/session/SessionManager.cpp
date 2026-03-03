#include"../include/session/SessionManager.h"
#include <iomanip>
#include <iostream>
#include <sstream>

namespace http
{
namespace session
{

SessionManager::SessionManager(std::unique_ptr<SessionStorage> storage,int maxAge)
    : storage_(std::move(storage)) 
    , rng_(std::random_device{}()) // 初始化随机数生成器，用于生成随机的会话ID
    , maxAge_(maxAge)
{}

// 从请求中获取或创建会话(如果请求中包含会话ID，则从存储中加载会话，否则创建一个新的会话)，随后下发身份凭证给客户端
std::shared_ptr<Session> SessionManager::getSession(const HttpRequest& req, HttpResponse& resp)
{
    // 从请求中的cookie获取sessionId
    std::string sessionId = getSessionIdFromCookie(req);

    std::shared_ptr<Session> session;

    if (!sessionId.empty()) // sessionId 存在
    {
        session = storage_->load(sessionId);
    }

    if (!session || session->isExpired()) // 不存在或过期则重新生成
    {
        sessionId = generateSessionId();
        session = std::make_shared<Session>(sessionId, this, maxAge_);
        setSessionCookie(sessionId, resp);
    }
    else 
    {
        session->setManager(this); // 为现有会话设置管理器
    }

    session->refresh();       // 刷新时间
    storage_->save(session);  // 这里可能有问题，需要确保正确保存会话
    return session;
}

// 销毁会话
void SessionManager::destroySession(const std::string& sessionId){
    storage_->remove(sessionId);

}


// 返回所有过期会话(还在内存中存储)
std::vector<std::shared_ptr<Session>> SessionManager::getExpiredSessions(){
    return storage_->returnExpired();
}

// 清理过期会话
void SessionManager::cleanExpiredSessions(){
    // 注意：这个实现依赖于具体的存储实现
    storage_->clean();
}

// 某个sessionId 是否存活(存在且没有过期)
bool SessionManager::isValid(const std::string& sessionId)
{
    std::shared_ptr<Session> session;
    if (sessionId.empty()) // sessionId 为空
    {
        return false;
    }

    session = storage_->load(sessionId); // load时也会清除过期会话(内存)
    if (!session || session->isExpired()) // 不存在或过期
    {
        return false;
    }
    return true;
}

// 生成随机会话id
std::string SessionManager::generateSessionId(){
    std::stringstream ss;
    std::uniform_int_distribution<> dist(0, 15);

    // 生成32个字符的会话ID，每个字符是一个十六进制数字
    for (int i = 0; i < 32; ++i)
    {
        ss << std::hex << dist(rng_);
    }
    return ss.str();
}     

// 读取客户端身份
// "Cookie: sessionId=abc123"
// "Cookie: sessionId=abc123; username=john; theme=dark"
std::string SessionManager::getSessionIdFromCookie(const HttpRequest& req){
    std::string sessionId;
    std::string cookie = req.getHeader("Cookie");

    if (!cookie.empty()){
        size_t pos = cookie.find("sessionId=");
        if (pos != std::string::npos)
        {
            pos += 10; // 跳过"sessionId="
            size_t end = cookie.find(';', pos);
            if (end != std::string::npos)
            {
                sessionId = cookie.substr(pos, end - pos);
            }
            else
            {
                sessionId = cookie.substr(pos);
            }
        }
    }
    return sessionId;
}


// 下发身份凭证
// Set-Cookie: sessionId=abc123xyz789; Path=/; HttpOnly
void SessionManager::setSessionCookie(const std::string& sessionId, HttpResponse& resp)
{
    // 设置会话ID到响应头中，作为Cookie
    std::string cookie = "sessionId=" + sessionId + "; Path=/; HttpOnly";
    resp.addHeader("Set-Cookie", cookie);
}

} // namespace session
} // namespace http
