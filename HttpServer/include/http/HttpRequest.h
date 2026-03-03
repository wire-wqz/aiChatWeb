// HttpRequest.h 
// wire 
// 2025.11.11
/* 
HttpRequest类是一个HTTP请求的封装类，它的主要作用包括:
1、存储HTTP请求的所有组成部分：方法、路径、版本、头部、正文等
2、提供统一的接口来访问请求的各个部分
3、在接收到客户端请求后，基于HttpContext解析报文并将信息封装到HttpServer中
*/

#pragma once    // 确保同一个头文件在同一个编译单元中只被包含一次
#include <map>
#include <string>
#include <unordered_map>
#include <muduo/base/Timestamp.h>
#include<iostream>

namespace http
{

class HttpRequest
{
public:
    // HTTP请求报文方法
    enum Method 
    {
        kInvalid, kGet, kPost, kHead, kPut, kDelete, kOptions             // (枚举常量：k前缀)
    };

    // 构造函数
    HttpRequest(): method_(kInvalid), version_("Unknown"){}

    // 测试
    void mTest();
    
    // 封装成员变量set
    bool setMethod(const char* start, const char* end);                  // start指向方法字符串的起始位置，end指向字符串的结束位置
    void setVersion(std::string v){version_ = v;}
    void setReceiveTime(muduo::Timestamp t){receiveTime_ = t;}
    void setPath(const char* start, const char* end);
    void setPathParameters(const std::string &key, const std::string &value);
    void setQueryParameters(const char* start, const char* end);
    bool setHeader(const char* start, const char* end);
    void setBody(const std::string& body) { content_ = body; }
    void setBody(const char* start, const char* end);
    void setContentLength(uint64_t length){contentLength_ = length;}

    // 获取成员变量get（使用const 定义常量成员函数）
    Method getMethod() const { return method_;} 
    std::string getVersion() const {return version_;}
    std::string getPath() const { return path_;}
    std::string getPathParameters(const std::string &key) const;         // 声明
    std::string getQueryParameters(const std::string &key) const;        // 声明
    muduo::Timestamp getReceiveTime() const { return receiveTime_;}     
    std::string getHeader(const std::string& field) const;               // 声明 getHeader根据field字段名返回请求头的具体字段，函数内部是map的查询返回
    const std::map<std::string, std::string>& getHeaders() const{ return headers_; }
    std::string getBody() const{ return content_; }
    uint64_t getContentLength() const{ return contentLength_; }

    // 交换两个 HttpRequest对象的所有内容
    void swap(HttpRequest& that);
    

private: 
    Method                                       method_;                 // 请求方法(成员变量：_后缀)
    std::string                                  version_;                // http版本
    std::string                                  path_;                   // 请求路径            完整的URL路径
    std::unordered_map<std::string, std::string> pathParameters_;         // 路径参数（哈希表）   从路径中提取的变量键值对（用于动态路由，将正则化匹配的参数存储，key为param1 value为正则化匹配的url）
    std::unordered_map<std::string, std::string> queryParameters_;        // 查询参数（哈希表）   URL中?后面的键值对，用于过滤、排序、分页等操作参数
    muduo::Timestamp                             receiveTime_;            // 接收时间
    std::map<std::string, std::string>           headers_;                // 请求头（表）
    std::string                                  content_;                // 请求体
    uint64_t                                     contentLength_ { 0 };    // 请求体长度
};
} // namespace http