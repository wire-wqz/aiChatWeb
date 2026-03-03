// DbConnectionPool.h 
// wire 
// 2025.12.3
/* 

数据库连接池模块，在程序启动时建立使用同一个mysql驱动（MySQL_Driver* driver）创建足够的数据库连接，并将这些连接组成一个连接池；
需要执行数据库操作时，从连接池中中取出现有的连接使用，执行完sql后，将连接放回连接池中。

三个核心类：
DbConnection:       管理单个数据库连接
DbConnectionPool:   管理连接池
MysqlUtil:          便捷的数据库操作接口

DbConnectionPool:   连接池
*/

#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <thread>
#include "DbConnection.h"

namespace http 
{
namespace db 
{

class DbConnectionPool 
{

public:
    // 单例模式(先getInstance，然后对该实例处理)，有且仅有一个实例即 static DbConnectionPool instance;
    static DbConnectionPool& getInstance() 
    {
        static DbConnectionPool instance;
        return instance;
    }

    // 初始化连接池
    void init(const std::string& host,
             const std::string& user,
             const std::string& password,
             const std::string& database,
             size_t poolSize = 10);

    // 获取连接
    std::shared_ptr<DbConnection> getConnection();

private:
    // 构造函数
    DbConnectionPool();
    // 析构函数
    ~DbConnectionPool();

    // 禁止拷贝
    DbConnectionPool(const DbConnectionPool&) = delete;
    DbConnectionPool& operator=(const DbConnectionPool&) = delete;

    // 创建连接
    std::shared_ptr<DbConnection> createConnection();
    
    // 连接检查方法
    void checkConnections(); 


private:
    std::string                               host_;
    std::string                               user_;
    std::string                               password_;
    std::string                               database_;
    std::queue<std::shared_ptr<DbConnection>> connections_;                    // 连接池 
    std::mutex                                mutex_;
    std::condition_variable                   cv_;
    bool                                      initialized_ = false;
    std::thread                               checkThread_;                    // 添加检查线程
};



} // namespace db
} // namespace http
