#include "../../include/AiUtil/AIToolRegistry.h"

// 构造函数（初始化注册工具）
AiToolRegistry::AiToolRegistry(){
    registerTool("get_weather", getWeather);
    registerTool("get_time", getTime);
    registerTool("search_web", searchWeb);
}

// 注册工具
void AiToolRegistry::registerTool(const std::string& name, ToolFunc func)
{
    if(tools_.find(name)!=tools_.end())
    {
        std::cerr<<"tool:"<<name<<"已存在"<<std::endl;
        return;
    }
    tools_[name]=func;
}

// 依据工具名和调用参数，调用工具
void AiToolRegistry::invokeTool(AiToolCall& aiToolCall) const{

    for(const auto& [name,args]:aiToolCall.invokeTools)
    {
        auto it = tools_.find(name);
        if (it == tools_.end()) {
            throw std::runtime_error("Tool not found: " + name);
        }
        json resultJson = it->second(args);
        aiToolCall.invokeResults.emplace(name,resultJson);
    }

    // auto it = tools_.find(name);
    // if (it == tools_.end()) {
    //     throw std::runtime_error("Tool not found: " + name);
    // }
    // return it->second(args);
}

// 判断工具名：name是否存在
// bool AiToolRegistry::hasTool(const std::string& name) const{
//     if(tools_.find(name)==tools_.end())
//     {
//         return false;
//     }
//     return true;
// }

// 将CURL接收的网络数据分块拼接成完整字符串
size_t AiToolRegistry::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// 通过curl 访问天气数据
json AiToolRegistry::getWeather(const json& args) {
    // 获取传入参数
    if(!args.contains("city"))
    {
        return json{ {"error", "Missing parameter: city"} };
    }
    std::string city = args["city"].get<std::string>();

    if(!args.contains("forecastMode"))
    {
        return json{ {"forecastMode", "Missing parameter: forecastMode"} };
    }
    bool forecastMode = args["forecastMode"].get<bool>();
  

    // 中文URL编码
    std::string encodedCity;
    char* encoded = curl_easy_escape(nullptr,city.c_str(),city.length()); 
    if(encoded)
    {
        encodedCity = encoded;
        curl_free(encoded);
    }
    else 
    {
        return json{ {"error", "URL encode failed"} };
    }

    // 初始化 libcurl 的“Easy Interface”会话
    CURL* curl = curl_easy_init();
    if (!curl) {
        return json{ {"error", "Failed to init CURL"} };
    }
    std::string response;

    // 设置请求的目标地址（URL）
    std::string path; // 不要使用url充当变量名
    if(forecastMode)
        path = "https://wttr.in/" + encodedCity + "?format=j1&lang=zh"; // 返回：JSON 详细模式：包含 current_condition 和 weather (forecast) 数组
    else
        path = "https://wttr.in/" + encodedCity + "?format=3&lang=zh";  // 返回：单行文本模式，例如，北京: 晴 +25°C
    curl_easy_setopt(curl, CURLOPT_URL, path.c_str());


    // 设置回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

    // 设置回调函数的数据容器(调用回调函数时，把 &readBuffer作为第四个参数传递)
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // 其它设置
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 60L);                       // 设置整个HTTP请求的超时时间为60秒
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);                 // 启用HTTP重定向自动跟随

    // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);                 //  强制忽略 SSL 证书校验 (解决证书无效、自签名、过期问题)
    // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    // curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);// 强制使用 TLS 1.2 或更高版本 (解决服务端拒绝旧协议导致的 SSL connect error)

    // 发送请求
    CURLcode res = curl_easy_perform(curl);

    // 清理内存
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return json{ 
            {"error", "CURL request failed"}, 
            {"curl_code", res},
            {"curl_message", curl_easy_strerror(res)} // 这里会告诉你具体原因
        };
    }

    // response处理
    json weatherData;
    if(forecastMode)
    {
        try
        {
            weatherData = json::parse(response);  // string 转 json
        }
        catch(...)
        {
            weatherData = response;
        }
    }
    else
    {
        weatherData = response; 
    }

    // 获取时间
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_now = {};
    localtime_r(&t, &tm_now);

    std::stringstream date;
    date << std::put_time(&tm_now, "%Y-%m-%d");

    return json{ 
        {"city", city},
        {"current_date",date.str()},
        {"weather", weatherData} 
    };
}


// 获取时间
json AiToolRegistry::getTime(const json& args) {
    // (void)args; // 传入参数为空
    // std::time_t t = std::time(nullptr);
    // std::tm* now = std::localtime(&t);
    // char buffer[64];
    // std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", now);
    // return json{ {"time", buffer} };

    (void)args;
    
    // 获取当前时间
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::system_clock::to_time_t(now);
    
    // 线程安全的本地时间转换
    std::tm tm_now = {};
    localtime_r(&t, &tm_now);
    
    // 格式化输出
    std::ostringstream oss;
    oss << std::put_time(&tm_now, "%Y-%m-%d %H:%M:%S");
    
    return json{{"time", "CST::"+oss.str()+"(如果用户询问其它时间，请根据您的知识进行换算)"}};
}


// 简单的网络搜索工具 
// 使用 Serper.dev 是实现 LLM 联网搜索的最佳方案之一。它是专门为 AI 开发者设计的 Google 搜索 API 封装，速度快、返回标准 JSON、且免费额度（2500次）足够开发调试。
json AiToolRegistry::searchWeb(const json& args) {
    // ---------------------------------------------------------
    // 1. 配置区域
    // ---------------------------------------------------------
    // 请将此处替换为你从 serper.dev 获取的真实 API Key
    const std::string API_KEY = "e681097b776e634ee66f291b6ca39312f385b729"; 

    // ---------------------------------------------------------
    // 2. 参数解析
    // ---------------------------------------------------------
    if(!args.contains("query")) {
        return json{ {"error", "Missing parameter: query"} };
    }
    std::string query = args["query"].get<std::string>();

    // ---------------------------------------------------------
    // 3. 构建请求体 (JSON Payload)
    // ---------------------------------------------------------
    json payloadJson = {
        {"q", query},
        {"gl", "cn"},   // 地理位置：中国 (cn) 或 美国 (us)
        {"hl", "zh-cn"} // 语言：简体中文
    };
    std::string payload = payloadJson.dump();

    // ---------------------------------------------------------
    // 4. CURL 初始化
    // ---------------------------------------------------------
    CURL* curl = curl_easy_init();
    if (!curl) return json{ {"error", "Failed to init CURL"} };

    std::string response;
    struct curl_slist *headers = NULL;

    // 设置 Header
    headers = curl_slist_append(headers, "Content-Type: application/json");
    std::string apiKeyHeader = "X-API-KEY: " + API_KEY;
    headers = curl_slist_append(headers, apiKeyHeader.c_str());

    // 设置 URL (Serper 标准端点)
    curl_easy_setopt(curl, CURLOPT_URL, "https://google.serper.dev/search");

    // 设置 POST 方式
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload.c_str());
    
    // 设置 Header 列表
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // 回调与容器
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // 超时与兼容性设置
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L); 
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // 避免 SSL 报错
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    // ---------------------------------------------------------
    // 5. 执行请求
    // ---------------------------------------------------------
    CURLcode res = curl_easy_perform(curl);
    
    // 清理 header 链表 (重要！防止内存泄漏)
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        return json{ 
            {"error", "Serper API request failed"}, 
            {"curl_message", curl_easy_strerror(res)}
        };
    }

    // ---------------------------------------------------------
    // 6. 解析结果 (提取 LLM 需要的核心信息)
    // ---------------------------------------------------------
    json rawJson;
    try {
        rawJson = json::parse(response);
    } catch (...) {
        return json{ {"error", "Invalid JSON response from Serper"} };
    }

    json result = json::object();
    result["query"] = query;
    result["source"] = "Google Search (via Serper)";
    
    json organicResults = json::array();

    // 优先提取 "answerBox" (如果 Google 直接给出了答案，如汇率、定义)
    if (rawJson.contains("answerBox")) {
        result["highlight"] = rawJson["answerBox"];
    }
    // 其次提取 "knowledgeGraph" (知识图谱，如人物简介)
    else if (rawJson.contains("knowledgeGraph")) {
        result["highlight"] = rawJson["knowledgeGraph"];
    }

    // 提取自然搜索结果 (organic) - 取前 5 条即可
    if (rawJson.contains("organic") && rawJson["organic"].is_array()) {
        int count = 0;
        for (const auto& item : rawJson["organic"]) {
            json entry;
            if (item.contains("title")) entry["title"] = item["title"];
            if (item.contains("link")) entry["link"] = item["link"];
            if (item.contains("snippet")) entry["snippet"] = item["snippet"]; // 摘要
            
            organicResults.push_back(entry);
            if (++count >= 5) break;
        }
    }

    result["results"] = organicResults;
    return result;
}