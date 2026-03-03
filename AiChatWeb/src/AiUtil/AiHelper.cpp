#include "../../include/AiUtil/AiHelper.h"

AiHelper::AiHelper(){
    // 注册工厂
    StrategyRegister<QianwenPlusStrategy>("1");
    StrategyRegister<DouBaoStrategy>("2");
    StrategyRegister<QianwenMaxStrategy>("3");
    StrategyRegister<QianwenMaxRagStrategy>("4");

    // 默认ai模型
    setStrategy("1");
}

//设置Ai
void AiHelper::setStrategy(std::string name){
    strategy_=StrategyFactory::instance().create(name);
}

// 添加一条消息到messages_
// 已登录会话 存储到sql
void AiHelper::addMessage(std::string Id, bool is_Login, bool is_user,const std::string& message, std::string sessionId) {
    // 时间戳
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    messages_.push_back({ message, ms});
    // 临时会话(为登录) 不存储到sql,仅存储在内存中,即messages_中,登录会话则存储到sql
    if(is_Login==true)
        pushMessageToMysql(Id, is_user, message, ms, sessionId);
}

// 从sql中恢复消息到messages_
void AiHelper::restoreMessage(const std::string& userInput,long long ms) {
    messages_.push_back({ userInput,ms });
}

// 输出messages_
std::vector<std::pair<std::string, long long>> AiHelper::GetMessages() {
    return this->messages_;
}

// 发送聊天消息，返回AI的响应内容,string 形式
json AiHelper::chat(std::string Id, bool is_Login, std::string sessionId, std::string userQuestion, std::string modelType) 
{
    json result;
    
    // 信息检查
    std::cout<<"test"
        << " ID:"<<Id
        <<" is_Login:"<<is_Login
        <<" sessionID:"<<sessionId
        <<" modelType:"<<modelType
        <<" isGoogle:"<<isGoogle_
        <<" isMCP:"<<isMCP_
        <<std::endl;

    // Tokens检查
    int totalTokens;  // 未登录用户不对tokens限制 
    if(is_Login)
    {
        totalTokens = getTokensFromMysql(Id);
        if(totalTokens<=0)
        {
            result["aiInformation"]="您的可用额度已耗尽，请充值后继续使用。";
            result["totalTokens"] = totalTokens;
            return result;
        }
    }

    //设置策略/ai模型
    setStrategy(modelType);
    
    // 不支持MCP逻辑
    if (!isMCP_&&!isGoogle_) 
    {
        result["aiInformation"]=chatMessage(Id,is_Login,userQuestion,sessionId,totalTokens);
        result["totalTokens"] = totalTokens;
        return result;
    }
    
    // MCP逻辑,初始化工具配置类
    AiToolConfig toolConfig(isMCP_,isGoogle_);
    toolConfig.loadFromFile();

    // 第一次提问：输入工具列表和任务，询问大模型是否需要调用哪些工具
    std::string firstPrompt = toolConfig.buildFirstPrompt(userQuestion);                // 组装第一次大模型输入Prompt
    std::string firstAnswer = tempChatMessage(Id,is_Login,firstPrompt,totalTokens);    // 输入提示获取响应

    // 第一次返回：大模型返回需要使用的工具
    std::cout << "工具调用的中间结果："<<firstAnswer<<std::endl; 
    AiToolCall toolCall = toolConfig.parseFirstAiAnswer(firstAnswer);      // 获取工具使用信息
    if(!toolCall.isToolCall)                                               // 不用调用工具
    {
        // addMessage(Id, is_Login, true, userQuestion, sessionId);
        // addMessage(Id, is_Login, false, firstAnswer, sessionId);
        // return firstAnswer;
        result["aiInformation"]=chatMessage(Id,is_Login,userQuestion,sessionId,totalTokens);
        result["totalTokens"] = totalTokens;
        return result;
    }

    // 第二次提问：服务器调用工具，然后将工具结果和任务一起输入大模型
    try{
        AiToolRegistry::instance().invokeTool(toolCall);
    }
    catch (const std::exception& e) {
        std::string toolResult = "[工具调用失败] " + std::string(e.what());
        std::cout << "Tool call failed:" << std::string(e.what()) << std::endl;
    }
    std::string secondPrompt = toolConfig.buildSecondPrompt(userQuestion,toolCall);           // 组装第二次大模型输入Prompt
    std::string secondAnswer = tempChatMessage(Id,is_Login,secondPrompt,totalTokens);                                 // 输入LLM API获取响应

    // 第二次返回：返回需要的回答
    addMessage(Id, is_Login, true, userQuestion, sessionId);
    addMessage(Id, is_Login, false, secondAnswer, sessionId);
    
    
    
    
    result["aiInformation"]=secondAnswer;
    result["totalTokens"] = totalTokens;
    return result;
}


// 临时提问，不将数据记录到messages_和sql中
std::string AiHelper::tempChatMessage(std::string Id, bool is_Login, const std::string& question,int &totalTokens){
    messages_.push_back({question, 0 });                                    // 添加到question到messages_
    json Req = strategy_->buildRequestBody(this->messages_);                // 封装请求体
    messages_.pop_back();                                                   // 从messages_移除secondPrompt
    json Resp = executeCurl(Req);                                           // 发送请求到LLM
    addTokenUsage(Id,is_Login,totalTokens,Resp);                            // 计算token消耗                                  
    std::string AiAnswer = strategy_->parseResponseBody(Resp);              // 解析响应
    return AiAnswer;
}

// 非临时提问
std::string AiHelper::chatMessage(std::string Id, bool is_Login, const std::string& userQuestion, std::string sessionId, int &totalTokens){
    addMessage(Id, is_Login, true, userQuestion, sessionId);
    // 封装请求体
    json payload = strategy_->buildRequestBody(this->messages_);
    //执行请求，返回响应体
    json response = executeCurl(payload);
    // 解析响应体，返回回复字符串
    std::string answer = strategy_->parseResponseBody(response);
    // 计算token消耗
    addTokenUsage(Id,is_Login,totalTokens,response);
    // 添加Ai message
    addMessage(Id, is_Login,false, answer, sessionId);
    // 返回结束
    return answer.empty() ? "[Error] 无法解析响应" : answer; 
}


// 访问Curl，请求体为payload 返回响应体，curl会直接返回响应体
json AiHelper::executeCurl(const json& payload)
{
    // 初始化 libcurl 的“Easy Interface”会话
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize curl");
    }
    std::string readBuffer;


    std::cout<<"test "<< strategy_->getApiUrl().c_str()<<' '<< strategy_->getApiKey()<<std::endl;

    
    // 设置请求的目标地址（URL）
    curl_easy_setopt(curl, CURLOPT_URL, strategy_->getApiUrl().c_str());

    // 装载请求头
    struct curl_slist* headers = nullptr;
    std::string authHeader = "Authorization: Bearer " + strategy_->getApiKey();
    headers = curl_slist_append(headers, authHeader.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // 装载 POST 请求的请求体(有请求体会自动将请求方法设置为POST)
    std::string payloadStr = payload.dump(); // 请求体序列化 json->string
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payloadStr.c_str());

    // 设置回调函数
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

    // 设置回调函数的数据容器(调用回调函数时，把 &readBuffer作为第四个参数传递)
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);

    // 发送请求
    CURLcode res = curl_easy_perform(curl);

    // 获取http状态码
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);

    // 打印调试 (在控制台查看)
    if(http_code!=200)
    {
        std::cout << "HTTP Status Code: " << http_code << std::endl;
        std::cout << "Request Body: " << payloadStr << std::endl;
        std::cout << "Response Body: " << readBuffer << std::endl;
    }
    std::cout << "Request Body: " << payloadStr << std::endl;


    // 清理内存
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (res != CURLE_OK) {
        throw std::runtime_error("curl_easy_perform() failed: " + std::string(curl_easy_strerror(res)));
    }

    try {
        return json::parse(readBuffer);  // 返回响应体
    }
    catch (...) {
        throw std::runtime_error("Failed to parse JSON response: " + readBuffer);
    }

}

// curl 回调函数，把返回的数据写到 string buffer
size_t AiHelper::WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t totalSize = size * nmemb;
    std::string* buffer = static_cast<std::string*>(userp);
    buffer->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

// 转义字符安全处理，防止sql注入（todo）/通过预处理语句 (PreparedStatement)可以规避该问题
std::string AiHelper::escapeString(const std::string& input){
    std::string output;
    output.reserve(input.size() * 2);
    for (char c : input) {
        switch (c) {
            case '\\': output += "\\\\"; break;
            case '\'': output += "\\\'"; break;
            case '\"': output += "\\\""; break;
            case '\n': output += "\\n"; break;
            case '\r': output += "\\r"; break;
            case '\t': output += "\\t"; break;
            default:   output += c; break;
        }
    }
    return output;
}

// token修改
void AiHelper::addTokenUsage(std::string userId,bool is_Login,int& totalTokens,const json resq)
{
    int consumedToken = strategy_->getTotalTokens(resq);
    std::cout<<"Tokens消耗:"<<consumedToken<<std::endl;
    totalTokens -= consumedToken;
    if(is_Login)
    {
        pushTokensToMysql(userId,totalTokens);
    }
}

// 将messages_存储到sql中
void AiHelper::pushMessageToMysql(std::string userId, bool is_user, const std::string& content, long long ms,std::string sessionId){


    // 同步sql
    // std::string sql= "INSERT INTO chat_message (id, session_id, is_user, content, ts) VALUES (?,?,?,?,?)";
    // mysqlUtil_.executeUpdate(sql,stoi(userId),sessionId,is_user,content,ms);



    // 利用消息队列异步执行mysql写入操作，用于流量削峰与解耦逻辑
    // std::string sql = "INSERT INTO chat_message (id, session_id, is_user, content, ts) VALUES ("
    //     + userId + ", "
    //     + sessionId + ", "
    //     + std::to_string(is_user ? 1 : 0) + ", "
    //     + "'" + escapeString(message) + "', "
    //     + std::to_string(ms) + ")";

    
    std::string sql= "INSERT INTO chat_message (id, session_id, is_user, content, ts) VALUES (?,?,?,?,?)";
    // 消息编码
    json sqlJson;
    sqlJson["sql"] = sql;
    sqlJson["userId"] = stoi(userId);
    sqlJson["sessionId"] = sessionId;
    sqlJson["is_user"] = is_user;
    sqlJson["content"] = content;
    sqlJson["ts"] = ms;

    // 转化为string格式
    std::string mes = sqlJson.dump(4);

    // 消息发送
    http::messagequeue::MQPublisher::instance().publish("message_sql_queue", mes);
}

// 从数据库中查询tokens
int AiHelper::getTokensFromMysql(std::string userId)
{
    try{
        int tokens = 0;
        std::string sql= "SELECT tokens FROM users WHERE id = ?";
        auto res = mysqlUtil_.executeQuery(sql,std::stoi(userId));
        while (res->next())
        {
            tokens = res->getInt64("tokens");
        }
        return tokens;
    }
    catch(const std::exception& e) {
        std::cerr << "getTokensFromMysql Failed : " << e.what() << std::endl;
        return 0; 
    }
}

// 将token修改添加到数据库中
void AiHelper::pushTokensToMysql(std::string userId,int totalTokens)
{
    // std::string sql= "UPDATE users SET tokens=? WHERE id = ?";
    // mysqlUtil_.executeUpdate(sql,totalTokens,userId);


    std::string sql= "UPDATE users SET tokens=? WHERE id = ?";
    // 消息编码
    json sqlJson;
    sqlJson["sql"] = sql;
    sqlJson["totalTokens"] = totalTokens;
    sqlJson["userId"] = userId;
   
    // 转化为string格式
    std::string mes = sqlJson.dump(4);

    // 消息发送
    http::messagequeue::MQPublisher::instance().publish("tokens_sql_queue", mes);
}