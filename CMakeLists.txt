# 设置版本要求
cmake_minimum_required(VERSION 3.10) # 设置CMake最低版本要求
project(AiHttpServer VERSION 1.0)    # 定义项目名称和版本
set(CMAKE_CXX_STANDARD 17)           # 设置CPP标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)  # 设置该标准项必须 

# 设置输出目录(设置目录要在子目录之前，否则会被覆盖)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 设置包含子目录
add_subdirectory(HttpServer)
add_subdirectory(AiChatWeb)
add_subdirectory(AliPayApi)

