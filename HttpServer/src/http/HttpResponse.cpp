#include "../../include/http/HttpResponse.h"

namespace http
{

void HttpResponse::setStatusLine(const std::string& version,
                                 HttpStatusCode statusCode,
                                 const std::string& statusMessage)
{
    HttpResponse::setVersion(version);
    HttpResponse::setStatusCode(statusCode);
    HttpResponse::setStatusMessage(statusMessage);
}


// 使用例子
// HttpResponse response;
// response封装...
// muduo::net::Buffer outputBuf;
// response.appendToBuffer(&outputBuf);
// conn->send(&outputBuf);
void HttpResponse::appendToBuffer(muduo::net::Buffer* outputBuf) const
{
    // 响应行 
    char buf[32]; // 格式化字符串
    snprintf(buf,sizeof buf,"%s %d ",httpVersion_.c_str(),statusCode_);
    outputBuf->append(buf);
    outputBuf->append(statusMessage_);
    outputBuf->append("\r\n");

    // 响应头

    // 根据连接是否关闭，动态添加 Connection 头部字段
    if (closeConnection_) 
    {
        outputBuf->append("Connection: close\r\n");
    }
    else
    {
        outputBuf->append("Connection: Keep-Alive\r\n");
    }

    for(const auto & header : headers_){
        outputBuf->append(header.first);
        outputBuf->append(": ");
        outputBuf->append(header.second);
        outputBuf->append("\r\n");
    }
    outputBuf->append("\r\n");

    // 响应体
    outputBuf->append(body_);
}


} // namespace http