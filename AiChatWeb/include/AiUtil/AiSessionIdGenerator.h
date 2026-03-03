// AiSessionIdGenerator.h 
// wire 
// 2025.12.30
/* 
随机生成一个聊天session ID
*/


#pragma once
#include <chrono>
#include <random>
#include <cstdlib>
#include <ctime>
#include <string>


class AiSessionIdGenerator {
public:
    AiSessionIdGenerator() {
        std::srand(static_cast<unsigned>(std::time(nullptr)));
    }
    
    std::string generate();
};
