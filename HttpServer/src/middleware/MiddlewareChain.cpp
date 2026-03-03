#include "../../include/middleware/MiddlewareChain.h"
#include <muduo/base/Logging.h>

namespace http
{
namespace middleware
{


// 添加中间件到中间件链管理
void MiddlewareChain::addMiddleware(std::shared_ptr<Middleware> middleware){
    middlewares_.push_back(middleware);
}

// 使用存储的所有中间件依次处理请求(正向) 1 2 3
void MiddlewareChain::processBefore(HttpRequest& request){
    for(auto  &middleware : middlewares_)
    {
        middleware->before(request);
    }
}

// 使用存储的所有中间件依次处理响应（反向）3 2 1
void MiddlewareChain::processAfter(HttpResponse& response){
    try
    {
        // 反向处理响应，以保持中间件的正确执行顺序
        for (auto it = middlewares_.rbegin(); it != middlewares_.rend(); ++it)
        {
            if (*it)
            { // 添加空指针检查
                (*it)->after(response); // (*it) 要加()
            }
        }
    }
    catch (const std::exception &e)
    {
        LOG_ERROR << "Error in middleware after processing: " << e.what();
    }
}




} // namespace middleware
} // namespace http
