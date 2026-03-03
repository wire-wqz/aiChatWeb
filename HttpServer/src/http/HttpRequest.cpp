# include "../../include/http/HttpRequest.h"

namespace http
{
    
void HttpRequest::mTest(){
    std::cout << "HttpRequest.cpp 已成功编译" << std::endl; 
}


// set 方法实现
bool HttpRequest::setMethod(const char* start, const char* end){
    assert(method_ == kInvalid);   // 断言method_ == kInvalid，否则失败；表示set方法应该仅能对刚初始化的对象赋值。       
    std::string m(start, end);     // [start, end)
    if (m == "GET")
    {
        method_ = kGet;
    }
    else if (m == "POST")
    {
        method_ = kPost;
    }
    else if (m == "PUT")
    {
        method_ = kPut;
    }
    else if (m == "DELETE")
    {
        method_ = kDelete;
    }
    else if (m == "OPTIONS")
    {
        method_ = kOptions;
    }
    else
    {
        method_ = kInvalid;
    }
    return method_ != kInvalid; // 方法有效则返回1
}

void HttpRequest::setPath(const char* start, const char* end){
    path_.assign(start, end);
}

void HttpRequest::setPathParameters(const std::string &key, const std::string &value){
    pathParameters_.emplace(key,value);// pathParameters_[key] = value;
}

void HttpRequest::setQueryParameters(const char* start, const char* end){
    // 从问号后逐级分割参数
    // 传入参数:status=shipped&page=2&limit=20&sort=date 
    std::string argumentStr(start, end);
    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    while((pos=argumentStr.find('&',prev))!=std::string::npos)
    {
        // 获取子串
        std::string subStr=argumentStr.substr(prev,pos-prev); // status=shipped
        // 找到'=' 前为key 后为value
        std::string::size_type equalPos = subStr.find('=',0);
        // 赋值
        if(equalPos!=std::string::npos)
        {
            std::string key = subStr.substr(0,equalPos);
            std::string value = subStr.substr(equalPos+1,pos-equalPos-1);
            // std::cout<< "key="<< key << " value="<< value << std::endl; // 测试
            queryParameters_.emplace(key,value);
        }
        prev = pos + 1; 
    }

    // 处理最后一个参数最后一个参数没有&不进入循环
    std::string subStr=argumentStr.substr(prev); 
    std::string::size_type equalPos = subStr.find('=',0);
    if(equalPos!=std::string::npos)
    {
        std::string key = subStr.substr(0,equalPos);
        std::string value = subStr.substr(equalPos+1,pos-equalPos-1);
        // std::cout<< "key="<< key << " value="<< value << std::endl; // 测试
        queryParameters_.emplace(key,value);
    }

}

bool HttpRequest::setHeader(const char* start, const char* end){
    // 逐行添加header (原函数需要输入:位置)
    // std::map<std::string, std::string> headers_;  
    bool ok = true;
    std::string h(start,end);
    std::string::size_type colon=h.find(':');
    if(colon!=std::string::npos){
        std::string key=h.substr(0,colon);
        std::string value=h.substr(colon+1);
        // 去除value的前导空格
        size_t start_pos = value.find_first_not_of(" \t\r\n");  // 找到第一个非空白字符
        if (start_pos != std::string::npos) 
            value = value.substr(start_pos);  // 截取前导空格之后的部分

        // 去除value的后导空格
        size_t end_pos = value.find_last_not_of(" \t\r\n");  // 找到最后一个非空白字符abc[] 找到c
        if (end_pos != std::string::npos) 
            value = value.substr(0,end_pos+1);  // 截取后导空格之前的部分
        
        // std::cout<< "key="<< key << " value="<< value << std::endl; // 测试
        headers_.emplace(key,value);
    }
    else
    {
        ok = false;
    }
    return ok;
    
}

void HttpRequest::setBody(const char* start, const char* end){
    content_.assign(start, end); 
}


// get 方法实现
std::string HttpRequest::getPathParameters(const std::string &key) const{
    try {
        return pathParameters_.at(key);
    } catch(const std::out_of_range& e){
        std::cout<<"getPathParameters键不存在"<<std::endl; 
        return "";  // 返回空字符串作为默认值
    }
}

std::string HttpRequest::getQueryParameters(const std::string &key) const{
    try {
        return queryParameters_.at(key);
    } catch(const std::out_of_range& e){
        std::cout<<"getQueryParameters键不存在"<<std::endl; 
        return "";  // 返回空字符串作为默认值
    }
}

std::string HttpRequest::getHeader(const std::string& field) const{
   try {
        return headers_.at(field);
    } catch(const std::out_of_range& e){
        // std::cout<<"getHeader键不存在"<<std::endl; 
        return "";  // 返回空字符串作为默认值
    }
}


//  交换两个 HttpRequest对象的所有内容
void HttpRequest::swap(HttpRequest& that){
    std::swap(method_, that.method_);
    std::swap(path_, that.path_);
    std::swap(pathParameters_, that.pathParameters_);
    std::swap(queryParameters_, that.queryParameters_);
    std::swap(version_, that.version_);
    std::swap(headers_, that.headers_);
    std::swap(receiveTime_, that.receiveTime_);
}


} // namespace http