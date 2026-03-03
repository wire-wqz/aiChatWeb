#include<iostream>
#include "../include/http/HttpContext.h"
#include "../include/http/HttpRequest.h"
#include "../include/http/HttpResponse.h"
#include "../include/http/HttpServer.h"

#include "../include/router/Router.h"
#include "../include/router/RouterHandler.h"
#include "../include/session/SessionStorage.h"
#include "../include/middleware/cors/CorsMiddleware.h"
#include "../include/utils/db/DbConnection.h"
#include "../include/utils/db/DbConnectionPool.h"
#include "../include/utils/MysqlUtil.h"

# include <sstream> // istringstream
# include <fstream>



// http初始化 全局变量
auto m_server=std::make_unique<http::HttpServer>(8081,"WireServer");


bool fileExist(const string &filePath){
    ifstream readF;
    bool result;
    readF.open(filePath,ios::in);
    result = readF.is_open();
    readF.close();
    return result;
}

string readFile(const string &filePath){
    ifstream readF;

    readF.open(filePath,ios::in);
    
    if(readF.is_open()==0){
        cout<<filePath<<"打开失败"<<endl;
        return "";
    }

    ostringstream buf;
    buf << readF.rdbuf();
    readF.close();
    return buf.str();
}

bool ends_with(const string& str, const string& suffix) {
    if (str.size() < suffix.size()) return false;
    return str.substr(str.size() - suffix.size()) == suffix;
}

string getContentType(const string &filePath){
    if (ends_with(filePath,".html")) {
        return "text/html";
    }
    if (ends_with(filePath,".css")) {
        return "text/css";
    }
    if (ends_with(filePath,".js")) {
        return "application/javascript"; 
    }
    if (ends_with(filePath,".jpg")) {
        return "image/jpeg";
    }
    if (ends_with(filePath,".jpeg")) {
        return "image/jpeg";
    }
    if (ends_with(filePath,".png")) {
        return "image/png";
    }
    if (ends_with(filePath,".gif")) {
        return "image/gif";
    }
    return "text/plain";  
}

std::string injectVisitCount(const std::string& htmlContent, const std::string& visitCount, const std::string& username) {
    std::string user = username.empty() ? "游客" : username;
    
    // 构建要插入的HTML代码
    std::string visitHtml = R"(
    <div style="position:fixed;top:20px;right:20px;background:#4CAF50;color:white;padding:12px;border-radius:8px;z-index:1000;">
        <div>用户: )" + user + R"(</div>
        <div>访问次数: <strong>)" + visitCount + R"(</strong></div>
    </div>
    )";
    
    // 在</body>标签前插入
    size_t bodyEndPos = htmlContent.find("</body>");
    if (bodyEndPos == std::string::npos) {
        return htmlContent + visitHtml;
    }
    
    return htmlContent.substr(0, bodyEndPos) + visitHtml + htmlContent.substr(bodyEndPos);
}

// 静态路由的局限性：显示图片等主页后续资源，每一个资源需要额外注册；因此采用动态路由处理，例如m_server.addRoute("GET", "/images/.*", handleStaticFile); 
// void helloHtml(const http::HttpRequest& req, http::HttpResponse& resp)
// {
//     string filePath="./hello.html";

//     if(fileExist(filePath))
//     {
//         // 获得html信息
//         string content = readFile(filePath); 
//         string contentType = getContentType(filePath);

//         // 填充响应报文
//         resp.setStatusLine(req.getVersion(),http::HttpResponse::k200Ok,"OK");
//         resp.setContentType(contentType);
//         resp.setContentLength(content.size());
//         resp.setBody(content);
//     }
//     else{
//         resp.setVersion(req.getVersion());
//         resp.setStatusCode(http::HttpResponse::k404NotFound);
//         resp.setStatusMessage("Not Found");
//         resp.setBody(filePath+" Not Found");
//         resp.setCloseConnection(true);   
//     }
// }


void cbMainHtml(const http::HttpRequest& req, http::HttpResponse& resp)
{
    string filePath="./index.html";

    if(fileExist(filePath))
    {
        // 从连接池获取一个conn连接mysql
        // std::shared_ptr<http::db::DbConnection> dbconn(http::db::DbConnectionPool::getInstance().getConnection()); 
        // if(dbconn->isValid())
        // {
        //     std::cout<< "Mysql连接成功"<< std::endl;
        //     std::unique_ptr<sql::ResultSet> res(dbconn->executeQuery("select * from user_tab where age>=?",18));
        //     while(res->next())
        //     {
        //         std::cout << " 姓名: "<< res->getString("name") 
        //                   << ", 年龄："<< res->getInt("age") 
        //                   << std::endl;
        //     }    
        // }


        // 使用工具类与mysql交互
        std::unique_ptr<sql::ResultSet> res(http::MysqlUtil::executeQuery("select * from user_tab where age>=?",18));
        while(res->next())
        {
            std::cout << " 姓名: "<< res->getString("name") 
                        << ", 年龄："<< res->getInt("age") 
                        << std::endl;
        }   
        
        
        // 会话管理
        auto session = m_server->getSessionManager()->getSession(req,resp);  // 得到session(存cookie存在而返回存在的，否则新建) 返回共享指针
        std:: string visitCountStr = session->getValue("visit_count");
        int visitCount;
        if(visitCountStr.size()==0) // 第一次访问
        {
            visitCount = 1;
            session->setValue("visit_count",std::to_string(visitCount));
            session->setValue("username","游客");
        }
        else
        {    
            visitCount = std::stoi(visitCountStr)+ 1; 
            session->setValue("visit_count",std::to_string(visitCount));
        }

        
        // 获得html信息
        string content = readFile(filePath); 
        string contentType = getContentType(filePath);

        // html注入函数（注入访问信息）
        content = injectVisitCount(content, std::to_string(visitCount), session->getValue("username"));

        // 填充响应报文
        resp.setStatusLine(req.getVersion(),http::HttpResponse::k200Ok,"OK");
        resp.setContentType(contentType);
        resp.setContentLength(content.size());
        resp.setBody(content);
    }
    else{
        resp.setVersion(req.getVersion());
        resp.setStatusCode(http::HttpResponse::k404NotFound);
        resp.setStatusMessage("Not Found");
        resp.setBody(filePath+" Not Found");
        resp.setCloseConnection(true);   
    }
}

void cbRouwanziShow(const http::HttpRequest& req, http::HttpResponse& resp)
{
    string filePath="."+req.getPath();
   

    if(fileExist(filePath))
    {
        string content = readFile(filePath); 
        string contentType = getContentType(filePath);
        resp.setStatusLine(req.getVersion(),http::HttpResponse::k200Ok,"OK");
        resp.setContentType(contentType);
        resp.setContentLength(content.size());
        resp.setBody(content);
    }
    else{
        resp.setVersion(req.getVersion());
        resp.setStatusCode(http::HttpResponse::k404NotFound);
        resp.setStatusMessage("Not Found");
        resp.setBody(filePath+" Not Found");
        resp.setCloseConnection(true);   
    }
}

int main(){
    // mysql连接池init(全局唯一实例)
    // http::db::DbConnectionPool::getInstance().init("127.0.0.1","wireMysql","wire123","demo_db");         // 类静态变量可以通过类名访问 
    http::MysqlUtil::init("127.0.0.1", "wireMysql", "wire123", "demo_db");                                  // 类静态变量可以通过类名访问

    // 添加中间件
    auto corsmiddleware=std::make_shared<http::middleware::CorsMiddleware>();                               // 创建Cors中间件（默认规则）
    m_server->addMiddleware(std::move(corsmiddleware));                                                     // 添加中间件                           
    
    
    // 路由管理器初始化
    auto sessionStoragePtr=std::make_unique<http::session::MemorySessionStorage>();                         // 创建路由会话存储（内存方式）
    auto SessionManagerPtr = std::make_unique<http::session::SessionManager>(std::move(sessionStoragePtr)); // 创建路由管理器
    m_server->setSessionManager(std::move(SessionManagerPtr));                                              // 设置路由管理器


    // 注册路由
    // m_server->Get("/",helloHtml);// 默认路径
    m_server->Get("/",cbMainHtml);// 默认路径
    m_server->addRoute(http::HttpRequest::kGet,"/images/:filename",cbRouwanziShow);                         // 动态正则匹配，自动匹配具有/images/xxxx的格式

    // ssl加密
    m_server->enableSSL(1);
    ssl::SslConfig config;
    config.setPrivateKeyFile("./CaServer.key");
    config.setCertificateFile("./CaServer.crt");
    m_server->setSslConfig(config);
    
    // 启动
    m_server->setThreadNum(2);
    m_server->start();
    return 0;
}