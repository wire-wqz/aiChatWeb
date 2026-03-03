// DbException.h 
// wire 
// 2025.12.3
/* 

数据库连接池模块，在程序启动时建立使用同一个mysql驱动（MySQL_Driver* driver）创建足够的数据库连接，并将这些连接组成一个连接池
需要执行数据库操作时，从连接池中中取出现有的连接使用，执行完sql后，将连接放回连接池中。

三个核心类：
DbConnection:       管理单个数据库连接
DbConnectionPool:   管理连接池
MysqlUtil:          便捷的数据库操作接口

DbException 用于抛出异常
*/

#pragma once
#include <stdexcept>
#include <string>

namespace http {
namespace db {

class DbException : public std::runtime_error 
{
public:
    explicit DbException(const std::string& message) 
        : std::runtime_error(message) {}
    
    explicit DbException(const char* message) 
        : std::runtime_error(message) {}
};

} // namespace db
} // namespace http