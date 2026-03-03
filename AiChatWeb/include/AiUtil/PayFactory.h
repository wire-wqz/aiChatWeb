// PayFactory.h 
// wire 
// 2025.2.3
/* 
支付工厂模式（Factory Pattern），并结合了单例模式（Singleton）和自动注册机制
*/

#pragma once
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <sstream>
#include <memory>
#include <functional>
#include <unordered_map>
#include <string>


#include"PayStrategy.h"

class PayStrategyFactory {

public:
    // 单例模式
    static PayStrategyFactory& instance();

    // 注册
    void registerStrategy(const std::string& name, std::shared_ptr<PayStrategy>);

    // 查询返回一个注册的creator
    std::shared_ptr<PayStrategy> create(const std::string& name);

private:
    PayStrategyFactory() = default;
    std::unordered_map<std::string, std::shared_ptr<PayStrategy>> creators;
};

// 工具类注册器
// 注册：
// StrategyRegister<AliyunStrategy>("1");
template<typename T>
struct PayStrategyRegister {
    PayStrategyRegister(const std::string& name) {
        PayStrategyFactory::instance().registerStrategy(name,std::make_shared<T>());
    }
};

