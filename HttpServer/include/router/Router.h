// Router.h 
// wire 
// 2025.11.27
/* 
路由模块，负责将URL 映射到 具体的回调函数 ， 根据回调函数处理请求报文封装响应报文

分为静态路由（绝对匹配）和 动态路由（正则化模糊匹配）
静态路由的局限性：显示图片等主页后续资源，每一个资源需要额外注册；
因此采用动态路由处理，例如m_server.addRoute("GET", "/images/.*", handleStaticFile); 

可以   注册对象式的路由处理器（复杂处理）：HandlerPtr（共享智能指针）
也可以 注册回调函数式的处理器（简单处理）：HandlerCallback

关键：
注册路由 一系列register()
使用路由 router()
*/

#pragma once
#include <iostream>
#include <unordered_map>
#include <string>
#include <memory>
#include <functional>
#include <regex>
#include <vector>

#include "RouterHandler.h"
#include "../http/HttpRequest.h"
#include "../http/HttpResponse.h"

namespace http
{
namespace router
{
class Router{

public:
    // 路由值（两种：注册为对象/注册为函数）
    using HandlerPtr = std::shared_ptr<RouterHandler>;                                         // 路由处理器对象，内部方法是一个路由回调函数（注册为对象式的路由处理器）
    using HandlerCallback = std::function<void(const HttpRequest & resq, HttpResponse & res)>; // 具体的路由回调函数（注册为回调函数式的路由处理器）

    // 路由键（请求方法 + URI）
    struct RouteKey // 自定结构体
    {
        HttpRequest::Method method;
        std::string path;
        
        // 重写RouteKey的==
        bool operator==(const RouteKey &other) const
        {
            return method == other.method && path == other.path;
        }
    };

    // 为RouteKey 定义哈希函数
    struct RouteKeyHash // 自定类型需要自定哈希函数
    {
        size_t operator()(const RouteKey &key) const
        {
            size_t methodHash = std::hash<int>{}(static_cast<int>(key.method));
            size_t pathHash = std::hash<std::string>{}(key.path);
            return methodHash * 31 + pathHash;
        }
    };

    // 注册对象式的静态路由处理器（本质在哈希表中添加）
    void registerHandler(HttpRequest::Method method, const std::string &path, HandlerPtr handler);

    // 注册函数型的静态路由处理器（本质在哈希表中添加）
    void registerCallback(HttpRequest::Method method, const std::string &path, const HandlerCallback &callback);

    
    // 注册动态路由处理器(本质是添加vector)
    void addRegexHandler(HttpRequest::Method method, const std::string &path, HandlerPtr handler)
    {
        std::regex pathRegex = convertToRegex(path); // 路径进行正则化
        regexHandlers_.emplace_back(method, pathRegex, handler);
    }
    
    void addRegexCallback(HttpRequest::Method method, const std::string &path, const HandlerCallback &callback)
    {
        std::regex pathRegex = convertToRegex(path);
        regexCallbacks_.emplace_back(method, pathRegex, callback);
    }
    
    
    // 处理router（根据HTTP请求的方法和路径，找到对应的处理函数并执行）
    bool route(const HttpRequest &req, HttpResponse &resp);


private:
    // 输入：/api/:version/item/:itemId
    // 输出：^/api/([^/]+)/item/([^/]+)$
    // 正则化规则1：/css/:xxx 将：后的一项任意匹配 （不能匹配到css/1/base.css 只能匹配到css/base.css）
    // 正则化规则1：/css/.*   将.*后的无数项任意匹配 （能匹配到css/base.css）
    std::regex convertToRegex(const std::string &pathPattern)
    { // 将路径模式转换为正则表达式，支持匹配任意路径参数
        std::string regexPattern = "^" + std::regex_replace(pathPattern, std::regex(R"(/:([^/]+))"), R"(/([^/]+))") + "$";
        return std::regex(regexPattern);
    }

    
    // 提取路径参数
    // 将正则表达式匹配到的路径段（如用户ID、文章标题等）提取出来，存储到请求对象中
    void extractPathParameters(const std::smatch &match, HttpRequest &request)
    {
        // Assuming the first match is the full path, parameters start from index 1
        for (size_t i = 1; i < match.size(); ++i)
        {
            request.setPathParameters("param" + std::to_string(i), match[i].str()); // setPathParameters在HttpContext没有解析，需要用到时要重新添加
        }
    }

private:
    // 动态路由对象
    struct RouteHandlerObj
    {
        HttpRequest::Method method_;
        std::regex pathRegex_;  // 正则化规则后的路径
        HandlerPtr handler_;    // 路由处理器对象
        RouteHandlerObj(HttpRequest::Method method, std::regex pathRegex, HandlerPtr handler)
            : method_(method), pathRegex_(pathRegex), handler_(handler) {}
    };

    struct RouteCallbackObj
    {
        HttpRequest::Method method_;
        std::regex pathRegex_;
        HandlerCallback callback_;
        RouteCallbackObj(HttpRequest::Method method, std::regex pathRegex, const HandlerCallback &callback)
            : method_(method), pathRegex_(pathRegex), callback_(callback) {}
    };


private:
    std::unordered_map<RouteKey, HandlerPtr, RouteKeyHash>      handlers_;       // 静态匹配的哈希表（key为方法和路径 value为路由处理器对象）表示注册为一个对象
    std::unordered_map<RouteKey, HandlerCallback, RouteKeyHash> callbacks_;      // 精准匹配的哈希表（key为方法和路径 value为路由回调函数）  表示注册为一个回调函数

    std::vector<RouteHandlerObj>                                regexHandlers_;    // 动态路由（对象型） 正则匹配 （为什么用vectoer，因为动态路由的正则匹配具有优先级）
    std::vector<RouteCallbackObj>                               regexCallbacks_;   // 动态路由（函数型） 正则匹配
};

} // namespace router
} // namespace http
