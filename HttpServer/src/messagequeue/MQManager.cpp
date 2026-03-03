#include "../../include/messagequeue/MQManager.h"

namespace http
{
namespace messagequeue
{

/***************************************************************/
/* 生产者 */
// 构造函数（默认连接池数为5）
MQPublisher::MQPublisher(size_t poolSize):
poolSize_(poolSize),
counter_(0)
{
    // 基于连接创建poolSize个连接池
    for(int i=0;i<poolSize_;i++)
    {
        auto conn = std::make_shared<MQConn>();
        conn -> channel = AmqpClient::Channel::Create(
            "localhost",  // RabbitMQ主机名或IP地址
            5672,         // RabbitMQ服务端口
            "guest",      // 用户名
            "guest",      // 密码
            "/"           // 虚拟主机
        );
        pool_.push_back(conn);
    }
}

// 均衡发送
void MQPublisher::publish(const std::string& queue, const std::string& msg){
    try{
        // 取出连接
        size_t index = counter_.fetch_add(1) % poolSize_;  // 轮询​ 负载均衡策略
        auto& conn = pool_[index];

        // 上锁
        std::lock_guard<std::mutex>(conn->mutexForChannel);

        // 消息创建
        auto message = AmqpClient::BasicMessage::Create(msg);

        // 消息发送
        conn->channel->BasicPublish(
            "",         // 目标交换机名称 可以为空：""
            queue,      // 路由键，用于决定消息路由到哪个队列
            message     // 要发送的消息对象
        );
    } catch(const std::exception &e){
        std::cerr << "MQManager publish error: " << e.what() << std::endl;
    }
}

/***************************************************************/
/* 单消费者线程池 */


// 构造函数（队列名、线程数量、消息处理函数）
RabbitMQThreadPool::RabbitMQThreadPool(const std::string& queue, int thread_num, HandlerFunc handler):
queue_name_(queue),
thread_num_(thread_num),
handlerFunc_(std::move(handler)),
stop_(true)
{}

RabbitMQThreadPool::RabbitMQThreadPool(const std::string& queue, int thread_num, HandlerPtr handler):
queue_name_(queue),
thread_num_(thread_num),
handlerPtr_(std::move(handler)),
stop_(true)
{}

// 设置消息处理函数/类
bool RabbitMQThreadPool::messageHandler(std::string msg){
    
    if (msg.empty()) {
        std::cerr << "警告：收到空消息" << std::endl;
        return false; 
    }

    if (!handlerFunc_ && !handlerPtr_) {
        std::cerr << "错误：没有设置消息处理器" << std::endl;
        return false;
    }

    try{
        
        if(handlerPtr_)
        {
            handlerPtr_->handle(msg);  
        }
        else if(handlerFunc_)
        {
            handlerFunc_(msg);   
        }
        return true;  // 处理成功

    } 
    catch (const std::exception& e) {
        std::cerr << "消息处理失败: " << e.what() 
                  << ", 消息内容: " << (msg.length() > 100 ? msg.substr(0, 100) + "..." : msg) 
                  << std::endl;
    }
    
    return false;
}



// 消费者处理函数
void RabbitMQThreadPool::worker(int id){
    try{
        // 建立连接
        auto channel = AmqpClient::Channel::Create(
            "localhost", 
            5672, 
            "guest", 
            "guest", 
            "/"
        );

        // 声明队列
        channel->DeclareQueue(
            queue_name_,  // 队列名称，空字符串则服务器生成随机名称
            false,       // passive: 仅检查队列是否存在
            true,        // durable: 持久化队列
            false,       // exclusive: 排他队列（仅当前连接可用）
            false        // auto_delete: 无消费者时自动删除
        );

        // 开始消费消息
        std::string consumer_tag = channel->BasicConsume(
            queue_name_,  // 要消费的队列名称
            "",          // consumer_tag: 消费者标识，空字符串由服务器生成
            true,        // no_local: 不接收自己发布的消息
            false,       // no_ack: 设置为false，需要手动确认消息
            false        // exclusive: 排他消费者
        );
    
        // 设置预取数量
        channel->BasicQos(consumer_tag, 1);  // 该消费者可以最多预取的消息数量

        // 处理消息循环
        while (!stop_)
        {   
            // 接收消息（设置超时时间避免永久阻塞）
            AmqpClient::Envelope::ptr_t env;
            bool success = channel->BasicConsumeMessage(consumer_tag, env, 1000); // 1000ms
            if (success && env) {
                std::string msg = env->Message()->Body();   // 取出消息
                messageHandler(msg);                        // 处理消息
                channel->BasicAck(env);                     // 响应消息
            }
        }

        // 停止消息接收 并优雅的关闭
        channel->BasicCancel(consumer_tag);
    } catch(const std::exception& e){
        std::cerr << "Thread " << id << " exception: " << e.what() << std::endl;
    }
}

// 启动 ，stop_ = false ，向线程池中添加函数
void RabbitMQThreadPool::start(){
    stop_ = false;
    for (int i = 0; i < thread_num_; ++i) {
        workers_.emplace_back(&RabbitMQThreadPool::worker, this, i);  // 添加类内函数
        // 等效于workers_.push_back(std::thread(&RabbitMQThreadPool::worker, this, i));
    }
}

// 停止， stop_ = true ， 向线程池中移除函数
void RabbitMQThreadPool::shutdown(){
    stop_ = true;  
    // 如果线程t可以被等待，那么就等待它执行结束
    for (auto& t : workers_) {
        if (t.joinable()) 
            t.join();
    }
}



/***************************************************************/
/* 多消费者池（单例模式）（基于map容器构造，导致队列名需要唯一） */

 // 添加消费者
void MQConsumersPool::addConsumer(std::string queueName, HandlerFunc handle)
{
    if(mqConsumers_.find(queueName)!=mqConsumers_.end())
    {
        std:: cerr<<queueName<<"已存在，插入失败"<< std::endl;
        return;
    }    

    auto consumerThread = std::make_shared<RabbitMQThreadPool>(queueName,thread_num_,handle);
    mqConsumers_[queueName]=std::move(consumerThread);
}
void MQConsumersPool::addConsumer(std::string queueName, HandlerPtr handle)
{
    if(mqConsumers_.find(queueName)!=mqConsumers_.end())
    {
        std:: cerr<<queueName<<"已存在，插入失败"<< std::endl;
        return;
    }    
    auto consumerThread = std::make_shared<RabbitMQThreadPool>(queueName,thread_num_,handle);
    mqConsumers_[queueName]=std::move(consumerThread);
}

// 启动某个消费者
void MQConsumersPool::startConsumer(std::string queueName)
{
    if(mqConsumers_.find(queueName)==mqConsumers_.end())
    {
        std:: cerr<<queueName<<"不存在，启动失败"<< std::endl;
        return;
    }
    mqConsumers_[queueName]->start();
}

// 停止某个消费者
void MQConsumersPool::shutdownConsumer(std::string queueName)
{
    if(mqConsumers_.find(queueName)==mqConsumers_.end())
    {
        std:: cerr<<queueName<<"不存在，停止失败"<< std::endl;
        return;
    }
    mqConsumers_[queueName]->shutdown();
}
    

}    // namespace messagequeue
}    // http