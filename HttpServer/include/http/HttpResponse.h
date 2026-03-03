// HttpResponse.h 
// wire 
// 2025.11.25
/* 
HttpResponse 是HTTP响应报文封装类
*/



#pragma once

#include <muduo/net/TcpServer.h>

namespace http
{
class HttpResponse {

public:

    enum HttpStatusCode  // HTTP 状态码
    {
        kUnknown,                       // 未知状态码
        k200Ok = 200,                   // 请求成功
        k204NoContent = 204,            // 请求成功，但响应中没有内容
        k301MovedPermanently = 301,     // 请求的资源已永久移动到新位置
        k400BadRequest = 400,           // 客户端请求错误，服务器无法理解
        k401Unauthorized = 401,         // 需要身份验证或认证失败
        k403Forbidden = 403,            // 服务器理解请求但拒绝执行
        k404NotFound = 404,             // 请求的资源不存在
        k409Conflict = 409,             // 请求与服务器当前状态冲突
        k500InternalServerError = 500,  // 服务器内部错误
    };

    // 构造函数
    HttpResponse(bool close = true): 
    statusCode_(kUnknown), 
    closeConnection_(close)
    {}

    // get函数

    HttpStatusCode getStatusCode() const { return statusCode_; }

    bool getCloseConnection() const{ return closeConnection_; }
  

    //set函数
    void setVersion(std::string version){ httpVersion_ = version; }
    void setStatusCode(HttpStatusCode code){ statusCode_ = code; }
    void setStatusMessage(const std::string message){ statusMessage_ = message;}
    void setStatusLine(const std::string& version, HttpStatusCode statusCode, const std::string& statusMessage); // 响应行(调用接口为版本，状态码/消息赋值)
    void setCloseConnection(bool on){ closeConnection_ = on; }
    void addHeader(const std::string& key, const std::string& value){ headers_[key] = value; }
    void setContentType(const std::string& contentType){ addHeader("Content-Type", contentType); }               // 响应头-响应体类型
    void setContentLength(uint64_t length) { addHeader("Content-Length", std::to_string(length)); }              // 响应头-响应体长度
    void setBody(const std::string& body) { body_ = body;}                                                       // 响应体
    void setErrorHeader(){}                                                                                      // 设置错误响应的 HTTP 头部信息 
    void appendToBuffer(muduo::net::Buffer* outputBuf) const;                                                    // 将 HTTP 响应内容序列化到输出缓冲区outputBuf
   



private:
    std::string                        httpVersion_;           // HTTP 版本，如 "HTTP/1.1" - 组成响应行
    HttpStatusCode                     statusCode_;            // 状态码，如 200、404 - 组成响应行  
    std::string                        statusMessage_;         // 状态消息，如 "OK"、"Not Found" - 组成响应行
    bool                               closeConnection_; 
    std::map<std::string, std::string> headers_;               // 响应头
    std::string                        body_;                  // 响应体
    bool                               isFile_;


};
} //namespace http