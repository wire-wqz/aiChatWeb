#include "../../include/handlers/ChatRegisterHandler.h"
#include "../../../HttpServer/include/messagequeue/MQManager.h"


void ChatRegisterHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
{
    try{
     
        // 解析请求报文
        std::string userName;
        std::string password;

        json parsed = json::parse(req.getBody());
        if (parsed.contains("userName")) 
            userName = parsed["userName"];
        if (parsed.contains("password")) 
            password = parsed["password"];

        bool registerStatus=addUserId(userName,password);


        // 封装响应报文
        json successResp;
        successResp["success"] = true;    
        successResp["registerStatus"] = registerStatus;
        std::string successBody = successResp.dump(4);
        resp.setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
        resp.setCloseConnection(false);
        resp.setContentType("application/json");
        resp.setContentLength(successBody.size());
        resp.setBody(successBody);
        return;
    }
    catch (const std::exception& e)
    {
        json failureResp;
        failureResp["success"] = false;
        std::string failureBody = failureResp.dump(4);
        resp.setStatusLine(req.getVersion(), http::HttpResponse::k400BadRequest, "Bad Request");
        resp.setCloseConnection(true);
        resp.setContentType("application/json");
        resp.setContentLength(failureBody.size());
        resp.setBody(failureBody);
    }
}

bool ChatRegisterHandler::addUserId(const std::string& username, const std::string& password){
    std::string sql1="select id from users where username = ?";
    auto res = mysqlUtil_.executeQuery(sql1,username);
    if(res->next())
    {
        return false;
    }

    // 密码哈希化+salt 存储
    // 定义了哈希字符串所需的长度
    char hashed_password[crypto_pwhash_STRBYTES];  
    // 进行哈希计算 (Argon2id)
    // 这个函数会自动：1、生成随机盐  2、使用 Argon2id 算法 3、将 盐 + 参数 + 哈希结果 格式化为一个字符串
    if (crypto_pwhash_str(
            hashed_password, 
            password.c_str(), 
            password.length(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE, // 计算强度（CPU时间）
            crypto_pwhash_MEMLIMIT_INTERACTIVE  // 内存消耗
        ) != 0) {
        std::cerr << "Out of memory" << std::endl;
        return false;
    }
    // 同步SQL
    // std::string sql2="INSERT INTO users (username,password) VALUES (?,?)";
    // mysqlUtil_.executeUpdate(sql2,username,std::string(hashed_password));

    // 消息队列MQ异步写SQL
    std::string sql2="INSERT INTO users (username,password,tokens) VALUES (?,?,?)";

    json jsonSql;
    jsonSql["sql"] = sql2;
    jsonSql["username"] = username;
    jsonSql["hashed_password"] = hashed_password;
    jsonSql["totalTokens"] = 50000;

    std::string mes = jsonSql.dump(4); 

    http::messagequeue::MQPublisher::instance().publish("register_sql_queue",mes);

    return true;
}