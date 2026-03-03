// Middleware.h 
// wire 
// 2025.12.2
/* 
HTTP中间件是一种处理HTTP请求和响应的函数/组件。
它在客户端请求到达服务器处理逻辑（路由）之前，或者在服务器响应返回客户端之前（conn->send()）


MiddlewareChain 中间件链管理，在一个vector中添加创建的中间件，并依次执行这些中间件
addMiddleware 向std::vector<std::shared_ptr<Middleware>> 添加中间件
processBefore 按照vector正向顺序 依次执行中间件before成员函数
processAfter  按照vector反向顺序 依次执行中间件afer成员函数

*/

#include <vector>
#include <memory>
#include "Middleware.h"

namespace http 
{
namespace middleware 
{

class MiddlewareChain 
{
public:
    // 添加中间件到中间件链管理
    void addMiddleware(std::shared_ptr<Middleware> middleware);
    
    // 使用存储的所有中间件依次处理请求
    void processBefore(HttpRequest& request);

    // 使用存储的所有中间件依次处理响应
    void processAfter(HttpResponse& response);

private:
    std::vector<std::shared_ptr<Middleware>> middlewares_;

};

} // namespace middleware 
} // namespace http 