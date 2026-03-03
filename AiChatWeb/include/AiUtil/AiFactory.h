// AiFactory.h 
// wire 
// 2025.1.4
/* 
工厂模式（Factory Pattern），并结合了单例模式（Singleton）和自动注册机制
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


#include"AiStrategy.h"

// class StrategyFactory {

// public:
//     // 输入为void 输出为std::shared_ptr<AiStrategy>的函数
//     using Creator = std::function<std::shared_ptr<AiStrategy>()>;

//     // 单例模式
//     static StrategyFactory& instance();

//     // 注册
//     void registerStrategy(const std::string& name, Creator creator);

//     // 查询返回一个注册的creator
//     std::shared_ptr<AiStrategy> create(const std::string& name);

// private:
//     StrategyFactory() = default;
//     std::unordered_map<std::string, Creator> creators;
// };

// // 工具类注册器
// // 注册：
// // StrategyRegister<AliyunStrategy>(name);
// // 使用：
// // std::shared_ptr<AiStrategy> strategy = StrategyFactory::instance().create(name);
// // Aliyun->getApiUrl();
// /*
// Creator 函数定义为：（虚函数）
// [] {
//         std::shared_ptr<AiStrategy> instance = std::make_shared<T>();
//         return instance;
//     }
// */
// template<typename T>
// struct StrategyRegister {
//     StrategyRegister(const std::string& name) {
//         StrategyFactory::instance().registerStrategy(name, [] {
//             std::shared_ptr<AiStrategy> instance = std::make_shared<T>();
//             return instance;
//             });
//     }
// };

class StrategyFactory {

public:
    // 单例模式
    static StrategyFactory& instance();

    // 注册
    void registerStrategy(const std::string& name, std::shared_ptr<AiStrategy>);

    // 查询返回一个注册的creator
    std::shared_ptr<AiStrategy> create(const std::string& name);

private:
    StrategyFactory() = default;
    std::unordered_map<std::string, std::shared_ptr<AiStrategy>> creators;
};

// 工具类注册器
// 注册：
// StrategyRegister<AliyunStrategy>("1");
// 使用：
// std::shared_ptr<AiStrategy> strategy = StrategyFactory::instance().create(name);
// Aliyun->getApiUrl();
/*
Creator 函数定义为：（虚函数）
*/
template<typename T>
struct StrategyRegister {
    StrategyRegister(const std::string& name) {
        StrategyFactory::instance().registerStrategy(name,std::make_shared<T>());
    }
};

