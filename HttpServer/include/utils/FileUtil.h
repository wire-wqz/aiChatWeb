/*
FileIUtil.h
wire
2025.12.10

文件处理工具类

使用案例：
FileUtil fileOperater(reqFile);
if (!fileOperater.isValid())
{
    LOG_WARN << reqFile << " not exist";
}

std::vector<char> buffer(fileOperater.size());                          创建固定大小字符缓冲区的代码
fileOperater.readFile(buffer);                                          将文件读取到buffer
std::string bufStr = std::string(buffer.data(), buffer.size());         buffer转string

resp.setContentLength(bufStr.size());
resp.setBody(bufStr);
*/

#pragma once
#include <fstream>
#include <string>
#include <vector>

#include <muduo/base/Logging.h>

class FileUtil
{
public:
    FileUtil(std::string filePath)
        : filePath_(filePath)
        , file_(filePath, std::ios::binary) // 打开文件，二进制模式
    {}

    ~FileUtil()
    {
        file_.close();
    }

    // 判断是否是有效路径
    bool isValid() const
    { return file_.is_open(); }
    
    // 重置打开默认文件
    void resetDefaultFile()
    {
        file_.close();
        filePath_="/root/AiHttpServer/AiChatWeb/resource/NotFound.html";
        file_.open(filePath_, std::ios::binary);
    }

    uint64_t size()
    {
        file_.seekg(0, std::ios::end); // 定位到文件末尾
        uint64_t fileSize = file_.tellg();
        file_.seekg(0, std::ios::beg); // 返回到文件开头
        return fileSize;
    }
    
    void readFile(std::vector<char>& buffer)
    {
        if (file_.read(buffer.data(), size()))
        {
            // LOG_INFO << "File content load into memory (" << size() << " bytes)";
        }    
        else
        {
            LOG_ERROR << "File read failed";
        }
    }

  
    // 文件类型判断
    string getContentType(){
        if (ends_with(filePath_,".html")) {
            return "text/html";
        }
        if (ends_with(filePath_,".css")) {
            return "text/css";
        }
        if (ends_with(filePath_,".js")) {
            return "application/javascript"; 
        }
        if (ends_with(filePath_,".jpg")) {
            return "image/jpeg";
        }
        if (ends_with(filePath_,".jpeg")) {
            return "image/jpeg";
        }
        if (ends_with(filePath_,".png")) {
            return "image/png";
        }
        if (ends_with(filePath_,".gif")) {
            return "image/gif";
        }
        return "text/plain";  
    }

private:
    bool ends_with(const string& str, const string& suffix)
    {
        if (str.size() < suffix.size()) return false;
        return str.substr(str.size() - suffix.size()) == suffix;
    }


private:
    std::string     filePath_;
    std::ifstream   file_;
};