// HttpContext.h 
// wire 
// 2025.11.11
/* 
HttpContext类用于解析客户端报文，并将消息封装到HttpRequest中，基于HTTP请求解析状态机实现；
在HttpServer中基于muduo库建立回调函数，实现报文的接收并使用HttpContext解析；
bool parseSuccess = context.parseRequest(&inputBuffer); // 处理报文封装到HttpRequest
HttpRequest request = context.request();                // 获取封装好的HttpRequest
*/

#pragma once
#include <iostream>
#include <sstream>
#include <muduo/net/TcpServer.h>
#include "HttpRequest.h"

namespace http
{

class HttpContext {
public:
    enum HttpRequestParseState{
        kExpectRequestLine,     // 解析请求行
        kExpectHeaders,         // 解析请求头
        kExpectBody,            // 解析请求体
        kGotAll,                // 解析完成
    };

    // 构造函数
    HttpContext(): state_(kExpectRequestLine){}

    // 解析报文（传入muduo缓冲区报文buf，时间戳receiveTime）
    bool parseRequest(muduo::net::Buffer* buf, muduo::Timestamp receiveTime);

    // 是否完成解析
    bool gotAll() const 
    { return state_ == kGotAll;  }

    // 重置，进行下一次解析
    void reset()
    {
        state_ = kExpectRequestLine;
        HttpRequest dummyData;
        request_.swap(dummyData);
    }

    // 返回封装好的request类 get （两种返回模式）
    const HttpRequest& getRequest() const
    { return request_;}

    HttpRequest& getRequest()
    { return request_;}

private:
    bool processRequestLine(const char* begin, const char* end);
    HttpRequestParseState hasbody();

private:
    HttpRequestParseState state_;          // 状态
    HttpRequest           request_;        // 请求HttpRequest类，接收报文封装
};


}// namespace http