// MQManager.h 
// wire 
// 2025.1.4
/* 
MQ:Message Queue 消息队列

MQManager —— 消息发送者：
这个类采用了 单例模式 (Singleton) 并且实现了一个 连接池 (Connection Pool)。它的主要作用是高效、安全地向 RabbitMQ 发送消息。

RabbitMQThreadPool —— 消息接收者
这个类实现了一个线程池。它的主要作用是启动多个线程，监听同一个队列，并行处理收到的消息
*/



#pragma once
#include "MQConsumerHandler.h"

#include <SimpleAmqpClient/SimpleAmqpClient.h>  // RabbitMQ客户端库rabbitmq-c的C++包装器，旨在简化处理AMQP过程
#include <vector>
#include <mutex>
#include <memory>
#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>
#include <functional>
#include <unordered_map>  

namespace http
{
namespace messagequeue
{

/***************************************************************/
/* 变量声明 */
 // 消息处理函数，输入string message 输出void
using HandlerFunc = std::function<void(const std::string&)>;

// 消息处理类（可以注册一个函数，也可以注册一个类（共享指针））
using HandlerPtr = std::shared_ptr<MQConsumerHandler>; 

/***************************************************************/
/* 生产者 */
class MQPublisher {
public:
    // 单例模式
    static MQPublisher& instance() {
        static MQPublisher mgr;
        return mgr;
    }

    // 均衡发送
    void publish(const std::string& queue, const std::string& msg);

private:

    // 线程安全的MQConn
    struct MQConn {
        AmqpClient::Channel::ptr_t channel;
        std::mutex mutexForChannel;
    };

    // 构造函数（默认连接池数为5）
    MQPublisher(size_t poolSize = 5);  

    // 单例模式：禁止拷贝和赋值
    MQPublisher(const MQPublisher&) = delete;  
    MQPublisher& operator=(const MQPublisher&) = delete;

private:
    std::vector<std::shared_ptr<MQConn>> pool_;        // 连接池
    size_t poolSize_;                                  // 连接池数量
    std::atomic<size_t> counter_;                      // 原子计数器
};


/***************************************************************/
/* 单消费者线程池 */
class RabbitMQThreadPool {
public:

    // 构造函数（队列名、线程数量、消息处理函数）
    RabbitMQThreadPool(const std::string& queue, int thread_num, HandlerFunc handler);
    RabbitMQThreadPool(const std::string& queue, int thread_num, HandlerPtr handler);

    // 析构函数
    ~RabbitMQThreadPool() {
        shutdown();
    }
    
    // 启动 ，stop_ = false ，向线程池中添加函数
    void start();

    // 停止， stop_ = true ， 向线程池中移除函数
    void shutdown();
    
    // 设置消息处理函数/类
    // void registerConsumer(const std::string& queue, int thread_num, HandlerFunc handler);
    // void registerConsumer(const std::string& queue, int thread_num, HandlerPtr handler);

private:
    // 消费者处理函数
    void worker(int id);

    // 具体的消息处理逻辑
    bool messageHandler(std::string msg);


private:
    std::vector<std::thread> workers_;         // 线程池，向线程池中提交一个函数，由线程池中的某个空闲线程在后台异步执行
    std::atomic<bool> stop_;                   // 控制线程池函数是否执行
    std::string queue_name_;
    int thread_num_;

    HandlerFunc handlerFunc_;                   // 消息处理函数
    HandlerPtr  handlerPtr_;                   // 消息处理类
};




/***************************************************************/
/* 多消费者池（单例模式）（基于map容器构造，导致队列名需要唯一） */
class MQConsumersPool {
public:
    
    // 单例模式
    static MQConsumersPool& instance() {
        static MQConsumersPool pool;
        return pool;
    }

    // 添加消费者
    void addConsumer(std::string queueName, HandlerFunc handle);
    void addConsumer(std::string queueName, HandlerPtr handle);

    // 启动某个消费者
    void startConsumer(std::string queueName);

    // 停止某个消费者
    void shutdownConsumer(std::string queueName);

private:
    // 构造函数
    MQConsumersPool(size_t thread_num = 2):
    thread_num_(thread_num)
    {};

    // 单例模式：禁止拷贝和赋值
    MQConsumersPool(const MQConsumersPool&) = delete;  
    MQConsumersPool& operator=(const MQConsumersPool&) = delete;

private:
    std::unordered_map<std::string,std::shared_ptr<http::messagequeue::RabbitMQThreadPool>> mqConsumers_;
    size_t thread_num_;
};


} // namespace messagequeue
} // namespace http