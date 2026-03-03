#include "../../../include/utils/db/DbConnection.h"
#include "../../../include/utils/db/DbException.h"
#include <muduo/base/Logging.h>

namespace http 
{
namespace db 
{

DbConnection::DbConnection(const std::string& host, 
            const std::string& user,
            const std::string& password,
            const std::string& database)
            : host_(host)
            , user_(user)
            , password_(password)
            , database_(database)
{

    try
    {
        // 获取mysql驱动实例
        sql::mysql::MySQL_Driver * driver = sql::mysql::get_mysql_driver_instance();
        
        // 基于驱动实例为连接指针赋值，使用reset方法
        conn_.reset(std::move(driver->connect(host_, user_, password_)));

        // 连接成功
        if(conn_)
        {
            // 设置数据库
            conn_->setSchema(database_);

            // 设置连接属性
            conn_->setClientOption("OPT_RECONNECT", "true");           // 控制连接意外断开时是否自动重连
            conn_->setClientOption("OPT_CONNECT_TIMEOUT", "10");       // ​设置建立新连接时，客户端等待服务器响应的最长时间
            conn_->setClientOption("multi_statements", "false");       // ​不允许在一个语句中中执行多条由分号 (;) 分隔的 SQL 语句
            
            // 设置字符集
            std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
            stmt->execute("SET NAMES utf8mb4");
            
            LOG_INFO << "Database connection established";
        }
    }
    catch (const sql::SQLException& e) 
    {
        LOG_ERROR << "Failed to create database connection: " << e.what();
        throw DbException(e.what());
    } 
}

// 析构函数
DbConnection::~DbConnection() 
{
    try 
    {
        cleanup();
    } 
    catch (...) 
    {
        // 析构函数中不抛出异常
    }
    LOG_INFO << "Database connection closed";
}


// 连接状态检查（判断是否能通过一个简单的测试）
bool DbConnection::isValid() 
{
    try 
    {
        if (!conn_) return false;
        std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
        std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT 1")); 
        return (res != nullptr); 
    } 
    catch (const sql::SQLException&) 
    {
        return false;
    }
}


// 重新连接
void DbConnection::reconnect() 
{
    try 
    {
        if (conn_) 
        {
            conn_->reconnect();
        } 
        else 
        {
            sql::mysql::MySQL_Driver* driver = sql::mysql::get_mysql_driver_instance();
            conn_.reset(driver->connect(host_, user_, password_));
            conn_->setSchema(database_);
        }
    } 
    catch (const sql::SQLException& e) 
    {
        LOG_ERROR << "Reconnect failed: " << e.what();
        throw DbException(e.what());
    }
}

//  资源释放
void DbConnection::cleanup()
{
    std::lock_guard<std::mutex> lock(mutex_);
    try 
    {
        if (conn_) 
        {
            // 确保所有事务都已完成
            if (!conn_->getAutoCommit()) 
            {
                conn_->rollback();
                conn_->setAutoCommit(true);
            }
            
            // 清理所有未处理的结果集
            std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
            while (stmt->getMoreResults()) 
            {
                auto result = stmt->getResultSet();
                while (result && result->next()) 
                {
                    // 消费所有结果
                }
            }
        }
    } 
    catch (const std::exception& e) 
    {
        LOG_WARN << "Error cleaning up connection: " << e.what();
        try 
        {
            reconnect();  // 没有正确清理，重新连接
        } 
        catch (...) 
        {
            // 忽略重连错误
        }
    }
}



// 事务回滚
void DbConnection::rollback(){
    conn_->rollback();
    conn_->setAutoCommit(1);
}

// 事务提交
void DbConnection::commit(){
    conn_->commit();
    conn_->setAutoCommit(1);
}

// 设置事务自动提交状态
void DbConnection::setAutoCommit(bool isAutoCommit){
    conn_->setAutoCommit(isAutoCommit);
    if(!isAutoCommit) // 非自动提交
    {
        conn_->setTransactionIsolation(sql::TRANSACTION_READ_COMMITTED); // 设置事务隔离级别
    }
}



}  // namespace db 
}  // namespace http 