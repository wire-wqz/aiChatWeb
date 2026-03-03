# AiChatWeb

## 上线网站:https::www.wire-wqz.xyz

基于 muduo 网络库构建的高性能 AI 对话 HTTP 框架，支持千问、豆包等大模型，集成 CMake 构建系统，并内置 SSL、MySQL、RabbitMQ、MCP 工具调用、RAG 知识库、支付宝支付等企业级功能模块。

---

## ✨ 项目特性

- 🚀 **高性能网络层**：基于 muduo 网络库实现 Reactor 模型，支持高并发 HTTP 服务
- 🤖 **多模型支持**：兼容千问、豆包等主流大模型接口
- 🧩 **模块化设计**：内置 MySQL、RabbitMQ、SSL、RAG、MCP 工具调用、支付等模块
- 🔧 **一体化构建**：采用 CMake 管理，依赖清晰，便于编译与集成
- 📦 **容器化支持**：提供完整 Docker 镜像，开箱即用

---

## 📦 依赖清单

| 依赖名称 | 说明 |
|----------|------|
| muduo_net / muduo_base | 高性能网络库 |
| SimpleAmqpClient / rabbitmq | RabbitMQ 客户端 |
| mysqlcppconn | MySQL C++ 驱动 |
| boost_system / boost_thread / boost_chrono | Boost 系统/线程/时间库 |
| ssl / crypto | OpenSSL 加密库 |
| curl | HTTP 客户端库 |
| sodium | 加密库 |
| pthread | 线程库 |

---

## 🔨 编译说明

### 使用 Docker 镜像（推荐）
我们提供了预编译的 [Docker 镜像](https://drive.google.com/file/d/1pS7b1gNG4VIe5S9adTs588vzvNbS9S_H/view?usp=sharing)，可免去手动编译依赖的步骤。  
下载镜像后直接加载即可运行。

### 手动编译步骤
若需从源码编译，请按以下流程操作：

### 1. 创建编译目录

```bash
mkdir build && cd build
```

### 2. 运行 CMake 编译

```bash
cmake .. && make
```

### 3. 输入文件目录：

```bash
./AiHttpServer/build/bin
```

---
## 🚀 快速启动

### 1. 加载 Docker 镜像

```bash
docker load -i aichatweb_image.tar
```

### 2. 创建并运行容器

```bash
docker run -d \
--name aichatweb_container \
-p 80:80 \
-p 443:443 \
-v /root/workspace:/workspace \
aichatweb_image \
tail -f /dev/null
```

### 3. 进入容器

```bash
docker exec -it aichatweb_container bash
```


### 4. 启动依赖服务

```bash
service mysql start
service rabbitmq-servce start
```


### 5. 启动 AiChatWeb 服务

```bash
cd AiHttpServer/build/bin
./AiChatWebServer
```

