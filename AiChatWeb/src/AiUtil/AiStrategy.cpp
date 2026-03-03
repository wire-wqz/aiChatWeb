#include"../../include/AiUtil/AiStrategy.h"

// qianwen-plus
// 阿里云 获取 API 地址
std::string QianwenPlusStrategy::getApiUrl() const
{
    return "https://dashscope.aliyuncs.com/compatible-mode/v1/chat/completions";
}

// 阿里云 获取 API 密钥 Key
std::string QianwenPlusStrategy::getApiKey() const
{
    return apiKey_;
}

// 阿里云 获取模型版本名称
std::string QianwenPlusStrategy::getModel() const
{
    return "qwen-plus";
}

// 阿里云 构建请求体
/*
将std::vector<std::pair<std::string, long long>>转化为json
[
    {"用户问题",long long},
    {"AI回复",long long},
    ...
]
转化为
[
    { role: 'user', content: "用户问题" },
    { role: 'assistant', content: "AI回复"`" },
    ...
]
*/
json QianwenPlusStrategy::buildRequestBody(const std::vector<std::pair<std::string, long long>>& messages) const 
{
    json payload;
    payload["model"]=getModel();

    json msgArray=json::array();
    for(int i=0;i<messages.size();i++)
    {
        json msg;
        if(i%2==0)// 用户消息
        {
            msg["role"]="user";
        }
        else      // AI消息
        {
            msg["role"]="assistant";
        }    
        msg["content"]=messages[i].first;
        msgArray.push_back(msg);
    }
    payload["messages"]=msgArray;
    return payload;
}

// 阿里云 解析响应体
std::string QianwenPlusStrategy::parseResponseBody(const json& responseBody) const
{
    if (responseBody.contains("choices") && !responseBody["choices"].empty()) {
        return responseBody["choices"][0]["message"]["content"];
    }
    return {};
}

int QianwenPlusStrategy::getTotalTokens(const json& responseBody) const{
    int total_tokens = 0;
    if (responseBody.contains("usage")) {
        const auto& model_usage = responseBody["usage"];
        if (model_usage.contains("total_tokens")) {
            total_tokens = model_usage["total_tokens"].get<int>();
        }
    }
    return total_tokens;
}

/*******************************************************************************/
// qianwen-Max
std::string QianwenMaxStrategy::getApiUrl() const {
    return "https://dashscope.aliyuncs.com/api/v1/apps/"+appid_+"/completion";
}

std::string QianwenMaxStrategy::getApiKey()const {
    return apiKey_;
}


std::string QianwenMaxStrategy::getModel() const {
    return ""; 
}

json QianwenMaxStrategy::buildRequestBody(const std::vector<std::pair<std::string, long long>>& messages) const {
    json payload;
    json msgArray = json::array();
    for (size_t i = 0; i < messages.size(); ++i) {
        json msg;
        msg["role"] = (i % 2 == 0 ? "user" : "assistant");
        msg["content"] = messages[i].first;
        msgArray.push_back(msg);
    }
    payload["input"]["messages"] = msgArray;
    payload["parameters"] = json::object(); 
    return payload;
}

std::string QianwenMaxStrategy::parseResponseBody(const json& response) const {
    if (response.contains("output") && response["output"].contains("text")) {
        return response["output"]["text"];
    }
    return {};
}

int QianwenMaxStrategy::getTotalTokens(const json& responseBody) const{
    int total_tokens = 0;
    if (responseBody.contains("usage")) {
        const auto& model_usage = responseBody["usage"];
        if (model_usage.contains("total_tokens")) {
            total_tokens = model_usage["total_tokens"].get<int>();
        }
    }
    return total_tokens;
}

/*******************************************************************************/
// qianwen-Max-RAG
std::string QianwenMaxRagStrategy::getApiUrl() const {
    return "https://dashscope.aliyuncs.com/api/v1/apps/"+appid_+"/completion";
}

std::string QianwenMaxRagStrategy::getApiKey()const {
    return apiKey_;
}


std::string QianwenMaxRagStrategy::getModel() const {
    return ""; 
}

json QianwenMaxRagStrategy::buildRequestBody(const std::vector<std::pair<std::string, long long>>& messages) const {
    json payload;
    json msgArray = json::array();
    for (size_t i = 0; i < messages.size(); ++i) {
        json msg;
        msg["role"] = (i % 2 == 0 ? "user" : "assistant");
        msg["content"] = messages[i].first;
        msgArray.push_back(msg);
    }
    payload["input"]["messages"] = msgArray;
    payload["parameters"] = json::object(); 
    return payload;
}

std::string QianwenMaxRagStrategy::parseResponseBody(const json& response) const {
    if (response.contains("output") && response["output"].contains("text")) {
        return response["output"]["text"];
    }
    return {};
}

int QianwenMaxRagStrategy::getTotalTokens(const json& responseBody) const{
    int total_tokens = 0;
    if (responseBody.contains("usage")) {
        const auto& model_usage = responseBody["usage"];
        if (model_usage.contains("total_tokens")) {
            total_tokens = model_usage["total_tokens"].get<int>();
        }
    }
    return total_tokens;
}

/*******************************************************************************/
// 豆包

std::string DouBaoStrategy::getApiUrl()const {
    return "https://ark.cn-beijing.volces.com/api/v3/chat/completions";
}

std::string DouBaoStrategy::getApiKey()const {
    return apiKey_;
}

std::string DouBaoStrategy::getModel() const {
    return "doubao-seed-2-0-pro-260215";
}

json DouBaoStrategy::buildRequestBody(const std::vector<std::pair<std::string, long long>>& messages) const {
    json payload;
    payload["model"] = getModel();
    json msgArray = json::array();

    for (size_t i = 0; i < messages.size(); ++i) {
        json msg;
        if (i % 2 == 0) {
            msg["role"] = "assistant";
        }
        else {
            msg["role"] = "system";
        }
        msg["content"] = messages[i].first;
        msgArray.push_back(msg);
    }
    payload["messages"] = msgArray;
    return payload;
}

std::string DouBaoStrategy::parseResponseBody(const json& response) const {
     if (response.contains("choices") && !response["choices"].empty()) {
        return response["choices"][0]["message"]["content"];
    }
    return {};
}

int DouBaoStrategy::getTotalTokens(const json& responseBody) const{
    int total_tokens = 0;
    if (responseBody.contains("usage")) {
        const auto& model_usage = responseBody["usage"];
        if (model_usage.contains("total_tokens")) {
            total_tokens = model_usage["total_tokens"].get<int>();
        }
    }
    return total_tokens;
}