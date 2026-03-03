# aiChatWeb
基于muduo网络库构造的AiChat-HTTP服务框架，支持千问豆包对话，集成cmake、ssl、mysql、rabbitmq、MCP工具调用、RAG知识库、支付宝支付等

# 项目依赖

# 项目编译
提供项目docker镜像，可省去编译过程；若自行编译请参考下述流程：
1、 创建编译文件夹
mkdir build
cd build
2、 
cmake ..
3、编译项目
make
4、生成目录
./AiHttpServer/build/bin

# 项目启动
1、加载镜像文件到docker
docker load -i aichatweb_image.tar

2、创建docker容器
docker run -d \
--name aichatweb_container \
-p 80:80 \
-p 443:443 \
-v /root/workspace:/workspace \
aichatweb_image \
tail -f /dev/null

3、进入容器
docker  exec -it aichatweb_container bash

4、启动mysql和rabbitmq
service mysql start
service rabbitmq-servce start

5、进入项目路径，启动
cd AiHttpServer/build/bin
./AiChatWebServer
