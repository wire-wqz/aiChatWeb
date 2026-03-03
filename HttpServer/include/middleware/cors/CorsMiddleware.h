// CorsMiddleware.h 
// wire 
// 2025.12.2
/* 

跨域中间件，主机访问的服务器不在本地而是去访问其它域名的服务器。
跨域中间件的作用就说给服务器设置，哪些域名的客户端可以访问服务器，哪些方法可以跨域请求，包含什么样的头文件跨域跨域。

例如：
前端应用：运行在 http://localhost:8080（您的本地开发服务器）。
后端API：运行在 http://localhost:3000（另一个本地服务器）。
后端API需要添加cors中间件，并设置允许访问的规则，。当前端调用后端API时，才允许访问

CorsMiddleware 为具体的跨域中间件（这里是一个简单的demo，只会对origin判断是否在规则内）

服务器接收的预检请求：
OPTIONS /api/data HTTP/1.1
Host: api.example.com
Origin: http://localhost:8080
Access-Control-Request-Method: POST
Access-Control-Request-Headers: Content-Type, Authorization
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36
会包含实际请求的方法和头部，服务器根据响应判断Origin是否被允许，发送给客户端，同时发送的还有允许的方法和头部，以及预检缓存时间，在该时间内无需预检

果预检响应未通过检查，浏览器会立即抛出一个 CORS 错误，并且根本不会发送后面的实际请求。

*/

#pragma once 
#include "../Middleware.h"
#include "../../http/HttpRequest.h"
#include "../../http/HttpResponse.h"
#include "CorsConfig.h"

namespace http 
{
namespace middleware 
{
// 继承Middleware
class CorsMiddleware : public Middleware 
{
public:
    explicit CorsMiddleware(const CorsConfig& config = CorsConfig::defaultConfig());

    // 请求前处理
    void before(HttpRequest& request) override;
    
    // 响应后处理
    void after(HttpResponse& response) override;

    // vector<std::string中的所有元素用指定的分隔符连接成一个字符串string
    std::string join(const std::vector<std::string>& strings, const std::string& delimiter);

private:

    // 验证请求的来源(origin)是否在允许访问的域名列表中
    bool isOriginAllowed(const std::string& origin) const;
    
    // 专门处理浏览器的预检请求(OPTIONS请求)
    void handlePreflightRequest(const HttpRequest& request, HttpResponse& response);
    
    // 在响应中添加CORS头部
    void addCorsHeaders(HttpResponse& response, const std::string& origin);


private:
    CorsConfig config_;
};

}   // namespace middleware 
}   // namespace http 