// CorsConfig.h 
// wire 
// 2025.12.2
/* 

跨域中间件，主机访问的服务器不在本地而是去访问其它域名的服务器。
跨域中间件的作用就说给服务器设置，哪些域名的客户端可以访问服务器，哪些方法可以跨域请求，包含什么样的头文件跨域跨域。

例如：
前端应用：运行在 http://localhost:8080（您的本地开发服务器）。
后端API：运行在 http://localhost:3000（另一个本地服务器）。
后端API需要添加cors中间件，并设置允许访问的规则，。当前端调用后端API时，才允许访问

CorsConfig中设置了规则
*/


#pragma once

#include <string>
#include <vector>

namespace http 
{
namespace middleware 
{

struct CorsConfig 
{
    std::vector<std::string> allowedOrigins;
    std::vector<std::string> allowedMethods;
    std::vector<std::string> allowedHeaders;
    bool allowCredentials = false;
    int maxAge = 3600;
    
    static CorsConfig defaultConfig() 
    {
        CorsConfig config;
        config.allowedOrigins = {"*"};
        config.allowedMethods = {"GET", "POST", "PUT", "DELETE", "OPTIONS"};
        config.allowedHeaders = {"Content-Type", "Authorization"};
        return config;
    }
};

} // namespace middleware
} // namespace http