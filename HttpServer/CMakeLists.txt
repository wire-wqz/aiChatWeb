# 收集核心源文件
file(GLOB_RECURSE HTTP_SERVER_SRC
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

# 创建静态库
add_library(HttpServer STATIC ${HTTP_SERVER_SRC})

# 设置包含目录
target_include_directories(HttpServer PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接库
target_link_libraries(HttpServer PRIVATE
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