#include"../../include/AiUtil/PayStrategy.h"
#include"../../include/AiUtil/PayFactory.h"

// 生成简单的订单号
string PayStrategy::generateSimpleOrderNo() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = std::localtime(&now_time_t);
    char timeBuffer[20];
    strftime(timeBuffer, sizeof(timeBuffer), "%Y%m%d_%H%M%S", now_tm);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    return "ORDER_" + string(timeBuffer) + "_" + std::to_string(dis(gen));
}

/*******************************************************************************/
// 支付宝支付

// 构造函数
AliPayStrategy::AliPayStrategy(const std::string& path)
    :AliPayStrategy(loadConfig(path))
{
}

AliPayStrategy::AliPayStrategy(const AliPayConfig& cfg)
    : appId_(cfg.appId), 
      apiUrl_(cfg.apiUrl), 
      pKey_(cfg.privateKey), 
      aliPubKey_(cfg.alipayPublicKey), 
      AliPayClient_(appId_, pKey_, apiUrl_, OpenapiClient::default_charset, aliPubKey_)
{
}

AliPayConfig AliPayStrategy::loadConfig(const std::string& path)
{

    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open AliPay config file: " + path);
    }
    json j;
    file >> j;

    AliPayConfig cfg;
    cfg.appId = j["app_id"];
    cfg.apiUrl = j["api_url"];
    cfg.privateKey = j["private_key"];
    cfg.alipayPublicKey = j["alipay_public_key"];
    return cfg;
}

// 组装请求参数
JsonMap AliPayStrategy::getPagePayContent(string& out_trade_no,double& total_amount, string& subject) const
{
    JsonMap contentMap;
    contentMap.insert(JsonMap::value_type(JsonType("out_trade_no"), JsonType(out_trade_no)));
    contentMap.insert(JsonMap::value_type(JsonType("total_amount"), JsonType(total_amount)));
    contentMap.insert(JsonMap::value_type(JsonType("subject"), JsonType(subject)));
    contentMap.insert(JsonMap::value_type(JsonType("product_code"), JsonType("FAST_INSTANT_TRADE_PAY")));
    return contentMap;
}

StringMap AliPayStrategy::getextendParamMap(string& notify_url, string& return_url) const
{
    StringMap extendParamMap;
    extendParamMap.insert(StringMap::value_type("notify_url", notify_url));            // 通知充值成功url
    extendParamMap.insert(StringMap::value_type("return_url", return_url));            // 充值后返回
    return extendParamMap;
}


// 获取PC支付网页跳转
string AliPayStrategy::getHtmlUrl(const JsonMap& contentMap,const StringMap& extendParamMap)
{
    string method = "alipay.trade.page.pay";  // PC网页支付
    string  htmlUrl= AliPayClient_.invokeGetHtml(method, contentMap,extendParamMap);
    return htmlUrl;
}

// 解析并验证NotifyResponse
map<string, string> AliPayStrategy::parseNotify(const string &responseBody){
    return AliPayClient_.getNotifyResponse(responseBody);
}

//————————————————————————————————————————————————————————————————————————————————————————————//
// // // 注册工厂
// namespace {
//     static PayStrategyRegister<AliPayStrategy> ali("AliPay");
// }