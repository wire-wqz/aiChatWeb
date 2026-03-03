// AIToolRegistry.h 
// wire 
// 2025.1.22
/* 
MCP工具调用中的工具注册类，负责具体工具的调用和注册

工具的调用流程：
1. 第一次提问：输入工具列表和任务，询问大模型是否需要调用哪些工具
2. 第一次返回：大模型返回需要使用的工具
3. 第二次提问：服务器调用工具，然后将工具结果和任务一起输入大模型
4. 第二次返回：返回需要的回答
*/

#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <stdexcept>
#include <iostream>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <curl/curl.h>

#include "../../../HttpServer/include/utils/JsonUtil.h"
#include "AiToolConfig.h"

class AiToolRegistry {
public:

    // 工具调用函数 类型 输入为调用参数json 
    using ToolFunc = std::function<json(const json&)>;

    // 单实例模式
    static AiToolRegistry& instance(){
        static AiToolRegistry aiToolRegistryInstance;
        return aiToolRegistryInstance;
    }
    
    // 依据工具名和调用参数，调用工具
    void invokeTool(AiToolCall& aiToolCall) const;

    // // 判断工具名：name是否存在
    // bool hasTool(const std::string& name) const;

private:
    // 构造函数（初始化注册工具）
    AiToolRegistry();

    // 单例模式：禁止拷贝和赋值
    AiToolRegistry(const AiToolRegistry&) = delete;  
    AiToolRegistry& operator=(const AiToolRegistry&) = delete;

    // 注册工具
    void registerTool(const std::string& name, ToolFunc func);

    // 将CURL接收的网络数据分块拼接成完整字符串（静态成员函数）
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
    
    // 获取天气工具
    static json getWeather(const json& args);
    
    // 获取时间工具
    static json getTime(const json& args);

    static json searchWeb(const json& args);

private:
    std::unordered_map<std::string, ToolFunc> tools_;     // 存储 工具名 和 工具调用函数 的键值对
};
