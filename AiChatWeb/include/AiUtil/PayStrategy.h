// PayStrategy.h 
// wire 
// 2025.2.3
/* 
统一管理和调用不同服务商的支付工具
*/

#pragma once
#include <string>
#include <iostream>
#include <atomic>
#include <chrono>
#include <random>
#include <fstream>
#include <sstream>


#include "../../../AliPayApi/openapi_client.h" 
#include "../../../HttpServer/include/utils/JsonUtil.h" 


class PayStrategy {
public:
    // 虚析构函数
    virtual ~PayStrategy() = default;

    // 生成简单的订单号
    string generateSimpleOrderNo();

    // 组装请求参数
    virtual JsonMap getPagePayContent(string& out_trade_no,double& total_amount, string& subject) const = 0;

    // 组装notify_url和return_url
    virtual StringMap getextendParamMap(string& notify_url, string& return_url) const = 0;

    // 获取PC支付网页跳转
    virtual string getHtmlUrl(const JsonMap& contentMap, const StringMap& extendParamMap=StringMap()) = 0;

    // 解析并验证NotifyResponse
    virtual map<string, string> parseNotify(const string &responseBody) = 0;
};

/*******************************************************************************/
// 支付宝支付

struct AliPayConfig {
    std::string appId;
    std::string apiUrl;
    std::string privateKey;
    std::string alipayPublicKey;
};

class AliPayStrategy : public PayStrategy {
public:
    // 构造函数
    AliPayStrategy(const std::string& path="/root/AiHttpServer/AiChatWeb/resource/AiUtils/PayTools/AliPayConfig.json");

    // 组装请求参数
    JsonMap getPagePayContent(string& out_trade_no,double& total_amount, string& subject) const override;

    // 组装notify_url和return_url
    StringMap getextendParamMap(string& notify_url, string& return_url) const override;

    // 获取PC支付网页跳转
    string getHtmlUrl(const JsonMap& contentMap, const StringMap& extendParamMap=StringMap()) override;

    // 解析并验证NotifyResponse
    map<string, string> parseNotify(const string &responseBody) override;
    
    


private:
    // 重写构造函数
    AliPayStrategy(const AliPayConfig& cfg);

    //json文件配置读取
    AliPayConfig loadConfig(const std::string& path);

private:
    string apiUrl_;                            // 沙盒模式请求的URL
    string appId_;                             // 应用ID
    string pKey_;                              // rsa私钥
    string aliPubKey_;                         // 支付宝公钥
    OpenapiClient AliPayClient_;
};