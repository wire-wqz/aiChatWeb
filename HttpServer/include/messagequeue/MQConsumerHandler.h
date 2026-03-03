// 虚拟类
// 消息队列消费类



#pragma once
#include <string>

namespace http
{
namespace messagequeue
{
    
    
class MQConsumerHandler 
{
public:
    virtual ~MQConsumerHandler() = default;
    virtual void handle(const std::string& mes) = 0;
};


}
}