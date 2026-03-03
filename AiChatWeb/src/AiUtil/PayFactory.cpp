#include"../../include/AiUtil/PayFactory.h"
   
// 单例模式
PayStrategyFactory& PayStrategyFactory::instance()
{
    static PayStrategyFactory p;
    return p;
}

// 注册
void PayStrategyFactory::registerStrategy(const std::string& name, std::shared_ptr<PayStrategy> payStrategyPtr)
{
    creators[name]=std::move(payStrategyPtr);
}

// 查询返回一个注册的creator
std::shared_ptr<PayStrategy> PayStrategyFactory::create(const std::string& name){
    auto it = creators.find(name);
    if (it == creators.end()) {
        throw std::runtime_error("Unknown strategy: " + name);
    }
    return it->second;
}

// // 工厂注册
// static PayStrategyRegister<AliPayStrategy> reg_alipay("1");



