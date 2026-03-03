// AiHelper.h 
// wire 
// 2025.1.4
/* 
调用工厂实例（AiStrategy）
*/

#include <string>
#include <vector>
#include <utility>
#include <curl/curl.h>
#include <iostream>
#include <sstream>

#include "../../../HttpServer/include/utils/JsonUtil.h"
#include "../../../HttpServer/include/utils/MysqlUtil.h"
#include "../../../HttpServer/include/messagequeue/MQManager.h"

#include "AiFactory.h" 
#include "AiToolConfig.h"
#include "AIToolRegistry.h"

class AiHelper {

public:
    // 构造函数
    AiHelper();

    //设置Ai
    void setStrategy(std::string name);

    // 添加一条消息到messages_
    void addMessage(std::string Id, bool is_Login, bool is_user, const std::string& userInput, std::string sessionId);        

    // 从sql中恢复消息到messages_，外部访问sql（todo）
    void restoreMessage(const std::string& userInput, long long ms);

    // 发送聊天消息，返回AI的响应内容,string 形式
    json chat(std::string Id, bool is_Login, std::string sessionId, std::string userQuestion, std::string modelType);        
    
    // 输出messages_
    std::vector<std::pair<std::string, long long>> GetMessages();

    // (外部可修改)
    bool isMCP_ = false;
    bool isGoogle_ = true;

private:
    // 转义字符安全处理，防止sql注入
    std::string escapeString(const std::string& input);

    // 将messages_存储到sql中
    void pushMessageToMysql(std::string userId, bool is_user, const std::string& content, long long ms,std::string sessionId);

    // 访问Curl，请求体为payload 返回响应体
    json executeCurl(const json& payload);

     // curl 回调函数，（作用不大，计算返回数量量）
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);

    // 临时提问，不将数据记录到messages_和sql中
    std::string tempChatMessage(std::string Id, bool is_Login, const std::string& userInput,int &totalTokens);

    std::string chatMessage(std::string Id, bool is_Login, const std::string& userInput, std::string sessionId,int &totalTokens);

    // 从数据库中插叙tokens
    int getTokensFromMysql(std::string userId);

    // 将token修改添加到数据库中
    void pushTokensToMysql(std::string userId,int totalTokens);

    // token修改
    void addTokenUsage(std::string userId,bool is_Login,int& totalTokens,const json resq);
private:
    std::shared_ptr<AiStrategy> strategy_;                      // 使用的Ai类
    std::vector<std::pair<std::string, long long>> messages_;   // 一个用户针对一个AIHelper，messages存放用户的历史对话 偶数下标代表用户的信息，奇数下标是ai返回的内容, long long代表时间戳
    http::MysqlUtil mysqlUtil_;  

    // 每个聊天会话都具有一个独有的AiHelper，因此不适合将token数作为类内变量存储
};