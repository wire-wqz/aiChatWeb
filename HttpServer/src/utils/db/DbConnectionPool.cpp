#include "../../../include/utils/db/DbConnectionPool.h"
#include "../../../include/utils/db/DbException.h"
#include <muduo/base/Logging.h>


namespace http 
{
namespace db 
{

// 构造函数（创建一个协程，检查连接池checkConnections）
DbConnectionPool::DbConnectionPool() 
{
    checkThread_ = std::thread(&DbConnectionPool::checkConnections, this);
    checkThread_.detach();
}

// 析构函数（销毁所有连接）
DbConnectionPool::~DbConnectionPool() 
{
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connections_.empty()) 
    {
        connections_.pop();
    }
    LOG_INFO << "Database connection pool destroyed";
}


// 初始化连接池
void DbConnectionPool::init(const std::string& host,
                          const std::string& user,
                          const std::string& password,
                          const std::string& database,
                          size_t poolSize) 
{
    std::lock_guard<std::mutex> lock(mutex_);
    // 确保仅初始化一次
    if (initialized_) 
    {
        return;
    }

    host_ = host;
    user_ = user;
    password_ = password;
    database_ = database;

    for (size_t i = 0; i < poolSize; ++i) 
    {
        connections_.push(createConnection());   // 向连接池（队列中）添加连接
    }

    initialized_ = true;
    LOG_INFO << "Database connection pool initialized with " << poolSize << " connections";
}

// 获得一个连接
std::shared_ptr<DbConnection> DbConnectionPool::getConnection() 
{
    std::shared_ptr<DbConnection> conn;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // 若池内为空则等待
        while (connections_.empty()) 
        {
            if (!initialized_) 
            {
                throw DbException("Connection pool not initialized");
            }
            LOG_INFO << "Waiting for available connection...";
            cv_.wait(lock);                      // 释放锁并等待
        }

        // 池内不为空获取队头连接
        conn = connections_.front();
        connections_.pop();
    } // 释放锁

    try 
    {
        // 在锁外检查连接
        if (!conn->isValid()) 
        {
            LOG_WARN << "Connection lost, attempting to reconnect...";
            conn->reconnect();
        }
        
        // 返回连接，它通过自定义 std::shared_ptr的删除器，使得当连接不再被需要时，能够自动将其归还到连接池中，而不是直接销毁
        // 具体地，一个新建立的shared_ptr指向conn.get()；并定义了删除器，当shared_ptr被删除则归还到内存
        return std::shared_ptr<DbConnection>(conn.get(), 
            [this, conn](DbConnection*) {
                std::lock_guard<std::mutex> lock(mutex_);
                connections_.push(conn);
                cv_.notify_one();
            });
    } 
    catch (const std::exception& e) 
    {
        LOG_ERROR << "Failed to get connection: " << e.what();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            connections_.push(conn);
            cv_.notify_one();
        }
        throw;
    }
}

// 创建连接
std::shared_ptr<DbConnection> DbConnectionPool::createConnection(){
    return std::make_shared<DbConnection>(host_, user_, password_, database_);
}

// 检查连接池的函数（在协程内不断执行）
void DbConnectionPool::checkConnections() 
{
    while (true) 
    {
        try 
        {
            std::vector<std::shared_ptr<DbConnection>> connsToCheck;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                if (connections_.empty()) 
                {
                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    continue;
                }
                
                auto temp = connections_;
                while (!temp.empty()) 
                {
                    connsToCheck.push_back(temp.front());
                    temp.pop();
                }
            }
            
            // 在锁外检查连接
            for (auto& conn : connsToCheck) 
            {
                if (!conn->isValid()) 
                {
                    try 
                    {
                        conn->reconnect();
                    } 
                    catch (const std::exception& e) 
                    {
                        LOG_ERROR << "Failed to reconnect: " << e.what();
                    }
                }
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(60));
        } 
        catch (const std::exception& e) 
        {
            LOG_ERROR << "Error in check thread: " << e.what();
            std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }
}





} // namespace db
} // namespace http