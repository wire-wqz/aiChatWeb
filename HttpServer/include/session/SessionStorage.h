// SessionStorage.h 
// wire 
// 2025.12.1
/* 

SessionStorage 定义了会话存储接口的抽象类

MemorySessionStorage SessionStorage的内存实现（存储在哈希表中，表名为随机的sessionId，内容为会话类std::shared_ptr<Session>）

在使用会话管理类SessionManager时需传入unique_ptr存储类指针
*/



#pragma once
#include "Session.h"
#include <memory>
#include <vector>

namespace http
{
namespace session
{

class SessionStorage
{
public:
    virtual ~SessionStorage() = default;
    
    // 保存或更新会话数据到存储中
    virtual void save(std::shared_ptr<Session> session) = 0;
    
    // 根据会话ID从存储中加载会话数据
    virtual std::shared_ptr<Session> load(const std::string& sessionId) = 0;

    // 从存储中删除指定的会话
    virtual void remove(const std::string& sessionId) = 0;

    // 清除过期session
    virtual void clean() = 0;

    // 返回过期会话
    virtual std::vector<std::shared_ptr<Session>> returnExpired() = 0;

};


// 基于内存的会话存储实现(在哈希表内save load remove)
class MemorySessionStorage : public SessionStorage
{
public:
    void save(std::shared_ptr<Session> session) override; // override是 C++11 引入的一个关键字， 它的作用是：显式声明重写
    std::shared_ptr<Session> load(const std::string& sessionId) override;
    void remove(const std::string& sessionId) override;
    void clean() override;
    std::vector<std::shared_ptr<Session>> returnExpired() override;
private:
    std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;  // 会话IDsessionId 与 会话session 的映射表
};






} //  namespace session
} // namespace http