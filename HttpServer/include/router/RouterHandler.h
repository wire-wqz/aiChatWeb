// 虚拟类
// 路由处理器（路由回调函数）
// Router 提供了一个模板类，通过继承的方式实现路由处理类
// 通常会将RouterHandler视为友元，并传入服务器类指针



#pragma once
#include <string>
#include <memory>
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"

namespace http
{
namespace router
{

class RouterHandler 
{
public:
    virtual ~RouterHandler() = default;
    virtual void handle(const HttpRequest& req, HttpResponse& resp) = 0;
};

} // namespace router
} // namespace http