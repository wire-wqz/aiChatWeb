// SessionManager.h 
// wire 
// 2025.12.1
/* 


SessionManager类负责创建、检索、销毁会话、还负责会话存储

SessionManager​ 通过 SessionStorage​ 加载/保存 Session
Session​ 持有 SessionManager​ 的引用以便自我管理
MemorySessionStorage​ 在内存中维护 Session​ 对象的映射表



使用：
SessionManager sessionmanager_;
sessionmanager_.getSession();   如果请求的Cookie中包含会话ID，则从存储中加载会话，否则创建一个新的会话)，随后下发身份凭证给客户端
*/

#include "SessionStorage.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"
#include <memory>
#include <random>


namespace http
{
namespace session
{

class SessionManager
{

public:
    explicit SessionManager(std::unique_ptr<SessionStorage> storage,int maxAge=3600);  // explicit 禁止隐式类型转换

    // 从请求中获取或创建会话
    // 如果请求的Cookie中包含会话ID，则从存储中加载会话，否则创建一个新的会话)，随后下发身份凭证给客户端
    std::shared_ptr<Session> getSession(const HttpRequest& req, HttpResponse& resp);
    
    // 销毁会话
    void destroySession(const std::string& sessionId);

    // 清理过期会话
    void cleanExpiredSessions();

    // 返回所有过期会话（仍存储在内存中）
    std::vector<std::shared_ptr<Session>> getExpiredSessions();

    // 某个sessionId 是否存活(存在且没有过期)
    bool isValid(const std::string& sessionId);

    // 更新会话
    void updateSession(std::shared_ptr<Session> session)
    {
        storage_->save(session);
    }

private:
    // 生成随机会话id
    std::string generateSessionId();           
    // 读取客户端身份
    std::string getSessionIdFromCookie(const HttpRequest& req);
    // 下发身份凭证
    void setSessionCookie(const std::string& sessionId, HttpResponse& resp);

private:
    std::unique_ptr<SessionStorage> storage_;             // SessionStorage类指针，用于存储Session
    std::mt19937 rng_;                                    // 用于生成随机会话id
    int  maxAge_;                                         // 过期时间（秒）
};

} // namespace session
} // namespace http