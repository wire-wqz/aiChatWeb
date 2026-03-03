// MysqlUtil.h 
// wire 
// 2025.12.3
/* 

数据库连接池模块，在程序启动时建立使用同一个mysql驱动（MySQL_Driver* driver）创建足够的数据库连接，并将这些连接组成一个连接池；
需要执行数据库操作时，从连接池中中取出现有的连接使用，执行完sql后，将连接放回连接池中。

三个核心类：
DbConnection:       管理单个数据库连接
DbConnectionPool:   管理连接池
MysqlUtil:          便捷的数据库操作接口

MysqlUtil:   负责连接池初始化，与mysql执行executeQuery和executeUpdate，避免每次操作都要从连接池取出一个连接，该工具集成上述操作，通过static通过类名访问避免创建对象实例
*/

/*
工具类使用demo

// 初始化
http::MysqlUtil::init(host,user,password,database,poolSize); // 静态变量通过类名访问

// executeQuery
std::unique_ptr<sql::ResultSet> res(http::MysqlUtil::executeQuery(sql,args));

*/




#pragma once
#include "db/DbConnectionPool.h"
namespace http
{

class MysqlUtil
{
public:
    // 静态成员变量类内声明，类外初始化，静态成员函数不是,别混淆
    static void init(const std::string& host, const std::string& user,
                    const std::string& password, const std::string& database,
                    size_t poolSize = 10)
    {
        http::db::DbConnectionPool::getInstance().init(host, user, password, database, poolSize);
    }

    template<typename... Args>
    static sql::ResultSet* executeQuery(const std::string& sql, Args&&... args)
    {
        // 从连接池获取连接
        std::shared_ptr<http::db::DbConnection> dbconn(http::db::DbConnectionPool::getInstance().getConnection());
        // 利用连接与mysql交互
        return dbconn->executeQuery(sql, std::forward<Args>(args)...);
    }

    template<typename... Args>
    static int executeUpdate(const std::string& sql, Args&&... args)
    {
        std::shared_ptr<http::db::DbConnection> dbconn(http::db::DbConnectionPool::getInstance().getConnection());
        return dbconn->executeUpdate(sql, std::forward<Args>(args)...);
    }

    static std::shared_ptr<http::db::DbConnection>  getDbConn()
    {
        std::shared_ptr<http::db::DbConnection> dbconn(http::db::DbConnectionPool::getInstance().getConnection());
        return dbconn;
    }
};

}