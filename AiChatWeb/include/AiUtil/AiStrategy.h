// AiStrategy.h 
// wire 
// 2025.1.4
/* 
统一管理和调用不同服务商或不同类型的 AI 模型接口
*/
#pragma once
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <sstream>
#include <memory>

#include "../../../HttpServer/include/utils/JsonUtil.h" 

// 虚函数
class AiStrategy {
public:
    // 虚析构函数
    virtual ~AiStrategy() = default;
    
    // 获取 API 地址
    virtual std::string getApiUrl() const = 0;

    // 获取 API 密钥 Key
    virtual std::string getApiKey() const = 0;

    // 获取模型版本名称
    virtual std::string getModel() const = 0;

    // 构建请求体（输入messages 输出请求体json类型）
    virtual json buildRequestBody(const std::vector<std::pair<std::string, long long>>& messages) const = 0;

    // 解析响应体获取输出（输入响应体json类型 输出字符串）
    virtual std::string parseResponseBody(const json& responseBody) const = 0;

    //解析响应体获取输入token数
    virtual int getTotalTokens(const json& responseBody) const = 0;
};

/*******************************************************************************/
// qianwen-plus
class QianwenPlusStrategy : public AiStrategy {
public:
    QianwenPlusStrategy() {
        // 建议把API Key配置到环境变量，从而避免在代码里显式地配置API Key，降低泄漏风险.但作为demo，暂时明文配置
        // const char* key = std::getenv("DASHSCOPE_API_KEY"); // 从系统环境中查询密钥
        // if (!key) 
        // throw std::runtime_error("Aliyun API Key not found!");
        std::string key="sk-80fc8e2e949c4da8a15f4244e2f3f245";
        apiKey_ = key;
    }

    std::string getApiUrl() const override;
    std::string getApiKey() const override;
    std::string getModel() const override;

    json buildRequestBody(const std::vector<std::pair<std::string, long long>>& messages) const override;
    std::string parseResponseBody(const json& responseBody) const override;
    int getTotalTokens(const json& responseBody) const override;

private:
    std::string apiKey_;
};

/*******************************************************************************/
// qianwen-Max
class QianwenMaxStrategy : public AiStrategy {
public:
    QianwenMaxStrategy() {
        std::string key="sk-80fc8e2e949c4da8a15f4244e2f3f245";
        std::string appid="f4d3b025cce04f2ab0ef5be1c23373ba";
        apiKey_ = key;
        appid_ = appid;
    }

    std::string getApiUrl() const override;
    std::string getApiKey() const override;
    std::string getModel() const override;

    json buildRequestBody(const std::vector<std::pair<std::string, long long>>& messages) const override;
    std::string parseResponseBody(const json& responseBody) const override;
    int getTotalTokens(const json& responseBody) const override;

private:
    std::string apiKey_;
    std::string appid_;
};

/*******************************************************************************/
// qianwen-Max-RAG
class QianwenMaxRagStrategy : public AiStrategy {
public:
    QianwenMaxRagStrategy() {
        std::string key="sk-80fc8e2e949c4da8a15f4244e2f3f245";
        std::string appid="74655a23efe543cfa269dab44dae3538";
        apiKey_ = key;
        appid_ = appid;
    }

    std::string getApiUrl() const override;
    std::string getApiKey() const override;
    std::string getModel() const override;

    json buildRequestBody(const std::vector<std::pair<std::string, long long>>& messages) const override;
    std::string parseResponseBody(const json& responseBody) const override;
    int getTotalTokens(const json& responseBody) const override;

private:
    std::string apiKey_;
    std::string appid_;
};

/*******************************************************************************/
// 豆包
class  DouBaoStrategy: public AiStrategy {
public:
    DouBaoStrategy() {
        std::string key="19055981-c86b-40d7-8aa8-210bb72a9041";
        apiKey_ = key;
    }

    std::string getApiUrl() const override;
    std::string getApiKey() const override;
    std::string getModel() const override;

    json buildRequestBody(const std::vector<std::pair<std::string, long long>>& messages) const override;
    std::string parseResponseBody(const json& responseBody) const override;
    int getTotalTokens(const json& responseBody) const override;

private:
    std::string apiKey_;
    std::string appid_;
};





