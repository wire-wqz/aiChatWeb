#include "../../include/AiUtil/AiToolConfig.h"

// 从配置文件加载工具定义和提示词模板（封装promptTemplate_和tools_）
bool AiToolConfig::loadFromFile(){

    // 提示词读取
    std::ifstream file(promptPath_);
    if (!file.is_open()) {
        std::cerr << "[AIConfig] Unable to open configuration file: " << promptPath_<< std::endl;
        return false;
    }
    json j;
    file >> j;
    if (!j.contains("prompt_template") || !j["prompt_template"].is_string()) {
        std::cerr << "[AIConfig] prompt_template is missing" << std::endl;
        return false;
    }
    promptTemplate_ = j["prompt_template"].get<std::string>();
    file.close();

    // 工具提取
    if(isMcp_)
    {
        std::ifstream file(promptTools_);
        if (!file.is_open()) {
            std::cerr << "[AIConfig] Unable to open configuration file: " << promptTools_<< std::endl;
            return false;
        }
        json j;
        file >> j;
        if (j.contains("tools") && j["tools"].is_array()) {
            for(auto& j_tool:j["tools"])
            {   
                AiTool aitool;
                aitool.name = j_tool.value("name","");
                aitool.desc = j_tool.value("desc", "");
                if (j_tool.contains("params") && j_tool["params"].is_object()) {
                for (auto& [key, val] : j_tool["params"].items()) {
                        aitool.params[key] = val.get<std::string>();
                    } 
                }
                tools_.push_back(std::move(aitool));
            }
        }
        else
        {
            return false;
        }
        file.close();
    }

    // 联网搜索工具提取
    if(isGoogle_)
    {
        std::ifstream file(promptGoogle_);
        if (!file.is_open()) {
            std::cerr << "[AIConfig] Unable to open configuration file: " << promptGoogle_<< std::endl;
        }
        json j;
        file >> j;
        if (j.contains("tools") && j["tools"].is_array()) {
            for(auto& j_tool:j["tools"])
            {   
                AiTool aitool;
                aitool.name = j_tool.value("name","");
                aitool.desc = j_tool.value("desc", "");
                if (j_tool.contains("params") && j_tool["params"].is_object()) {
                for (auto& [key, val] : j_tool["params"].items()) {
                        aitool.params[key] = val.get<std::string>();
                    } 
                }
                tools_.push_back(std::move(aitool));
            }
        }
        else
        {
            return false;
        }
        file.close();
    }

    return true;
}

// 将tools_列表格式化为字符串
std::string AiToolConfig::buildToolList() const {
    std::ostringstream oss;
    for (const auto& t : tools_) {
        oss << t.name << "(";
        bool first = true;
        for (const auto& [key, val] : t.params) {
            if (!first) oss << ", ";
            oss << key;
            oss << ": ";
            oss << val;
            first = false;
        }
        oss << "), " << t.desc << "\n";
    }
    return oss.str();
}

// 基于正则匹配封装第一次提问的提示词
std::string AiToolConfig::buildFirstPrompt(const std::string& userInput) const {
    std::string result = promptTemplate_;
    if(isGoogle_)
        result = std::regex_replace(result, std::regex("\\{user_input\\}"), "必须使用search_web工具的基础上，可以使用其它工具辅助:"+userInput); // 替换{user_input}为userInput
    else
        result = std::regex_replace(result, std::regex("\\{user_input\\}"), userInput); // 替换{user_input}为userInput
    result = std::regex_replace(result, std::regex("\\{tool_list\\}"), buildToolList());
    return result;
}

// 解析第一次回答，返回AIToolCall
AiToolCall AiToolConfig::parseFirstAiAnswer(const std::string& response) const {
    AiToolCall result;
    try {
        // Try parsing as JSON
        json jAll = json::parse(response);


        for(const auto& j:jAll)
        {
            if (j.contains("tool") && j["tool"].is_string()) 
            {
                if (j.contains("args") && j["args"].is_object()) 
                {
                    result.invokeTools.emplace(j["tool"],j["args"]);
                }
                else
                {
                    result.invokeTools.emplace(j["tool"],json());
                }
            }
            result.isToolCall = true;
        }
       
        // if (j.contains("tool") && j["tool"].is_string()) 
        // {
        //     result.toolName = j["tool"].get<std::string>();
        //     if (j.contains("args") && j["args"].is_object()) 
        //     {
        //         result.args = j["args"];
        //     }
        //     result.isToolCall = true;
        // }
    }
    catch (...) {
        // Not JSON, directly return text response
        result.isToolCall = false;
    }
    return result;
}

// 组装第二次提问的提示词输入（将工具结果和任务一起输入大模型）
std::string AiToolConfig::buildSecondPrompt(
    const std::string& userInput,  // 用户输入
    AiToolCall& aiToolCall    // 工具调用参数和工具调用结果
    ) const  
{   
    
    json toolsArray = json::array();
    json resultsArray = json::array();

    // 遍历 multimap 整理结果
    // 注意：unordered_multimap 遍历顺序不一定与插入顺序一致， 如果大模型对顺序敏感，建议在 invokeTools 和 invokeResults 中维护一个 ID 进行匹配，
    for(const auto& pair : aiToolCall.invokeTools)
    {
        json item;
        item["tool"] = pair.first;
        item["args"] = pair.second;
        toolsArray.push_back(item);
    }
    
    for (const auto& pair : aiToolCall.invokeResults) {
        json item;
        item["tool"] = pair.first;    // 工具名
        item["result"] = pair.second; // 工具返回的具体数据
        resultsArray.push_back(item);
    }

    // 组装提示词
    std::ostringstream prompt;
    prompt 
        << "下面是用户说的话：" << userInput << "\n"
        << "根据您之前要求调用了工具，参数如下(JSON格式)：" << "\n"
        << toolsArray.dump(4) << "\n"
        << "以下是工具执行后返回的详细结果(JSON格式)：\n" 
        << resultsArray.dump(4) << "\n"
        << "请结合上述工具结果，直接以自然语言回答用户的问题，不要输出JSON，不要提及你使用了工具。";
    
    return prompt.str();
}