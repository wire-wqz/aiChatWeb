#include "../../include/handlers/GetStaticFileHandler.h"

void GetStaticFileHandler::handle(const http::HttpRequest& req, http::HttpResponse& resp)
{
    string path=req.getPath();
    
    // 设置目录
    std::string basePath = "/root/AiHttpServer/AiChatWeb/resource/";
    std::string filePath = basePath + path;
    
    // 
    FileUtil fileOperater(filePath);
    if (!fileOperater.isValid())
    {
        LOG_WARN << filePath << " not exist";
        fileOperater.resetDefaultFile(); // 返回NotFound.html      
    }

    // 读取文件
    std::vector<char> buffer(fileOperater.size());
    fileOperater.readFile(buffer); 
    std::string bufStr = std::string(buffer.data(), buffer.size());
    string fileType = fileOperater.getContentType();

    // 响应报文
    resp.setStatusLine(req.getVersion(), http::HttpResponse::k200Ok, "OK");
    resp.setCloseConnection(false);
    resp.setContentType(fileType);
    resp.setContentLength(bufStr.size());
    resp.setBody(bufStr);

}