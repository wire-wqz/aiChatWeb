// Middleware.h 
// wire 
// 2025.12.2
/* 
HTTP中间件是一种处理HTTP请求和响应的函数/组件。
它在客户端请求到达服务器处理逻辑（路由）之前，或者在服务器响应返回客户端之前（conn->send()）


Middleware是一个虚拟类，是中间类的基类接口
*/


#pragma once

#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"

namespace http
{
namespace middleware
{

class Middleware{

public:
virtual ~Middleware() = default;

// 请求前处理
virtual void before(HttpRequest& request) = 0;
    
// 响应后处理
virtual void after(HttpResponse& response) = 0;
    
// 设置下一个中间件（可以不使用 依赖MiddlewareChain管理）
void setNext(std::shared_ptr<Middleware> next) 
{
    nextMiddleware_ = next;
}

protected: // 派生类可访问
    std::shared_ptr<Middleware> nextMiddleware_;

};


} // namespace middleware
} // namespace http