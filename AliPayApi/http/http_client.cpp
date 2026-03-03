#include "http_client.h"

#define HTTP_TIMEOUT_MSECS    120000

string HttpClient::proxyIp;
string HttpClient::proxyPort;
string HttpClient::proxyUserName;
string HttpClient::proxyPassword;

HttpClient::HttpClient() {
    CURLcode init_ret = curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    responseEntity.clear();
}

HttpClient::~HttpClient() {
    curl_easy_cleanup(curl);
    curl_global_cleanup();
}

void HttpClient::setProxy(const string &proxyIp,
                          const string &proxyPort,
                          const string &proxyUserName,
                          const string &proxyPassword) {
    HttpClient::proxyIp       = proxyIp;
    HttpClient::proxyPort     = proxyPort;
    HttpClient::proxyUserName = proxyUserName;
    HttpClient::proxyPassword = proxyPassword;
}

size_t HttpClient::write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
    size_t length = size * nmemb;
    if (length == 0) return 0;

    HttpClient *client = static_cast<HttpClient*>(stream);
    client->responseEntity.append(static_cast<char*>(ptr), length);
    return length;
}

string HttpClient::sendSyncRequest(const string &url,
                                   const string &requestEntity,
                                   const HeaderMap &headers) {
    if(curl == NULL) {
        DebugLog("%s | %s", url.c_str(), "curl easy init error");
        return string();
    }

    struct curl_slist * headerlist = NULL;
    string headitem;
    for (HeaderMap::const_iterator iter = headers.begin(); iter != headers.end(); ++iter) {
        headitem = iter->first;
        headitem += ": ";
        headitem += iter->second;
        headerlist = curl_slist_append(headerlist, headitem.c_str());
        headitem.clear();
    }

    //curl_easy_setopt(curl, CURLOPT_VERBOSE,          1L);//调试信息打开
    //curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L); // 禁止自动重定向
    curl_easy_setopt(curl, CURLOPT_POSTREDIR, CURL_REDIR_POST_ALL); // 关键！

    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER,   0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST,   0L);
    curl_easy_setopt(curl, CURLOPT_HEADER,           1);  // 是否返回响应头
    curl_easy_setopt(curl, CURLOPT_URL,              url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST,             1);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS,       requestEntity.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER,       headerlist);

    if (!proxyIp.empty() && !proxyPort.empty()) {
        string proxyStr = proxyIp;
        proxyStr += ":";
        proxyStr += proxyPort;
        curl_easy_setopt(curl, CURLOPT_PROXY,        proxyStr.c_str());
        if (!proxyUserName.empty() && !proxyPassword.empty()) {
            string proxyUserPwd = proxyUserName;
            proxyUserPwd += ":";
            proxyUserPwd += proxyPassword;
            curl_easy_setopt(curl, CURLOPT_PROXYUSERPWD, proxyUserPwd.c_str());
        }
    }

    DebugLog("send:%s", requestEntity.c_str());

    // // ===== 手动构造并输出完整的 HTTP 请求报文（用于调试）=====
    // std::ostringstream requestStream;

    // // 1. 请求行（POST 方法固定）
    // requestStream << "POST " << url << " HTTP/1.1\r\n";

    // // 2. Host 头（从 URL 中提取，简化处理）
    // // 注意：这里简化处理，实际应解析 URL 获取 host:port
    // size_t start = url.find("://");
    // size_t hostStart = (start == std::string::npos) ? 0 : start + 3;
    // size_t hostEnd = url.find('/', hostStart);
    // std::string host = (hostEnd == std::string::npos) ? url.substr(hostStart) : url.substr(hostStart, hostEnd - hostStart);
    // requestStream << "Host: " << host << "\r\n";

    // // 3. 其他自定义头
    // for (HeaderMap::const_iterator iter = headers.begin(); iter != headers.end(); ++iter) {
    //     requestStream << iter->first << ": " << iter->second << "\r\n";
    // }

    // // 4. Content-Length（必须显式设置，否则 curl 可能用 chunked）
    // // 注意：libcurl 默认对 POSTFIELDS 设置 Content-Length
    // size_t contentLength = requestEntity.length();
    // requestStream << "Content-Length: " << contentLength << "\r\n";

    // // 5. 空行（分隔头和体）
    // requestStream << "\r\n";

    // // 6. 请求体（如果是文本，可直接打印；二进制需谨慎）
    // requestStream << requestEntity;

    // // 输出完整请求报文
    // std::cout<<"Full HTTP Request:"<<requestStream.str().c_str()<<std::endl;
    // // ==============================================


    responseEntity.clear();

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,    write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA,        this);

    curl_easy_perform(curl);
    curl_slist_free_all(headerlist);

    DebugLog("resv:%s", responseEntity.c_str());
    // std::cout<<responseEntity<<std::endl;
    return responseEntity;
}

string HttpClient::sendSyncRequest(const string &url,
                                   const ParamsMap &paramsMap,
                                   const HeaderMap &headers) {
    // 分离业务参数和其他参数
    string bizContent;
    string queryParams;
    string item;
    
    for (ParamsMap::const_iterator iter = paramsMap.begin(); iter != paramsMap.end(); ++iter) {
        const string& key = iter->first;
        const string& value = iter->second;
        
        // 对参数进行URL编码
        char *encodedKey = curl_easy_escape(curl, key.c_str(), key.length());
        char *encodedValue = curl_easy_escape(curl, value.c_str(), value.length());
        
        if (key == "biz_content") {
            // biz_content 放到 body 中
            if (!bizContent.empty()) {
                bizContent.push_back('&');
            }
            if (encodedKey) {
                bizContent += encodedKey;
            } else {
                bizContent += key;
            }
            bizContent += "=";
            if (encodedValue) {
                bizContent += encodedValue;
            } else {
                bizContent += value;
            }
        } else {
            // 其他参数放到 URL query 中
            if (!queryParams.empty()) {
                queryParams.push_back('&');
            }
            if (encodedKey) {
                queryParams += encodedKey;
            } else {
                queryParams += key;
            }
            queryParams += "=";
            if (encodedValue) {
                queryParams += encodedValue;
            } else {
                queryParams += value;
            }
        }
        
        if (encodedKey) {
            curl_free(encodedKey);
        }
        if (encodedValue) {
            curl_free(encodedValue);
        }
    }
    
    // 构造完整的 URL（包含 query 参数）
    string fullUrl = url;
    if (!queryParams.empty()) {
        if (fullUrl.find('?') == string::npos) {
            fullUrl += "?";
        } else {
            fullUrl += "&";
        }
        fullUrl += queryParams;
    }
    
    // std::cout << "Request URL: " << fullUrl << std::endl;
    // std::cout << "Request Body: " << bizContent << std::endl;
    
    // 发送请求，biz_content 放在 body 中
    string responseStr = sendSyncRequest(fullUrl, bizContent, headers);
    return responseStr;
}