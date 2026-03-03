#include"../../include/AiUtil/AiFactory.h"

// 单例模式
StrategyFactory& StrategyFactory:: instance(){
    static StrategyFactory factory;
    return factory;
}

// 注册
// void StrategyFactory::registerStrategy(const std::string& name, Creator creator){
//     creators[name]=std::move(creator);
// }

void StrategyFactory::registerStrategy(const std::string& name, std::shared_ptr<AiStrategy> creator){
    creators[name]=std::move(creator);
}

// 查询返回一个注册的creator
// std::shared_ptr<AiStrategy> StrategyFactory::create(const std::string& name) {
//     auto it = creators.find(name);
//     if (it == creators.end()) {
//         throw std::runtime_error("Unknown strategy: " + name);
//     }
//     return it->second();
// }
std::shared_ptr<AiStrategy> StrategyFactory::create(const std::string& name) {
    auto it = creators.find(name);
    if (it == creators.end()) {
        throw std::runtime_error("Unknown strategy: " + name);
    }
    return it->second;
}