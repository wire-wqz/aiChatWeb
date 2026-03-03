#include "../include/session/SessionStorage.h"
#include <iostream>


namespace http
{
namespace session
{



// 保存或更新会话数据到存储中
void MemorySessionStorage::save(std::shared_ptr<Session> session){
    sessions_[session->getId()]=session;
}

// 根据会话ID从存储中加载会话数据
std::shared_ptr<Session> MemorySessionStorage::load(const std::string& sessionId){
    auto it = sessions_.find(sessionId);
    if(it != sessions_.end())
    {
        // 找到

        // if 会话没有过期则返回
        if (!it->second->isExpired())
        {
            return it->second;
        }
        else
        {
            // 如果会话已过期，则从存储中移除
            sessions_.erase(it);
        }
    }

    // 如果会话不存在或已过期，则返回nullptr
    return nullptr;
}


 // 从存储中删除指定的会话
void MemorySessionStorage::remove(const std::string& sessionId){
    sessions_.erase(sessionId);
}

// 返回所有存储的未过期会话id(遍历sessions_,并检查是否过期)
void MemorySessionStorage::clean(){
    std::vector<std::string> toRemove;
    for (auto& it : sessions_) // 这里it是取出的键值对,不是迭代器
    {
        if (it.second->isExpired())
        {
            // remove(it.first); // 在循环中删除当前的值,字段错误
            toRemove.push_back(it.first);
        }
    }
    for(const auto& removeId: toRemove)
    {
        sessions_.erase(removeId);
    }
}

// 返回所有过期会话(还在内存中存储)
std::vector<std::shared_ptr<Session>> MemorySessionStorage::returnExpired(){
    std::vector<std::shared_ptr<Session>> expiredSessions;
    for (auto& it : sessions_) // 这里it是取出的键值对,不是迭代器
    {
        if (it.second->isExpired())
        {
            // remove(it.first); // 在循环中删除当前的值,字段错误
            expiredSessions.push_back(it.second);
        }
    }
    return expiredSessions;
}

}
}