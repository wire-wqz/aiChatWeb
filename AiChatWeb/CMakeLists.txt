# 收集主程序源文件

file(GLOB_RECURSE AICHAT_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# 创建可执行文件
add_executable(AiChatWebServer ${AICHAT_SRC})

# 设置包含目录
target_include_directories(AiChatWebServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接库
target_link_libraries(AiChatWebServer PRIVATE
    HttpServer
    AliPayApi
    SimpleAmqpClient
    rabbitmq
    boost_chrono
    muduo_net
    muduo_base
    boost_system
    boost_thread
    pthread
    mysqlcppconn
    ssl
    crypto
    curl
    sodium   
)