#include "../../include/http/HttpContext.h"

namespace http{

// 请求报文例子
// "POST /api/users HTTP/1.1\r\n"
// "Host: api.example.com\r\n"
// "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36\r\n"
// "Content-Type: application/json\r\n"
// "Content-Length: 76\r\n"
// "Accept: application/json\r\n"
// "Authorization: Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9\r\n"
// "Connection: keep-alive\r\n"
// "\r\n"   -- 请求体和请求头用crlf隔开
// "{\r\n"
// "  \"name\": \"张三\",\r\n"
// "  \"email\": \"zhangsan@example.com\",\r\n"
// "  \"age\": 25,\r\n"
// "  \"city\": \"北京\"\r\n"
// "}";

bool HttpContext::parseRequest(muduo::net::Buffer* buf, muduo::Timestamp receiveTime){ // 状态机
    bool ok;
    const char * crlf;
    
     // 处理请求行
    if(state_==kExpectRequestLine)
    {   
        crlf=buf->findCRLF();
        if(crlf)
        {
            ok = processRequestLine(buf->peek(), crlf); // 输入一行数据
            if(ok)
            {
                request_.setReceiveTime(receiveTime); // 更新时间戳
                buf->retrieveUntil(crlf+2);           // 移动到下一行
                state_=kExpectHeaders;                // 变更状态
            }
            else
            {
                std::cout << state_<<"格式有误"<< std::endl; 
                return false;
            }
        }
        else
        {    
            std::cout << state_<<"为空"<< std::endl; 
            return false;
        }
    }


    // 逐行处理请求头
    while(state_==kExpectHeaders){
        crlf=buf->findCRLF();
        if(crlf)
        {
            if (buf->peek()==crlf)        // 请求头结束
            {
                // 根据请求方法（post/put）和Content-Length判断是否需要继续读取body
                state_=hasbody();
                buf->retrieveUntil(crlf+2);// 进入下一行
            }
            else
            {
                ok = request_.setHeader(buf->peek(),crlf);   // 解析请求头
                if(ok) // 添加header
                {
                     buf->retrieveUntil(crlf+2);// 进入下一行
                }
                else
                {
                    std::cout << state_<<"格式有误"<< std::endl; 
                    return false;
                }
            }  
        }
        else
        {    
            std::cout << state_<<"为空"<< std::endl; 
            return false;
        }
    }
  
    // 处理请求体
    if(state_==kExpectBody)
    {
        // 判断请求体长度
        std::string contentLength = request_.getHeader("Content-Length");
        if(contentLength.empty())
        {
            std::cout << state_<<"为空"<< std::endl; 
            return false;
        }
        else
        {
            // 设置请求体长度
            request_.setContentLength(std::stoi(contentLength));
            if(request_.getContentLength()==0)
            {
                state_ = kGotAll;
            }
            else
            {   
                std::cout << buf->readableBytes()<< std::endl;
                 // 检查缓冲区中是否有足够的数据
                if (buf->readableBytes() < request_.getContentLength())
                {
                    std::cout<<"数据不完整"<< std::endl;
                    return true;  // 数据不完整，等待更多数据，下次进入state_依旧为kExpectBody，buf指向请求体头
                }

                // 只读取 Content-Length 指定的长度
                request_.setBody(buf->peek(),buf->peek()+request_.getContentLength());  //  这里不是逐行处理

                // 移动指针
                buf->retrieve(request_.getContentLength()); // buf->retrieveUntil->(buf->peek()+request_.getContentLength());

                state_ = kGotAll;
            }
        }
    }

  
    return true;
  
}


// 处理请求行
bool HttpContext::processRequestLine(const char* start,const char* end){
    // std::string method;
    // std::string url;
    // std::string version;
    // std::string strRequestLine(start,end);
    // std::istringstream request(strRequestLine);
    // request >> method >> url >> version;

    // GET /api/users/456/orders?status=shipped&page=2&limit=20&sort=date HTTP/1.1
    // method
    const char* begin=start;
    const char* spacepos = std::find(begin,end,' '); // 直接返回const char*
    if(spacepos==end)
        return false;
    if(request_.setMethod(begin,spacepos)==0)   
        return false;

    // url：?之前为path_，?之后为queryParameters_，但有可能不存在
    begin = spacepos+1;
    spacepos = std::find(begin,end,' '); 
    if(spacepos==end)
        return false;    
    const char* questpos = std::find(begin,spacepos,'?'); 
    if(questpos==spacepos) // 没有？，i.e. 没有查询参数
    {
        request_.setPath(begin,spacepos);
    }
    else
    {
        request_.setPath(begin,questpos);
        request_.setQueryParameters(questpos+1,spacepos);
    }

    // version
    begin = spacepos+1;
    std::string version(begin,end);
    if(version == "HTTP/1.1" || version == "HTTP/1.0")
        request_.setVersion(version);
    else    
        return false;

    // 成功解析
    return true;
}

// 判断是否有body
HttpContext::HttpRequestParseState HttpContext::hasbody(){
    if(request_.getMethod()==HttpRequest::kPost ||request_.getMethod()==HttpRequest::kPut)
        return kExpectBody;
    else
        return kGotAll;
}

} // namespace http