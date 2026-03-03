// AiToolConfig.h 
// wire 
// 2025.1.22
/* 
MCP工具调用中的工具设置类

工具的调用流程：
1. 第一次提问：输入工具列表和任务，询问大模型是否需要调用哪些工具
2. 第一次返回：大模型返回需要使用的工具
3. 第二次提问：服务器调用工具，然后将工具结果和任务一起输入大模型
4. 第二次返回：返回需要的回答
*/


#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <regex>
#include <fstream>
#include <sstream>
#include <iostream>
#include "../../../HttpServer/include/utils/JsonUtil.h"  


// 工具类
struct AiTool {
    std::string name;                                       // 工具名称（"get_weather"）
    std::unordered_map<std::string, std::string> params;    // 可传递参数("city":"北京")
    std::string desc;                                       // 工具描述（"获取天气"）
};

// 工具是否被调用
struct AiToolCall {
    // std::vector<std::string> toolName;                      // 工具名称
    // std::vector<json> args;                                 // 调用工具时传递的参数

    std::unordered_multimap<std::string, json>  invokeTools;   // 工具名-传入参数对照表
    std::unordered_multimap<std::string, json>  invokeResults;   // 工具名-结果   对照表
    bool isToolCall = false;                                   // 是否被调用
};

class AiToolConfig {
public:
    // 构造函数
    AiToolConfig(bool isMcp,bool isGoogle):
        isMcp_(isMcp),
        isGoogle_(isGoogle)
    {};

    // 从配置文件加载工具定义和提示词模板（封装promptTemplate_和tools_）
    bool loadFromFile();             
    
    // 组装第一次提问的提示词输入（输入工具列表和任务，询问大模型是否需要调用哪些工具）
    std::string buildFirstPrompt(const std::string& userInput) const; 

    // 解析第一次回答，返回AIToolCall
    AiToolCall parseFirstAiAnswer(const std::string& response) const;

    // 组装第二次提问的提示词输入（将工具结果和任务一起输入大模型）
    std::string buildSecondPrompt(const std::string& userInput,AiToolCall& aiToolCall) const;

private:
    // 将tools_列表格式化为字符串
    std::string buildToolList() const;  

private:
    std::string promptTemplate_;                            // 提示词模板
    std::vector<AiTool> tools_;                             // 工具列表


    bool isMcp_;
    bool isGoogle_;


    // 提示词路径
    std::string promptPath_="/root/AiHttpServer/AiChatWeb/resource/AiUtils/AiTools/AiToolsPrompt.json";
    std::string promptTools_="/root/AiHttpServer/AiChatWeb/resource/AiUtils/AiTools/AiTools.json";
    std::string promptGoogle_="/root/AiHttpServer/AiChatWeb/resource/AiUtils/AiTools/AiGoogle.json";

};
