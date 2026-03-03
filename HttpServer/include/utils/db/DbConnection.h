// DbConnection.h 
// wire 
// 2025.12.3
/* 

数据库连接池模块，在程序启动时建立使用同一个mysql驱动（MySQL_Driver* driver）创建足够的数据库连接，并将这些连接组成一个连接池；
需要执行数据库操作时，从连接池中中取出现有的连接使用，执行完sql后，将连接放回连接池中。

三个核心类：
DbConnection:       管理单个数据库连接
DbConnectionPool:   管理连接池
MysqlUtil:          便捷的数据库操作接口

DbConnection:       管理单个数据库连接
*/


#pragma once
#include <memory>
#include <string>
#include <mutex>
#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <mysql_driver.h>
#include <mysql/mysql.h>
#include <muduo/base/Logging.h>
#include "DbException.h"
#include <type_traits>



namespace http 
{
namespace db 
{

class DbConnection {
public:

    // 构造函数（负责连接的建立，数据库和连接属性等选择）
    DbConnection(const std::string& host, 
                const std::string& user,
                const std::string& password,
                const std::string& database);
    
    // 析构函数
    ~DbConnection();

    // 禁止拷贝(取消了=和传入类对象的默认构造函数)
    DbConnection(const DbConnection&) = delete;
    DbConnection& operator=(const DbConnection&) = delete;

    // 连接状态检查（判断是否能通过一个简单的测试）
    bool isValid();
    
    // 连接恢复
    void reconnect();
    
    // 资源释放
    void cleanup();

    // 事务提交
    void commit();

    // 事务回滚
    void rollback();

    // 设置事务自动提交状态
    void setAutoCommit(bool isAutoCommit);

    // 查询sql语句（使用PreparedStatement和可变参数模板）
    template<typename...Args>
    sql::ResultSet* executeQuery(const std::string&sql,Args&&... args)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        try
        {
            // 创建PreparedStatement
            std::unique_ptr<sql::PreparedStatement> pstmt(conn_->prepareStatement(sql));
            // 利用递归将可变参数模板类型args与pstmt参数绑定
            bindParams(pstmt.get(), 1, std::forward<Args>(args)...);  // 智能指针的get方法返回裸指针
            return pstmt->executeQuery();
        }
        catch (const sql::SQLException& e) 
        {
            LOG_ERROR << "Query failed: " << e.what() << ", SQL: " << sql;
            throw DbException(e.what());
        }
    }

    // 处理更新语句
    template<typename... Args>
    int executeUpdate(const std::string& sql, Args&&... args)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        try 
        {
            // 直接创建新的预处理语句，不使用缓存
            std::unique_ptr<sql::PreparedStatement> pstmt(conn_->prepareStatement(sql));
            bindParams(pstmt.get(), 1, std::forward<Args>(args)...);  // 智能指针的get方法返回裸指针
            return pstmt->executeUpdate();
        } 
        catch (const sql::SQLException& e) 
        {
            LOG_ERROR << "Update failed: " << e.what() << ", SQL: " << sql;
            throw DbException(e.what());
        }
    }

private:
    // 递归终止函数：当参数包为空时调用
    void  bindParams(sql::PreparedStatement *,int){}

    // 递归函数： 处理第一个参数，参数包的剩余参数放入下一次迭代继续处理
    // 仅数值类型 (仅允许算术类型，如 int, long, double, bool)
    template<typename T, typename... Args>
    typename std::enable_if<std::is_arithmetic<typename std::decay<T>::type>::value>::type
    bindParams(sql::PreparedStatement* pstmt, int index, T&& value, Args&&... args) 
    {
        pstmt->setString(index, std::to_string(std::forward<T>(value)));
        bindParams(pstmt, index + 1, std::forward<Args>(args)...);
    }

    // 特化 string 类型的参数绑定
    template<typename... Args>
    void bindParams(sql::PreparedStatement* pstmt, int index, const std::string& value, Args&&... args) 
    {
        pstmt->setString(index, value);                                  // 处理第一个参数
        bindParams(pstmt, index + 1, std::forward<Args>(args)...);       // 参数包的剩余参数放入下一次迭代继续处理
    }

    // 特化 const char* 类型的参数绑定
    template<typename... Args>
    void bindParams(sql::PreparedStatement* pstmt, int index, const char*  value, Args&&... args) 
    {
        pstmt->setString(index, value);                                  // 处理第一个参数
        bindParams(pstmt, index + 1, std::forward<Args>(args)...);       // 参数包的剩余参数放入下一次迭代继续处理
    }





private:
    std::shared_ptr<sql::Connection> conn_;             // 连接对象指针
    std::string                      host_;             // 连接地址
    std::string                      user_;             // mysql用户
    std::string                      password_;         // mysql密码
    std::string                      database_;         // 连接的数据库
    std::mutex                       mutex_;            // 互斥锁（Mutex）
};


} // namespace db 
} // namespace http 