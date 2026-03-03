#include "../../include/router/Router.h"
#include <muduo/base/Logging.h>

namespace http
{
namespace router
{


   // 注册对象式的静态路由处理器（本质在哈希表中添加）
    void Router::registerHandler(HttpRequest::Method method, const std::string &path, HandlerPtr handler){
        RouteKey key;
        key.method = method;
        key.path = path;
        handlers_[key]=std::move(handler); // hander 是一个<share_ptr>指针
    }

    // 注册函数型的静态路由处理器（本质在哈希表中添加）
    void Router::registerCallback(HttpRequest::Method method, const std::string &path, const HandlerCallback &callback){
        RouteKey key;
        key.method = method;
        key.path = path;
        callbacks_[key]=std::move(callback); //  callbacks是一个函数

    }

    bool Router::route(const HttpRequest &req, HttpResponse &resp){
        
        // 静态路由
        // 查哈希表
        RouteKey key{req.getMethod(), req.getPath()};
        // 查找对象型哈希表
        auto handlerIt=handlers_.find(key);
        if(handlerIt!=handlers_.end())
        {
            handlerIt->second->handle(req,resp); // 执行类对象中的回调函数
            return true;
        }
        // 查找函数型哈希表
        auto callbackIt=callbacks_.find(key);
        if(callbackIt!=callbacks_.end())
        {
            callbackIt->second(req,resp); // 执行类对象中的回调函数
            return true;
        }

        // 动态路由
        for(const auto regexHandler : regexHandlers_){
            std::smatch match; // 存储匹配结果
            std::string pathStr(req.getPath());
            if( regexHandler.method_==req.getMethod() && std::regex_match(pathStr,match, regexHandler.pathRegex_)) //方法和正则路径均匹配
            {
                // 额外将PathParameters传递给req，因为setPathParameters在HttpContext没有解析
                HttpRequest newreq(req);
                extractPathParameters(match,newreq); // match中存储匹配结果
                regexHandler.handler_->handle(newreq,resp);
                return true;
            }
        }

        for(const auto& regexCallback : regexCallbacks_){
        std::smatch match; // 存储匹配结果
        std::string pathStr(req.getPath());
        if(regexCallback.method_==req.getMethod() && std::regex_match(pathStr,match,regexCallback.pathRegex_)) //方法和正则路径均匹配
            {
                // 额外将PathParameters传递给req，因为setPathParameters在HttpContext没有解析
                HttpRequest newreq(req);
                extractPathParameters(match,newreq); // match中存储匹配结果
                regexCallback.callback_(newreq,resp);
                return true;
            }
        }

        return false;
    }


} // namespace router
} // namespace http