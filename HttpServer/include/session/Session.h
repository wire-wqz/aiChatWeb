// Session.h 
// wire 
// 2025.12.1
/* 
HTTP本身没有提供任何记住客户端的途径；
因此，当服务端接收到客户端首次请求时，服务端初始化一个会话并分配该会话一个唯一的会话标识符sessionId；
在之后的请求中，客户端会将该唯一标识符包含在请求中，服务器根据此标识符分配对应的会话资源。

Session类表示一个用户会话，它会保存会话数据并管理会话存活周期
setValue() 会自动调用会话管理中updateSession方法，即 storage_->save(session); 

*/

#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <chrono>

namespace http
{
namespace session
{
    
class SessionManager;

class Session :  public std::enable_shared_from_this<Session>
{
public:
    Session(const std::string& sessionId, SessionManager* sessionManager, int maxAge = 3600); // 默认1小时过期

    // 返回sessionId
    const std::string& getId() const { return sessionId_; }

    // 刷新过期时间
    void refresh(); 

    // 判断是否过期（过期则返回true）
    bool isExpired() const;

    // 设置会话管理器
    void setManager(SessionManager* sessionManager) { sessionManager_ = sessionManager; }

    // 返回会话管理器
    SessionManager* getManager() const { return sessionManager_; }

    // 数据存取
    void setValue(const std::string&key, const std::string&value);
    std::string getValue(const std::string&key) const;                // 没有查询到则返回string::()
    void remove(const std::string&key);
    void clear();



private:
    std::string                                  sessionId_;                // 会话ID
    std::unordered_map<std::string, std::string> data_;                     // 存储session的值
    std::chrono::system_clock::time_point        expiryTime_;               // 过期时间
    int                                          maxAge_;                   // 过期时间（秒）
    SessionManager*                              sessionManager_;           // 会话管理类
};




} // namespace session
} // namespace http